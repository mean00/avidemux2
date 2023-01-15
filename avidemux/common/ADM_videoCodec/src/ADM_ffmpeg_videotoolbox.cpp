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
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "ADM_hwAccel.h"
#include "ADM_image.h"
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
        snprintf(name,300,"%s",avcodec_get_name(avctx->codec_id));
        ADM_info("\t  Evaluating codec %d, %s\n",avctx->codec_id,name);
        if(c!=AV_PIX_FMT_VIDEOTOOLBOX) continue;
#define FMT_V_CHECK(x,y) case AV_CODEC_ID_##x: outPix=AV_PIX_FMT_VIDEOTOOLBOX; id=avctx->codec_id; break;

        switch(avctx->codec_id)
        {
            FMT_V_CHECK(H264,H264)
            FMT_V_CHECK(H265,H265) // requires ffmpeg >= 3.4
#if 0
            FMT_V_CHECK(MPEG1VIDEO,MPEG1) // actually works, but no benefit
            FMT_V_CHECK(MPEG2VIDEO,MPEG2) // check succeeds, hw decoder init fails
#endif
            FMT_V_CHECK(VC1,VC1)
            FMT_V_CHECK(VP9,VP9)
            default:
                ADM_info("No hw support for %s\n",name);
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
    AVBufferRef *hwDevRef = NULL;
    int err = av_hwdevice_ctx_create(&hwDevRef, AV_HWDEVICE_TYPE_VIDEOTOOLBOX, NULL, NULL, 0);
    if(err < 0)
    {
        ADM_error("Cannot initialize VideoToolbox\n");
        avctx->hw_device_ctx = NULL;
        return AV_PIX_FMT_NONE;
    }
    avctx->hw_device_ctx = av_buffer_ref(hwDevRef);

    return AV_PIX_FMT_VIDEOTOOLBOX;
}
}

/**
    \fn ctor
*/
decoderFFVT::decoderFFVT(struct AVCodecContext *avctx, decoderFF *parent) : ADM_acceleratedDecoderFF(avctx,parent)
{
    swframeIdx = 0;
    alive = false;
    for(int i = 0; i < NB_SW_FRAMES; i++)
    {
        AVFrame *cpy = av_frame_alloc();
        if(!cpy) return;
        swframes[i] = cpy;
    }
    alive = true;
    ADM_info("VideoToolbox hw accel decoder object created with hw dev ctx at %p\n", avctx->hw_device_ctx);
}
/**
    \fn dtor
*/
decoderFFVT::~decoderFFVT()
{
    ADM_info("Destroying VideoToolbox decoder\n");
    if(_context && _context->hw_device_ctx)
        av_buffer_unref(&_context->hw_device_ctx);

    for(int i = 0; i < NB_SW_FRAMES; i++)
    {
        AVFrame *cpy = swframes[i];
        if(!cpy) continue;
        av_frame_free(&cpy);
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

    int ret = 0;

    if(_parent->getDrainingState())
    {
        if(_parent->getDrainingInitiated()==false)
        {
            avcodec_send_packet(_context, NULL);
            _parent->setDrainingInitiated(true);
        }
    }else if(!handover)
    {
        AVPacket *pkt = _parent->getPacketPointer();
        ADM_assert(pkt);
        pkt->data = in->data;
        pkt->size = in->dataLength;

        if(in->flags&AVI_KEY_FRAME)
            pkt->flags = AV_PKT_FLAG_KEY;
        else
            pkt->flags = 0;

        ret = avcodec_send_packet(_context, pkt);

        av_packet_unref(pkt);

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
        if(ret)
        {
            char er[AV_ERROR_MAX_STRING_SIZE]={0};
            av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, ret);
            ADM_warning("Ignoring error %d submitting packet to decoder (\"%s\")\n",ret,er);
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

    if(frame->format != AV_PIX_FMT_VIDEOTOOLBOX)
    {
        ADM_warning("No hw image in the AVFrame\n");
        return false;
    }

    AVFrame *copy = swframes[swframeIdx];
    swframeIdx++;
    swframeIdx %= NB_SW_FRAMES;
    if(!copy)
        return false;

    av_frame_unref(copy);

    ret = av_hwframe_transfer_data(copy, frame, 0);

    if(ret)
    {
        char er[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, ret);
        ADM_error("Error %d downloading from hw surface (\"%s\")\n", ret, er);
        return false;
    }

    av_frame_copy_props(copy, frame);

    bool swap = false;
    ADM_pixelFormat pix_fmt;
    pix_fmt = _parent->admPixFrmtFromLav((AVPixelFormat)copy->format, &swap);
    if (pix_fmt == ADM_PIXFRMT_INVALID)
    {
        printf("[decoderFFVT::uncompress] Unhandled pixel format: %d\n", copy->format);
        return false;
    }
    out->_pixfrmt = pix_fmt;
    _parent->clonePic(copy, out, swap);

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
    {
        delete dec;
        dec = NULL;
        return NULL;
    }
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
