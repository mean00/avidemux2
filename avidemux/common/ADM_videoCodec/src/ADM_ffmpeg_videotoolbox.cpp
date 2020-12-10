/***************************************************************************
    \file       ADM_ffmpeg_videotoolbox.cpp
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_cpp.h"
#include "ADM_default.h"

#ifdef USE_VIDEOTOOLBOX
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/videotoolbox.h"
#include "libavutil/pixdesc.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "ADM_hwAccel.h"
#include "ADM_image.h"
#include "ADM_coreVideoToolbox.h"
#include "prefs.h"
#include "../private_inc/ADM_ffmpeg_videotoolbox_internal.h"

extern "C"
{
static enum AVPixelFormat ADM_VT_getFormat(struct AVCodecContext *avctx, const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("[VideoToolbox]: GetFormat\n");
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    AVPixelFormat outPix;
    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        c=fmt[i];
        char name[300]={0};
        av_get_pix_fmt_string(name,sizeof(name),c);
        ADM_info("[VideoToolbox]: Evaluating PIX_FMT %d,%s\n",c,name);
        av_get_codec_tag_string(name,sizeof(name),avctx->codec_id);
        ADM_info("\t  Evaluating codec %d,%s\n",avctx->codec_id,name);
        if(c!=AV_PIX_FMT_VIDEOTOOLBOX) continue;
#define FMT_V_CHECK(x,y) case AV_CODEC_ID_##x: outPix=AV_PIX_FMT_VIDEOTOOLBOX; id=avctx->codec_id; break;

        switch(avctx->codec_id)
        {
            FMT_V_CHECK(H264,H264)
            FMT_V_CHECK(H265,H265) // requires ffmpeg >= 3.4
#if 0
            FMT_V_CHECK(MPEG1VIDEO,MPEG1)
            FMT_V_CHECK(MPEG2VIDEO,MPEG2) // check succeeds, hw decoder init fails
#endif
            FMT_V_CHECK(VC1,VC1)
            default:
                ADM_info("No hw support for format %d\n",avctx->codec_id);
                continue;
                break;
        }
        break;
    }
    if(id==AV_CODEC_ID_NONE)
    {
        return AV_PIX_FMT_NONE;
    }
    // Finish intialization of VideoToolbox decoder
#if 0 // The lavc functions we rely on in ADM_acceleratedDecoderFF::parseHwAccel are no more
    const AVHWAccel *accel=ADM_acceleratedDecoderFF::parseHwAccel(outPix,id,AV_PIX_FMT_VIDEOTOOLBOX);
    if(accel)
    {
        ADM_info("Found matching hw accelerator : %s\n",accel->name);
        ADM_info("Successfully setup hw accel\n");
        return AV_PIX_FMT_VIDEOTOOLBOX;
    }
    return AV_PIX_FMT_NONE;
#endif
    return AV_PIX_FMT_VIDEOTOOLBOX;
}
}

/**
    \fn ctor
*/
decoderFFVT::decoderFFVT(struct AVCodecContext *avctx, decoderFF *parent) : ADM_acceleratedDecoderFF(avctx,parent)
{
    AVCodecID codecID;
    const char *name="";
    alive = false;
    copy = NULL;

    switch(_context->codec_id)
    {
        case AV_CODEC_ID_HEVC:
            name="h265";
            break;
        case AV_CODEC_ID_H264:
            name="h264";
            break;
        case AV_CODEC_ID_MPEG1VIDEO:
        case AV_CODEC_ID_MPEG2VIDEO:
            name="mpegvideo";
            break;
        case AV_CODEC_ID_VC1:
            name="vc1";
            break;
        default:
            ADM_warning("codec not in the list\n");
            break;
    }
    if(admCoreVideoToolbox::initVideoToolbox(avctx))
    {
        ADM_error("VideoToolbox init failed\n");
        return;
    }
    alive = true;
    copy = new ADMImageDefault(avctx->width, avctx->height);
    ADM_info("Successfully setup hw accel\n");
}
/**
    \fn dtor
*/
decoderFFVT::~decoderFFVT()
{
    ADM_info("Destroying VideoToolbox decoder\n");
    if(copy)
    {
        delete copy;
        copy = NULL;
    }
}
/**
    \fn uncompress
*/
bool decoderFFVT::uncompress(ADMCompressedImage *in, ADMImage *out)
{
    if(!_parent->getDrainingState() && !in->dataLength) // Null frame, silently skipped
    {
        out->_noPicture = 1;
        out->Pts=ADM_COMPRESSED_NO_PTS;
        ADM_info("[VideoToolbox] Nothing to decode -> no picture\n");
        return true;
    }

    out->Pts=in->demuxerPts;
    _context->reordered_opaque=in->demuxerPts;

    AVFrame *frame=_parent->getFramePointer();
    ADM_assert(frame);

    int ret;

    if(_parent->getDrainingState())
    {
        if(_parent->getDrainingInitiated()==false)
        {
            avcodec_send_packet(_context, NULL);
            _parent->setDrainingInitiated(true);
        }
    }else if(!handover)
    {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data=in->data;
        pkt.size=in->dataLength;
        if(in->flags&AVI_KEY_FRAME)
            pkt.flags=AV_PKT_FLAG_KEY;
        else
            pkt.flags=0;

        ret = avcodec_send_packet(_context, &pkt);

        /* libavcodec doesn't handle switching between field and frame encoded parts of H.264 streams
        in the way VideoToolbox expects. Proceeding with avcodec_receive_frame as if nothing happened
        triggers a segfault. As a workaround, retry once with the same data. We do lose one picture. */
        if(ret == AVERROR_UNKNOWN)
        {
            ADM_warning("Unknown error from avcodec_send_packet, retrying...\n");
            if(!alive)
                return false; // avoid endless loop
            alive = false; // misuse, harmless
            return _parent->uncompress(in,out); // retry
        }
    }else
    {
        handover=false;
    }
    alive = true;

    ret = avcodec_receive_frame(_context, frame);

    if(!_parent->decodeErrorHandler(ret))
        return false;

    if(frame->pict_type==AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture=true;
        out->Pts = (uint64_t)(frame->reordered_opaque);
        ADM_info("[VideoToolbox] No picture \n");
        return false;
    }

    int result=admCoreVideoToolbox::copyData(_context, frame, copy);
    if(result)
    {
        ADM_error("copying hw image failed, return value was %d\n",result);
        return false;
    }

    copy->Pts = (uint64_t)(frame->reordered_opaque);
    copy->flags = admFrameTypeFromLav(frame);
    copy->_range = (frame->color_range == AVCOL_RANGE_JPEG)? ADM_COL_RANGE_JPEG : ADM_COL_RANGE_MPEG;
    copy->refType=ADM_HW_NONE;
    for(int i=0;i<3;i++)
    {
        out->_planes[i] = copy->_planes[i];
        out->_planeStride[i] = copy->_planeStride[i];
    }
    out->copyInfo(copy);

    return true;
}


class ADM_hwAccelEntryVideoToolbox : public ADM_hwAccelEntry
{
public: 
                        ADM_hwAccelEntryVideoToolbox();
     virtual bool       canSupportThis( struct AVCodecContext *avctx, const enum AVPixelFormat *fmt, enum AVPixelFormat &outputFormat );
     virtual            ADM_acceleratedDecoderFF *spawn( struct AVCodecContext *avctx, const enum AVPixelFormat *fmt );
     virtual            ~ADM_hwAccelEntryVideoToolbox() {};
};
/**
 * 
 */
ADM_hwAccelEntryVideoToolbox::ADM_hwAccelEntryVideoToolbox()
{
    name="VideoToolbox";
}
/**
 * 
 * @param avctx
 * @param fmt
 * @param outputFormat
 * @return 
 */
bool ADM_hwAccelEntryVideoToolbox::canSupportThis( struct AVCodecContext *avctx, const enum AVPixelFormat *fmt, enum AVPixelFormat &outputFormat )
{
    bool enabled=true;
    prefs->get(FEATURES_VIDEOTOOLBOX,&enabled);
    if(!enabled)
    {
        ADM_info("VideoToolbox not enabled\n");
        return false;
    }

    enum AVPixelFormat ofmt=ADM_VT_getFormat(avctx,fmt);
    if(ofmt==AV_PIX_FMT_NONE)
        return false;
    outputFormat=ofmt;
    ADM_info("Assuming that this is supported by VideoToolbox\n");
    return true;
}

ADM_acceleratedDecoderFF *ADM_hwAccelEntryVideoToolbox::spawn( struct AVCodecContext *avctx, const enum AVPixelFormat *fmt )
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    decoderFFVT *dec=new decoderFFVT(avctx,ff);
    if(!dec->alive)
        return NULL;

    return (ADM_acceleratedDecoderFF *)dec;
}

bool videotoolboxProbe(void)
{
    return true; // FIXME
}

bool admVideoToolbox_exitCleanup(void)
{
    return true; // FIXME
}

static ADM_hwAccelEntryVideoToolbox videoToolboxEntry;

/**
 *
 */
bool initVideoToolboxDecoder(void)
{
    ADM_info("Registering VideoToolbox hw decoder\n");
    ADM_hwAccelManager::registerDecoder(&videoToolboxEntry);
    return true;
}
#endif
// EOF
