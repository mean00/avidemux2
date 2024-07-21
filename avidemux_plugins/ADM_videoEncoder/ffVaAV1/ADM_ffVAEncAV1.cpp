/***************************************************************************
                          \fn ADM_ffVaEnc
                          \brief Front end for libavcodec Mpeg4 asp encoder
                             -------------------

    copyright            : (C) 2002/2009 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_ffVAEncAV1.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
//#define USE_VBR
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

extern "C"
{
    #include "libavutil/opt.h"
#include "libavutil/hwcontext.h"
#include "libavutil/hwcontext_vaapi.h"
}

ffvaAV1_encoder VaEncAV1Settings = VAENC_AV1_CONF_DEFAULT;

/**
    \fn ADM_ffVAEncAV1
*/
ADM_ffVAEncAV1::ADM_ffVAEncAV1(ADM_coreVideoFilter *src, bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,globalHeader)
{
    ADM_info("Creating.\n");
    hwDeviceCtx=NULL;
    swFrame=NULL;
    hwFrame=NULL;
}

/**
    \fn configureContext
    \brief Set necessary fields in AVCodecContext and allocate hw frame pool.
*/
bool ADM_ffVAEncAV1::configureContext(void)
{
    ADM_info("Configuring context for VAAPI encoder\n");
    ADM_info("Our display: %#x\n",admLibVA::getDisplay());

    switch(VaEncAV1Settings.rc_mode)
    {
        case ADM_FFVAENC_RC_CRF:
            _context->global_quality=VaEncAV1Settings.quality;
            break;
        case ADM_FFVAENC_RC_VBR:
#ifdef USE_VBR
            _context->bit_rate=VaEncAV1Settings.bitrate*1000;
            _context->rc_max_rate=VaEncAV1Settings.max_bitrate*1000;
            break;
#endif
        case ADM_FFVAENC_RC_CBR:
            _context->bit_rate=VaEncAV1Settings.bitrate*1000;
            _context->rc_max_rate=_context->bit_rate;
            break;
        default:
            ADM_error("Unknown rate control mode %u\n",VaEncAV1Settings.rc_mode);
            return false;
    }

    _context->pix_fmt =AV_PIX_FMT_VAAPI;

#define CLEARTEXT(x) char buf[AV_ERROR_MAX_STRING_SIZE]={0}; av_make_error_string(buf,AV_ERROR_MAX_STRING_SIZE,x);
    hwDeviceCtx = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_VAAPI);
    if(!hwDeviceCtx)
    {
        ADM_error("Cannot allocate hw device context.\n");
        return false;
    }

    AVHWDeviceContext *hwctx = (AVHWDeviceContext *)hwDeviceCtx->data;
    AVVAAPIDeviceContext *vactx = (AVVAAPIDeviceContext *)hwctx->hwctx;
    vactx->display = admLibVA::getDisplay();

    int err = av_hwdevice_ctx_init(hwDeviceCtx);
    if(err)
    {
        CLEARTEXT(err)
        ADM_warning("Cannot initialize VAAPI hwdevice (%d, %s)\n",err,buf);
        return false;
    }

    AVBufferRef *hwFramesRef = NULL;
    AVHWFramesContext *hwFramesCtx = NULL;
    hwFramesRef = av_hwframe_ctx_alloc(hwDeviceCtx);
    if(!hwFramesRef)
    {
        ADM_error("Cannot create VAAPI frame context.\n");
        return false;
    }
    hwFramesCtx=(AVHWFramesContext*)(hwFramesRef->data);
    hwFramesCtx->format=AV_PIX_FMT_VAAPI;
    hwFramesCtx->sw_format=AV_PIX_FMT_NV12;
    hwFramesCtx->width = getWidth();
    hwFramesCtx->height = getHeight();
    hwFramesCtx->initial_pool_size=20;
    err = av_hwframe_ctx_init(hwFramesRef);
    if(err<0)
    {
        CLEARTEXT(err)
        ADM_error("Cannot initialize VAAPI frame context (%d, %s)\n",err,buf);
        av_buffer_unref(&hwFramesRef);
        return false;
    }
    _context->hw_frames_ctx = av_buffer_ref(hwFramesRef);
    if(!_context->hw_frames_ctx)
    {
        ADM_error("hw_frames_ctx is NULL!\n");
        return false;
    }
    av_buffer_unref(&hwFramesRef);

    swFrame = av_frame_alloc();

    if(!swFrame)
    {
        ADM_error("Could not allocate sw frame\n");
        return false;
    }

    swFrame->width = getWidth();
    swFrame->height = getHeight();
    swFrame->format = AV_PIX_FMT_NV12;

    err = av_frame_get_buffer(swFrame, 64);

    if(err<0)
    {
        CLEARTEXT(err)
        ADM_warning("get buffer for sw frame failed with error code %d (%s)\n",err,buf);
        return false;
    }

    hwFrame = av_frame_alloc();

    if(!hwFrame)
    {
        ADM_error("Could not allocate hw frame\n");
        return false;
    }

    return true;
}

/**
    \fn setup
*/
bool ADM_ffVAEncAV1::setup(void)
{
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName("av1_vaapi"))
    {
        ADM_info("[ffMpeg] Setup failed\n");
        return false;
    }

    ADM_info("[ffMpeg] Setup ok\n");

    return true;
}


/**
    \fn ~ADM_ffVAEncAV1
*/
ADM_ffVAEncAV1::~ADM_ffVAEncAV1()
{
    ADM_info("[ffVAEncAV1] Destroying.\n");
    if(swFrame)
        av_frame_free(&swFrame);
    if(hwFrame)
        av_frame_free(&hwFrame);
    if(hwDeviceCtx)
    {
        av_buffer_unref(&hwDeviceCtx);
        hwDeviceCtx=NULL;
    }
}

/**
    \fn preEncode
    \brief Get next picture and upload it to a hw surface
*/
bool ADM_ffVAEncAV1::preEncode(void)
{
    uint32_t nb;
    if(source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[ffVAEncAV1] Cannot get next image\n");
        return false;
    }

    if(image->_width != getWidth() || image->_height != getHeight())
    {
        ADM_error("[ffVAEncAV1] Input picture size mismatch: expected %d x %d, got %d x %d\n", getWidth(), getHeight(), image->_width, image->_height);
        return false;
    }

    image->convertToNV12(swFrame->data[0],swFrame->data[1],swFrame->linesize[0],swFrame->linesize[1]);

    av_frame_unref(hwFrame);

    hwFrame->width = getWidth();
    hwFrame->height = getHeight();
    hwFrame->format = AV_PIX_FMT_VAAPI;

    int err = av_hwframe_get_buffer(_context->hw_frames_ctx, hwFrame, 0);

    if(err<0)
    {
        CLEARTEXT(err)
        ADM_warning("get buffer for hw frame failed with error code %d (%s)\n",err,buf);
        return false;
    }

    err=av_hwframe_transfer_data(hwFrame, swFrame, 0);
    if(err<0)
    {
        CLEARTEXT(err)
        ADM_warning("data transfer to the hw frame failed with error code %d (%s)\n",err,buf);
        return false;
    }

    uint64_t p=image->Pts;
    queueOfDts.push_back(p);
    aprintf("Incoming frame PTS=%" PRIu64", delay=%" PRIu64"\n",p,getEncoderDelay());
    p+=getEncoderDelay();
    hwFrame->pts=timingToLav(p);
    if(!hwFrame->pts)
        hwFrame->pts=AV_NOPTS_VALUE;

    ADM_timeMapping map; // Store real PTS <->lav value mapping
    map.realTS=p;
    map.internalTS=hwFrame->pts;
    mapper.push_back(map);

    return true;
}

/**
    \fn encode
*/
bool ADM_ffVAEncAV1::encode (ADMBitstream * out)
{
int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop - out the frames stored in the queue due to B-frames
    {
        sz=encodeWrapper(NULL,out);
        if (sz<= 0)
        {
            if(sz<0)
                ADM_info("[ffVAEncAV1] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[ffVAEncAV1] Popping delayed bframes (%d)\n",sz);
        goto link;
        return false;
    }

    q=image->_Qp;
    if(!q) q=2;
    aprintf("[CODEC] Flags=%#x, QSCALE=%x, bit_rate=%d, quality=%d, qz=%d, incoming qz=%d\n",
        _context->flags,
        AV_CODEC_FLAG_QSCALE,
        _context->bit_rate,
        hwFrame->quality,
        hwFrame->quality / FF_QP2LAMBDA,
        q);

    sz=encodeWrapper(hwFrame,out);
    if(sz<0)
    {
        CLEARTEXT(sz)
        ADM_warning("[ffVAEncAV1] Error %d (%s) encoding video\n",sz,buf);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
    aprintf("[ffVAEncAV1] encoder produces %d bytes\n",sz);
link:
    return postEncode(out,sz);
}

/**
    \fn ffVAEncAV1Configure
    \brief UI configuration for ffVAEncAV1 encoder
*/

bool ffVAEncAV1Configure(void)
{
    ffvaAV1_encoder *conf=&VaEncAV1Settings;

    diaMenuEntry rateControlMode[]={
        {ADM_FFVAENC_RC_CRF,QT_TRANSLATE_NOOP("ffVAEncAV1","Constant Rate Factor"),NULL},
        {ADM_FFVAENC_RC_CBR,QT_TRANSLATE_NOOP("ffVAEncAV1","Constant Bitrate"),NULL},
#ifdef USE_VBR
        {ADM_FFVAENC_RC_VBR,QT_TRANSLATE_NOOP("ffVAEncAV1","Variable Bitrate"),NULL}
#endif
    };

#define PX(x) &(conf->x)
#define NB_ELEM(x) (sizeof(x) / sizeof(diaMenuEntry))

    diaElemMenu rcMode(PX(rc_mode), QT_TRANSLATE_NOOP("ffVAEncAV1","Rate Control:"), NB_ELEM(rateControlMode), rateControlMode);
    diaElemUInteger quality(PX(quality), QT_TRANSLATE_NOOP("ffVAEncAV1","Quality:"),1,51);
    diaElemUInteger bitrate(PX(bitrate), QT_TRANSLATE_NOOP("ffVAEncAV1","Bitrate (kbps):"),1,50000);
#ifdef USE_VBR
    diaElemUInteger maxBitrate(PX(max_bitrate),QT_TRANSLATE_NOOP("ffVAEncAV1","Max Bitrate (kbps):"),1,50000);
#endif
    diaElemUInteger gopSize(PX(gopsize),QT_TRANSLATE_NOOP("ffVAEncAV1","GOP Size:"),1,250);

    diaElemFrame rateControl(QT_TRANSLATE_NOOP("ffVAEncAV1","Rate Control"));
    diaElemFrame frameControl(QT_TRANSLATE_NOOP("ffVAEncAV1","Frame Control"));

    rateControl.swallow(&rcMode);
    rateControl.swallow(&quality);
    rateControl.swallow(&bitrate);
#ifdef USE_VBR
    rateControl.swallow(&maxBitrate);

    rcMode.link(rateControlMode+2,1,&bitrate);
    rcMode.link(rateControlMode+2,1,&maxBitrate);
#endif
    rcMode.link(rateControlMode,1,&quality);
    rcMode.link(rateControlMode+1,1,&bitrate);

    frameControl.swallow(&gopSize);

    diaElem *diamode[] = {&rateControl,&frameControl};

#undef NB_ELEM
#define NB_ELEM(x) (sizeof(x) / sizeof(diaElem *))

    if (diaFactoryRun(QT_TRANSLATE_NOOP("ffVAEncAV1","FFmpeg VA-API AV1 Encoder Configuration"), NB_ELEM(diamode), diamode))
    {
        return true;
    }
    return false;
}
// EOF
