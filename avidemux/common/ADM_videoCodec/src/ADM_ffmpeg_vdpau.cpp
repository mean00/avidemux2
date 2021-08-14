/***************************************************************************
            \file              ADM_ffmpeg_vdpau.cpp  
            \brief Decoder using half ffmpeg/half &vdpau

    The ffmpeg part is to preformat inputs for &vdpau
    &vdpau is loaded dynamically to be able to make a binary
        and have something working even if the target machine
        does not have vdpau
    Some part, especially get/buffer and ip_age borrowed from xbmc
        as the api from ffmpeg is far from clear....


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
#include "BVector.h"
#include "ADM_default.h"

#ifdef USE_VDPAU
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/vdpau.h"
#include "libavutil/buffer.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "DIA_coreToolkit.h"
#include "ADM_dynamicLoading.h"
#include "ADM_render/GUI_render.h"
#include "../private_inc/ADM_ffmpeg_vdpau_internal.h"
#include "prefs.h"
#include "ADM_coreVdpau.h"
#include "ADM_threads.h"
#include "prefs.h"

#if defined(__sun__)
#include <alloca.h>
#endif /* : defined(__sun__) */

static bool         vdpauWorking=false;
static admMutex     surfaceMutex;

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

typedef enum 
{
    ADM_VDPAU_INVALID=0,
    ADM_VDPAU_H264=1,
    ADM_VDPAU_MPEG2=2,
    ADM_VDPAU_VC1=3,
    ADM_VDPAU_H265=4,
}ADM_VDPAU_TYPE;

#ifdef USE_VDPAU
#include "ADM_ffmpeg_vdpau_utils.cpp"
#endif


/**
    \fn markSurfaceUsed
    \brief mark the surfave as used. Can be called multiple time.
*/
static bool vdpauMarkSurfaceUsed(void *v, void * cookie)
{
    vdpauContext *vd=(vdpauContext*)v;
    ADM_vdpauRenderState *render=(ADM_vdpauRenderState *)cookie;
    render->refCount++;
    return true;
}
/**
    \fn markSurfaceUnused
    \brief mark the surfave as unused by the caller. Can be called multiple time.
*/
static bool vdpauMarkSurfaceUnused(void *v, void * cookie)
{
    vdpauContext *vd=(vdpauContext*)v;
    ADM_vdpauRenderState *render=(ADM_vdpauRenderState *)cookie;
    render->refCount--;
    if(!render->refCount)
    {
        surfaceMutex.lock();
        vd->freeQueue.push_back(render);
        surfaceMutex.unlock();
    }
    return true;
}
/**
    \fn vdpauRefDownload
    \brief Convert a &vdpau image to a regular image
*/

static bool vdpauRefDownload(ADMImage *image, void *instance, void *cookie)
{
    vdpauContext *vd=(vdpauContext*)instance;
    ADM_vdpauRenderState *render=(ADM_vdpauRenderState *)cookie;
    ADM_assert(render->refCount);
    ADM_assert(image->refType==ADM_HW_VDPAU);
    
    // other part will be done in goOn
    VdpVideoSurface  surface;

    surface=render->surface;
    uint8_t *planes[3];
    uint32_t stride[3];
    int      istride[3];

     image->GetWritePlanes(planes);
     image->GetPitches(istride);
     
     for(int i=0;i<3;i++) stride[i]=(uint32_t)istride[i];

    //ADM_info("Getting surface %d\n",(int)surface);
    // Copy back the decoded image to our output ADM_image
    aprintf("[VDPAU] Getting datas from surface %d\n",surface);
 
    VdpStatus status=admVdpau::getDataSurface(surface,planes, stride );
    if(VDP_STATUS_OK!=status)
    {
        ADM_error("[VDPAU] Cannot get data from surface <%s>\n",admVdpau::getErrorString(status));
    }
    image->refType=ADM_HW_NONE;
    bool r=vdpauMarkSurfaceUnused(instance,cookie);
    if(r==false || status!=VDP_STATUS_OK) 
    {
        ADM_warning("Cannot get VDPAU content from surface %d\n",(int)surface);
        return false;
    }
    return true;
}

/**
    \fn getBuffer
    \brief returns a &vdpau render masquerading as a AVFrame
*/
int decoderFFVDPAU::getBuffer(AVCodecContext *avctx, AVFrame *pic)
{
    ADM_vdpauRenderState *render=NULL;
    if(vdpau.freeQueue.size()==0)
    {
        ADM_info("[VDPAU] No more available surface, creating a new one\n");
        render=new ADM_vdpauRenderState;
        memset(render,0,sizeof( ADM_vdpauRenderState));
        render->surface=VDP_INVALID_HANDLE;
        int widthToUse = admVdpau::dimensionRoundUp(avctx->coded_width);
        int heightToUse= admVdpau::dimensionRoundUp(avctx->coded_height);
        if(VDP_STATUS_OK!=admVdpau::surfaceCreate(widthToUse,heightToUse,&(render->surface)))
        {
            ADM_error("Cannot create surface \n");
            delete render;
            alive=false;
            return -1;
        }
        surfaceMutex.lock();
        vdpau.fullQueue.push_back(render);
        surfaceMutex.unlock();
    }else
    {
        surfaceMutex.lock();
        render=vdpau.freeQueue.front();
        ADM_assert(render);
        vdpau.freeQueue.erase(vdpau.freeQueue.begin());
        surfaceMutex.unlock();
    }
    render->refCount=0;
    render->state=0;  
    vdpauMarkSurfaceUsed(&vdpau,(void *)render); // it is out of the queue, no need to lock it
    // It only works because surface is the 1st field of render!
    pic->buf[0]=av_buffer_create((uint8_t *)&(render->surface),  // Maybe a memleak here...
                                     sizeof(render->surface),
                                     ADM_VDPAUreleaseBuffer, 
                                     (void *)this,
                                     AV_BUFFER_FLAG_READONLY);

    pic->data[0]=(uint8_t *)render;
    pic->data[3]=(uint8_t *)(uintptr_t)render->surface;
    pic->reordered_opaque= avctx->reordered_opaque;
    aprintf("Alloc ===> Got buffer = %d\n",render->surface);
    return 0;
}
/**
    \fn releaseBuffer
*/
void decoderFFVDPAU::releaseBuffer(struct ADM_vdpauRenderState *rdr)
{
  ADM_assert(rdr);
  ADM_assert(rdr->refCount);
  rdr->state &= ~FF_VDPAU_STATE_USED_FOR_REFERENCE;
  aprintf("Release ===> Release buffer = %d\n",rdr->surface);
  vdpauMarkSurfaceUnused(&vdpau,(void *)rdr);
}

extern "C"
{

static enum AVPixelFormat vdpauGetFormat(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("[vdpau]: GetFormat\n");
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    AVPixelFormat outPix;
    int maxW,maxH;
    if(avctx->sw_pix_fmt==AV_PIX_FMT_YUV420P) // doont even try non yv12 for the moment
        for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
        {
            c=fmt[i];
            ADM_info("[vdpau]: Evaluating %d\n",c);
            if(c!=AV_PIX_FMT_VDPAU) continue;        
    #define FMT_V_CHECK(x,y) case AV_CODEC_ID_##x: \
            if(admVdpau::queryDecoderCapabilities(y,&maxW,&maxH)) \
            { \
                if(avctx->width > maxW) { ADM_warning(#x ": width %d exceeds max %u supported by HW\n",avctx->width,maxW); break; } \
                if(avctx->height > maxH) { ADM_warning(#x ": height %d exceeds max %u supported by HW\n",avctx->height,maxH); break; } \
                id=avctx->codec_id; \
                outPix=AV_PIX_FMT_VDPAU; \
            }else \
            {   ADM_info(#x ":Not supported by HW\n"); } \
            break;
            switch(avctx->codec_id)
            {
                FMT_V_CHECK(H264,      VDP_DECODER_PROFILE_H264_HIGH)
                FMT_V_CHECK(HEVC,      VDP_DECODER_PROFILE_HEVC_MAIN) 
                FMT_V_CHECK(MPEG1VIDEO,VDP_DECODER_PROFILE_MPEG1)
                FMT_V_CHECK(MPEG2VIDEO,VDP_DECODER_PROFILE_MPEG2_MAIN)
                FMT_V_CHECK(WMV3,      VDP_DECODER_PROFILE_VC1_MAIN)
                FMT_V_CHECK(VC1,       VDP_DECODER_PROFILE_VC1_MAIN)
                default: 
                    continue;
                    break;
            }
            break;
        }
    if(id==AV_CODEC_ID_NONE)
    {
        ADM_info("No matching colrospace compatible hw accelerator found \n");
        return AV_PIX_FMT_NONE;
    }
    // Finish intialization of Vdpau decoder
#if 0 // The lavc functions we rely on in ADM_acceleratedDecoderFF::parseHwAccel are no more
    const AVHWAccel *accel=ADM_acceleratedDecoderFF::parseHwAccel(outPix,id,AV_PIX_FMT_VDPAU);
    if(accel)
    {
        ADM_info("Found matching hw accelerator : %s\n",accel->name);
        ADM_info("Successfully setup hw accel\n");
        return AV_PIX_FMT_VDPAU;
    }
    ADM_info("No matching hw accelerator Found \n");
    return AV_PIX_FMT_NONE;
#endif
    return AV_PIX_FMT_VDPAU;
}
}

/**
 * 
 * @return 
 */
bool decoderFFVDPAU::initVdpContext()
{         
        _context->slice_flags     = SLICE_FLAG_CODED_ORDER|SLICE_FLAG_ALLOW_FIELD;           
        AVVDPAUContext *v= av_alloc_vdpaucontext();;
        _context->hwaccel_context = v;
        v->render=NULL;
        v->decoder=VDP_INVALID_HANDLE; 
        
        if(0>av_vdpau_bind_context(_context, (VdpDevice)(uint64_t) admVdpau::getVdpDevice(),
                          vdpGetProcAddressWrapper, AV_HWACCEL_FLAG_IGNORE_LEVEL))
        {
             ADM_error("Cannot bind\n");
                return false;
        }
        ADM_info("Init VDP context ok\n");
        return true;
}
/**
    \fn decoderFF&VDPAU
*/
decoderFFVDPAU::decoderFFVDPAU(struct AVCodecContext *avctx,decoderFF *parent) : ADM_acceleratedDecoderFF(avctx,parent)
{
        alive=true;
        const char *name="";
        VdpDevice dev=(VdpDevice)(uint64_t)admVdpau::getVdpDevice();
        
        switch(_context->codec_id)
        {
            case AV_CODEC_ID_HEVC:
                 name="h265";
                 break;
            case AV_CODEC_ID_H264:
                 name="h264";
                 break;
            case AV_CODEC_ID_VC1:
                  name="vc1";
                  break;
            case AV_CODEC_ID_MPEG1VIDEO:
            case AV_CODEC_ID_MPEG2VIDEO:
                  name="mpegvideo";
                  break;
            case AV_CODEC_ID_WMV3:
                  name="wm3_vdpau";
                  break;
            default:
                ADM_assert(0);
                break;
        }

     
        if(!initVdpContext())
        {
            ADM_warning("Cannot setup context\n");
            alive=false;
            return ;
        }
              
        _context->get_buffer2     = ADM_VDPAUgetBuffer;
        _context->draw_horiz_band = NULL;
        ADM_info("Successfully setup hw accel\n");              
}
/**
    \fn ~            void    goOn( const AVFrame *d);
*/
decoderFFVDPAU::~decoderFFVDPAU()
{
        // Delete free only, the ones still used will be deleted later
        int n=vdpau.fullQueue.size();        
        for(int i=0;i<n;i++)
        {
            ADM_vdpauRenderState *r=vdpau.fullQueue[i];
            if(r)
            {
                if(r->surface!=VDP_INVALID_HANDLE)
                {
                        if(VDP_STATUS_OK!=admVdpau::surfaceDestroy((r->surface)))
                                ADM_error("Error destroying surface %d\n",i);
                }
                delete r;
            }
        }
        vdpau.fullQueue.clear();
}
/**
    \fn uncompress
*/
bool decoderFFVDPAU::uncompress (ADMCompressedImage * in, ADMImage * out)
{
    
    aprintf("==> uncompress %s\n",_context->codec->long_name);
    if(out->refType==ADM_HW_VDPAU)
    {
            vdpauMarkSurfaceUnused(&vdpau,out->refDescriptor.refHwImage);
            out->refType=ADM_HW_NONE;
    }

    if(!_parent->getDrainingState() && !in->dataLength) // Null frame, silently skipped
    {
        out->_noPicture = 1;
        out->Pts=ADM_COMPRESSED_NO_PTS;
        ADM_info("[VDpau] Nothing to decode -> no Picture\n");
        return true;
    }

   // Put a safe value....
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
        if(ret)
        {
            char er[AV_ERROR_MAX_STRING_SIZE]={0};
            av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, ret);
            ADM_warning("Ignoring error %d submitting packet to decoder (\"%s\")\n",ret,er);
        }
        av_packet_unref(pkt);
    }else
    {
        handover=false;
    }

    ret = avcodec_receive_frame(_context, frame);

    if(!_parent->decodeErrorHandler(ret))
        return false;

    if(frame->pict_type==AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture=true;
        out->Pts= (uint64_t)(frame->reordered_opaque);
        ADM_info("[VDPAU] No picture \n");
        return false;
    }
    return readBackBuffer(frame,in,out);
}


 /**
  * \fn readBackBuffer
  * @param decodedFrame
  * @param in
  * @param out
  * @return 
  */   
bool     decoderFFVDPAU::readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage * in, ADMImage * out)
{
   // VdpSurface *surface=decodedFrame->data[3];
    struct ADM_vdpauRenderState *rndr = (struct ADM_vdpauRenderState *)decodedFrame->data[0];
    ADM_assert(rndr);
    aprintf("Decoding ===> Got surface = %d\n",rndr->surface);
    out->refType=ADM_HW_VDPAU;
    out->refDescriptor.refHwImage=(void *)rndr;
    out->refDescriptor.refCodec=&vdpau;
    out->refDescriptor.refMarkUsed=vdpauMarkSurfaceUsed;
    out->refDescriptor.refMarkUnused=vdpauMarkSurfaceUnused;
    out->refDescriptor.refDownload=vdpauRefDownload;
    vdpauMarkSurfaceUsed(&vdpau,(void *)rndr);
    
    uint64_t pts_opaque=(uint64_t)(decodedFrame->reordered_opaque);
    out->Pts= (uint64_t)(pts_opaque);    
    out->flags=admFrameTypeFromLav(decodedFrame);
    out->_range=(decodedFrame->color_range==AVCOL_RANGE_JPEG)? ADM_COL_RANGE_JPEG : ADM_COL_RANGE_MPEG;
    return true;
}

class ADM_hwAccelEntryVdpau : public ADM_hwAccelEntry
{
public: 
                            ADM_hwAccelEntryVdpau();
     virtual bool           canSupportThis(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat);
     virtual                ADM_acceleratedDecoderFF *spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt );     
     virtual                ~ADM_hwAccelEntryVdpau() {};
};
/**
 * 
 */
ADM_hwAccelEntryVdpau::ADM_hwAccelEntryVdpau()
{
    name="vdpau";
}
/**
 * 
 * @param avctx
 * @param fmt
 * @param outputFormat
 * @return 
 */
bool           ADM_hwAccelEntryVdpau::canSupportThis(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat)
{
    bool enabled=false;
    prefs->get(FEATURES_VDPAU,&enabled);
    if(!enabled)
    {
        ADM_info("Vdpau not enabled\n");
        return false;
    }
    
    enum AVPixelFormat ofmt=vdpauGetFormat(avctx,fmt);
    if(ofmt==AV_PIX_FMT_NONE)
        return false;
    outputFormat=ofmt;
    ADM_info("This is supported by VDPAU\n");
    return true;
}
ADM_acceleratedDecoderFF *ADM_hwAccelEntryVdpau::spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt )
{
    decoderFF *ff=(decoderFF *)avctx->opaque;
    return new decoderFFVDPAU(avctx,ff);
}
static ADM_hwAccelEntryVdpau vdpEntry;
/**
 * 
 * @return 
 */
bool initVDPAUDecoder(void)
{
    ADM_info("Registering VDPAU hw decocer\n");
    ADM_hwAccelManager::registerDecoder(&vdpEntry);
    return true;
}
#endif
// EOF
