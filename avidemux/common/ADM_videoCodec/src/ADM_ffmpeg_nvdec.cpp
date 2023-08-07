/***************************************************************************
    \file       ADM_ffmpeg_nvdec.cpp
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

#ifdef USE_NVENC
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixdesc.h"
#include "libavutil/hwcontext.h"
#include "dynlink_loader.h"
}

#include "ADM_codec.h"
#include "ADM_ffmp43.h"
#include "ADM_hwAccel.h"
#include "ADM_image.h"
#include "prefs.h"

#include "../private_inc/ADM_ffmpeg_nvdec_internal.h"

#define H264_IS_SUPPORTED           (1 << 0)
#define HEVC_IS_SUPPORTED           (1 << 1)
#define HEVC_10BITS_IS_SUPPORTED    (1 << 2)
#define VP9_IS_SUPPORTED            (1 << 3)
#define VP9_10BITS_IS_SUPPORTED     (1 << 4)
#define AV1_IS_SUPPORTED            (1 << 5)
#define AV1_10BITS_IS_SUPPORTED     (1 << 6)
#define NOT_PROBED 0xFFFFFF80

static uint32_t nvDecCodecSupportFlags = NOT_PROBED;

/**
    \fn nvDecMarkSurfaceUsed
    \brief Mark surface as used by the caller.
*/
static bool nvDecMarkSurfaceUsed(void *v, void *cookie)
{
    decoderFFnvDec *inst = (decoderFFnvDec *)v;
    ADM_nvDecRef *render = (ADM_nvDecRef *)cookie;
    return inst->markRefUsed(render);
}
/**
    \fn nvDecMarkSurfaceUnused
    \brief Mark surface as unused by the caller.
*/
static bool nvDecMarkSurfaceUnused(void *v, void *cookie)
{
    decoderFFnvDec *inst = (decoderFFnvDec *)v;
    ADM_nvDecRef *render = (ADM_nvDecRef *)cookie;
    return inst->markRefUnused(render);
}
/**
    \fn nvDecRefDownload
    \brief Transfer decoded picture from hw surface to main memory.
*/
static bool nvDecRefDownload(ADMImage *image, void *v, void *cookie)
{
    decoderFFnvDec *inst = (decoderFFnvDec *)v;
    ADM_nvDecRef *render = (ADM_nvDecRef *)cookie;
    return inst->downloadFromRef(render, image);
}

extern "C"
{
static enum AVPixelFormat ADM_nvDecGetFormat(struct AVCodecContext *avctx, const enum AVPixelFormat *fmt)
{
    int i;
    ADM_info("[NVDEC]: GetFormat\n");
    AVCodecID id=AV_CODEC_ID_NONE;
    AVPixelFormat c;
    AVPixelFormat outPix;
    AVBufferRef *hwDevRef;

    switch(avctx->sw_pix_fmt)
    {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_YUVJ420P:
        case AV_PIX_FMT_YUV420P10LE:
        case AV_PIX_FMT_NV12:
        case AV_PIX_FMT_P010:
            break;
        default:
            ADM_warning("[NVDEC] Unsupported sw pixel format %d (%s)\n", avctx->sw_pix_fmt, av_get_pix_fmt_name(avctx->sw_pix_fmt));
            return AV_PIX_FMT_NONE;
    }

    for(i=0;fmt[i]!=AV_PIX_FMT_NONE;i++)
    {
        c=fmt[i];
        char name[300]={0};
        av_get_pix_fmt_string(name,sizeof(name),c);
        ADM_info("[NVDEC]: Evaluating PIX_FMT %d,%s\n",c,name);
        snprintf(name,300,"%s",avcodec_get_name(avctx->codec_id));
        ADM_info("\t  Evaluating codec %d, %s\n",avctx->codec_id,name);
        if(c!=AV_PIX_FMT_CUDA) continue;
#define FMT_V_CHECK(x,y) case AV_CODEC_ID_##x: outPix=AV_PIX_FMT_CUDA; id=avctx->codec_id; break;

        switch(avctx->codec_id)
        {
            FMT_V_CHECK(H264,H264)
            FMT_V_CHECK(H265,H265)
            FMT_V_CHECK(MPEG1VIDEO,MPEG1)
            FMT_V_CHECK(MPEG2VIDEO,MPEG2)
            FMT_V_CHECK(VC1,VC1)
            FMT_V_CHECK(VP9,VP9)
            FMT_V_CHECK(AV1,AV1)
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
    if(avctx->hw_device_ctx)
    {
        ADM_info("hw device context already exists\n");
        return AV_PIX_FMT_CUDA;
    }
    // Finish intialization of NVDEC decoder
    hwDevRef = NULL;
    int err = av_hwdevice_ctx_create(&hwDevRef, AV_HWDEVICE_TYPE_CUDA, NULL, NULL, 0);
    if(err < 0)
    {
        ADM_error("Cannot initialize NVDEC\n");
        avctx->hw_device_ctx = NULL;
        return AV_PIX_FMT_NONE;
    }
    avctx->hw_device_ctx = av_buffer_ref(hwDevRef);

    return AV_PIX_FMT_CUDA;
}
}

/**
    \fn ctor
*/
decoderFFnvDec::decoderFFnvDec(struct AVCodecContext *avctx, decoderFF *parent) : ADM_acceleratedDecoderFF(avctx,parent)
{
    renderMutex = new admMutex("NVDEC surface mutex");
    color8bits = NULL;
    color10bits = NULL;
    alive = false;
    if(avctx->hw_device_ctx)
        alive = true;
    ADM_info("NVDEC hw accel decoder object created with hw dev ctx at %p\n", avctx->hw_device_ctx);
}
/**
    \fn dtor
*/
decoderFFnvDec::~decoderFFnvDec()
{
    int i;
    ADM_info("Destroying NVDEC decoder\n");
    // hw device context will be uninited and freed when AVCodecContext gets closed
    for(i = 0; i < pool.size(); i++)
    {
        ADM_nvDecRef *rdr = pool[i];
        if(!rdr) continue;
        if(!rdr->pic) continue;
        av_frame_free(&rdr->pic);
        delete rdr;
        rdr = NULL;
    }
    if(color8bits)
    {
        delete color8bits;
        color8bits = NULL;
    }
    if(color10bits)
    {
        delete color10bits;
        color10bits = NULL;
    }
    if(renderMutex)
    {
        delete renderMutex;
        renderMutex = NULL;
    }
}
/**
    \fn uncompress
*/
bool decoderFFnvDec::uncompress(ADMCompressedImage *in, ADMImage *out)
{
    if(!_parent->getDrainingState() && !in->dataLength) // Null frame, silently skipped
    {
        out->_noPicture = 1;
        out->Pts = ADM_COMPRESSED_NO_PTS;
        ADM_info("[NVDEC] Nothing to decode -> no picture\n");
        return true;
    }

    out->Pts = in->demuxerPts;
    _context->reordered_opaque = in->demuxerPts;

    AVFrame *frame = _parent->getFramePointer();
    ADM_assert(frame);

    int ret = 0;

    if(_parent->getDrainingState())
    {
        if(!_parent->getDrainingInitiated())
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

        if(in->flags & AVI_KEY_FRAME)
            pkt->flags = AV_PKT_FLAG_KEY;
        else
            pkt->flags = 0;

        ret = avcodec_send_packet(_context, pkt);

        av_packet_unref(pkt);

        if(ret == AVERROR_UNKNOWN)
        {
            ADM_warning("Unknown error from avcodec_send_packet, bailing out.\n");
            alive = false;
            return false;
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

    if(frame->pict_type == AV_PICTURE_TYPE_NONE)
    {
        out->_noPicture = true;
        out->Pts = (uint64_t)(frame->reordered_opaque);
        ADM_info("[NVDEC] No picture \n");
        return false;
    }

    if(frame->format != AV_PIX_FMT_CUDA)
    {
        ADM_warning("No hw image in the AVFrame\n");
        alive = false;
        return false;
    }

    return readBackBuffer(frame, in, out);
}

bool decoderFFnvDec::readBackBuffer(AVFrame *decodedFrame, ADMCompressedImage *in, ADMImage *out)
{
    AVFrame *copy = NULL;
    ADM_nvDecRef *rndr = NULL;

    renderMutex->lock();
    if(pool.size() < FRAMES_POOL_SIZE)
    {
        rndr = new ADM_nvDecRef;
        rndr->pic = NULL;
    }else
    {
        rndr = pool[0];
        pool.popFront();
    }
    renderMutex->unlock();
    if(rndr->pic)
        av_frame_unref(rndr->pic);
    else
        rndr->pic = av_frame_alloc();

    ADM_assert(rndr->pic);

    int ret = av_frame_ref(rndr->pic, decodedFrame);

    if(ret)
    {
        av_frame_free(&rndr->pic);
        char er[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, ret);
        ADM_error("Error %d referencing hw frame (\"%s\")\n", ret, er);
        delete rndr;
        rndr = NULL;
        return false;
    }

    rndr->refCount = 1;

    renderMutex->lock();
    pool.append(rndr);
    renderMutex->unlock();

    out->refType = ADM_HW_NVDEC;
    out->refDescriptor.refHwImage = (void *) rndr;
    out->refDescriptor.refCodec = this;
    out->refDescriptor.refMarkUsed = nvDecMarkSurfaceUsed;
    out->refDescriptor.refMarkUnused = nvDecMarkSurfaceUnused;
    out->refDescriptor.refDownload = nvDecRefDownload;

    if(_context->codec_id == AV_CODEC_ID_AV1)
        out->Pts = in->demuxerPts;
    else
        out->Pts = (uint64_t)(decodedFrame->reordered_opaque);
    out->flags = admFrameTypeFromLav(decodedFrame);
    _parent->cloneColorInfo(decodedFrame, out);
    bool swap = false;
    out->_pixfrmt = _parent->admPixFrmtFromLav(_context->sw_pix_fmt, &swap);
    return true;
}

bool decoderFFnvDec::markRefUsed(ADM_nvDecRef *render)
{
    ADM_assert(render);
    renderMutex->lock();
    render->refCount++;
    renderMutex->unlock();
    return true;
}

bool decoderFFnvDec::markRefUnused(ADM_nvDecRef *render)
{
    ADM_assert(render);
    renderMutex->lock();
    if(render->refCount < 1)
        ADM_warning("NVDEC render already marked as unused, nothing to do.\n");
    else
        render->refCount--;

    bool r = (render->refCount < 1);
    renderMutex->unlock();
    return r;
}

bool decoderFFnvDec::downloadFromRef(ADM_nvDecRef *render, ADMImage *image)
{
    ADM_assert(render);
    renderMutex->lock();

    if(render->pic->format != AV_PIX_FMT_CUDA)
    {
        ADM_warning("No CUVID hw surface in the render!\n");
        renderMutex->unlock();
        return false;
    }

    if(!image)
    {
        renderMutex->unlock();
        ADM_assert(image);
    }

    AVFrame *tmp = av_frame_alloc();
    if(!tmp)
    {
        renderMutex->unlock();
        ADM_assert(tmp);
    }

    int ret = av_hwframe_transfer_data(tmp, render->pic, 0);

    if(ret)
    {
        av_frame_free(&tmp);
        char er[AV_ERROR_MAX_STRING_SIZE]={0};
        av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, ret);
        ADM_error("Error %d downloading from hw surface (\"%s\")\n", ret, er);
        renderMutex->unlock();
        return false;
    }

    av_frame_copy_props(tmp, render->pic);

    ADMColorScalerSimple *converter = NULL;
    ADM_pixelFormat from = ADM_PIXFRMT_INVALID;

    switch(tmp->format)
    {
        case AV_PIX_FMT_NV12:
            converter = color8bits;
            from = ADM_PIXFRMT_NV12;
            break;
        case AV_PIX_FMT_P010:
            converter = color10bits;
            from = ADM_PIXFRMT_NV12_10BITS;
            break;
        default:
            printf("[decoderFFnvDec::downloadFromRef] Unhandled pixel format: %d (%s)\n", tmp->format, av_get_pix_fmt_name((AVPixelFormat)tmp->format));
            av_frame_free(&tmp);
            av_frame_unref(render->pic);
            renderMutex->unlock();
            return false;
    }

    bool swap = false;
    uint64_t restorePts = image->Pts;

    ADMImageRef ref(tmp->width, tmp->height);
    ref._pixfrmt = _parent->admPixFrmtFromLav(_context->sw_pix_fmt, &swap);
    _parent->clonePic(tmp, &ref, swap);

    if(!converter)
        converter = new ADMColorScalerSimple(ref._width, ref._height, from, ADM_PIXFRMT_YV12);
    if(from == ADM_PIXFRMT_NV12)
        color8bits = converter;
    else if(from == ADM_PIXFRMT_NV12_10BITS)
        color10bits = converter;

    for(int i=0; i < 2; i++)
    {
        ref._planes[i] = tmp->data[i];
        ref._planeStride[i] = tmp->linesize[i];
    }
    ref._planes[2] = NULL;
    ref._planeStride[2] = 0;

    converter->convertImage(&ref, image);

    // now we may free AVFrame holding buffers used by ref
    av_frame_free(&tmp);

    ref.Pts = restorePts;
    image->copyInfo(&ref);
    renderMutex->unlock();
    return true;
}

class ADM_hwAccelEntryNvDec : public ADM_hwAccelEntry
{
public:
                    ADM_hwAccelEntryNvDec();
     virtual bool   canSupportThis(struct AVCodecContext *avctx, const enum AVPixelFormat *fmt, enum AVPixelFormat &outputFormat);
     virtual        ADM_acceleratedDecoderFF *spawn(struct AVCodecContext *avctx, const enum AVPixelFormat *fmt);
     virtual        ~ADM_hwAccelEntryNvDec() {};
};
/**
 * 
 */
ADM_hwAccelEntryNvDec::ADM_hwAccelEntryNvDec()
{
    name="NVDEC";
}
/**
 * 
 * @param avctx
 * @param fmt
 * @param outputFormat
 * @return 
 */
bool ADM_hwAccelEntryNvDec::canSupportThis( struct AVCodecContext *avctx, const enum AVPixelFormat *fmt, enum AVPixelFormat &outputFormat )
{
#if 0
    bool enabled=true;
    prefs->get(FEATURES_VDPAU,&enabled);
    if(!enabled)
    {
        ADM_info("NVDEC not enabled\n");
        return false;
    }
#endif
    enum AVPixelFormat ofmt=ADM_nvDecGetFormat(avctx,fmt);
    if(ofmt==AV_PIX_FMT_NONE)
        return false;
    outputFormat=ofmt;
    ADM_info("Seems to be supported by NVDEC and Avidemux\n");
    return true;
}

ADM_acceleratedDecoderFF *ADM_hwAccelEntryNvDec::spawn( struct AVCodecContext *avctx, const enum AVPixelFormat *fmt )
{
    decoderFF *ff = (decoderFF *)avctx->opaque;
    ADM_assert(ff);
    decoderFFnvDec *dec = new decoderFFnvDec(avctx,ff);
    return (ADM_acceleratedDecoderFF *)dec;
}

static void printNvDecProbeResults(void)
{
#define SAY(feature) ((nvDecCodecSupportFlags & feature)? "supported" : "not supported")
    printf("\tH.264: %s\n\t"
        "HEVC: %s\n\t"
        "HEVC 10 bits: %s\n\t"
        "VP9: %s\n\t"
        "VP9 10 bits: %s\n\t"
        "AV1: %s\n\t"
        "AV1 10 bits: %s\n",
        SAY(H264_IS_SUPPORTED),
        SAY(HEVC_IS_SUPPORTED),
        SAY(HEVC_10BITS_IS_SUPPORTED),
        SAY(VP9_IS_SUPPORTED),
        SAY(VP9_10BITS_IS_SUPPORTED),
        SAY(AV1_IS_SUPPORTED),
        SAY(AV1_10BITS_IS_SUPPORTED));
#undef SAY
}

/**
 * \fn checkNvDec
 */
extern "C" {
static bool checkNvDec(void)
{
    // Code compiled from libavutil/hwcontext_cuda.c and libavcodec/nvdec.c
    int r;
    bool fatal;
    CudaFunctions *cudaFunc;
    CuvidFunctions *cuvidFunc;
    CUVIDDECODECAPS caps;
    CUdevice dev;
    CUcontext ctx;
    CUcontext dummy;

    if(!(nvDecCodecSupportFlags & NOT_PROBED))
    {
        ADM_info("Already probed, the results were:\n");
        printNvDecProbeResults();
        return (nvDecCodecSupportFlags & H264_IS_SUPPORTED);
    }
    nvDecCodecSupportFlags & ~NOT_PROBED;

    cudaFunc = NULL;

    r = cuda_load_functions(&cudaFunc, NULL);

    if(r < 0 || !cudaFunc)
    {
        ADM_info("Cannot load CUDA functions, NVDEC is not available.\n");
        return false;
    }

    if(!cudaFunc->cuInit || !cudaFunc->cuDeviceGet ||
       !cudaFunc->cuCtxCreate || !cudaFunc->cuCtxDestroy ||
       !cudaFunc->cuCtxPushCurrent || !cudaFunc->cuCtxPopCurrent)
    {
        ADM_warning("Necessary CUDA functions not found, bailing out.\n");
        goto fail2;
    }

    r = cudaFunc->cuInit(0);

    if(r < 0)
    {
        ADM_warning("Cannot init CUDA, NVDEC is not available.\n");
        goto fail2;
    }

    r = cudaFunc->cuDeviceGet(&dev, 0);

    if(r < 0)
    {
        ADM_warning("Cannot get CUDA device, NVDEC is not available.\n");
        goto fail2;
    }

    r = cudaFunc->cuCtxCreate(&ctx, CU_CTX_SCHED_BLOCKING_SYNC, dev);

    if(r < 0)
    {
        ADM_warning("Cannot create CUDA context, NVDEC is not available.\n");
        goto fail2;
    }

    cuvidFunc = NULL;

    r = cuvid_load_functions(&cuvidFunc, NULL);

    if(r < 0)
    {
        ADM_info("Cannot load CUVID functions, NVDEC is not available.\n");
        cudaFunc->cuCtxDestroy(ctx);
        goto fail2;
    }
    if(!cuvidFunc->cuvidGetDecoderCaps)
    {
        ADM_warning("Driver does not support quering decoder capabilities.\n");
        cuvid_free_functions(&cuvidFunc);
        cudaFunc->cuCtxDestroy(ctx);
        goto fail2;
    }

    r = cudaFunc->cuCtxPushCurrent(ctx);

    if(r < 0)
    {
        ADM_warning("Cannot push CUDA context, bailing out.\n");
        cuvid_free_functions(&cuvidFunc);
        cudaFunc->cuCtxDestroy(ctx);
        cuda_free_functions(&cudaFunc);
        return false;
    }

    memset(&caps, 0, sizeof(caps));

#define CHECK_CAPS(codec, chroma, bit_depth, flag, fatal) \
    caps.eCodecType      = cudaVideoCodec_##codec; \
    caps.eChromaFormat   = cudaVideoChromaFormat_##chroma; \
    caps.nBitDepthMinus8 = bit_depth - 8; \
    ADM_info("Checking " #codec " (%d bits)\n", bit_depth); \
    r = cuvidFunc->cuvidGetDecoderCaps(&caps); \
    if(r < 0) \
    { \
        ADM_warning("Quering decoder capabilities for " #codec " has failed.\n"); \
        if(fatal) \
        { \
            goto fail; \
        } \
    } else { \
        if(caps.bIsSupported) \
        { \
            nvDecCodecSupportFlags |= flag; \
        } else if(fatal) { \
            ADM_warning("Check for " #codec " has failed, skipping further checks.\n"); \
            goto fail; \
        } \
    }

    CHECK_CAPS(H264, 420,  8, H264_IS_SUPPORTED, 1)
    CHECK_CAPS(HEVC, 420,  8, HEVC_IS_SUPPORTED, 0)
    CHECK_CAPS(HEVC, 420, 10, HEVC_10BITS_IS_SUPPORTED, 0)
    CHECK_CAPS(VP9,  420,  8, VP9_IS_SUPPORTED, 0)
    CHECK_CAPS(VP9,  420, 10, VP9_10BITS_IS_SUPPORTED, 0)
    CHECK_CAPS(AV1,  420,  8, AV1_IS_SUPPORTED, 0)
    CHECK_CAPS(AV1,  420, 10, AV1_10BITS_IS_SUPPORTED, 0)

    ADM_info("NVDEC decoder probed, the results are:\n");
    printNvDecProbeResults();

fail:
    cuvid_free_functions(&cuvidFunc);
    cudaFunc->cuCtxPopCurrent(&dummy);
    cudaFunc->cuCtxDestroy(ctx);
fail2:
    cuda_free_functions(&cudaFunc);
    return (nvDecCodecSupportFlags & H264_IS_SUPPORTED);
}
}
/**
 * \fn nvDecProbe
 * \brief Try to load CUDA libs, return true if decoder supports at least H.264.
 */
bool nvDecProbe(void)
{
    return checkNvDec();
}

bool admNvDec_exitCleanup(void)
{
    return true; // FIXME
}

static ADM_hwAccelEntryNvDec nvDecEntry;

/**
 *
 */
bool initNvDecDecoder(void)
{
    ADM_info("Registering NVDEC hw decoder\n");
    ADM_hwAccelManager::registerDecoder(&nvDecEntry);
    return true;
}
#endif
// EOF
