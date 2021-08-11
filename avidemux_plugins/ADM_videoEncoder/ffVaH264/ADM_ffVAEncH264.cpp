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
#include "ADM_ffVAEncH264.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
//#define USE_NV12
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

ffvaenc_encoder VaEncSettings = VAENC_CONF_DEFAULT;

/**
        \fn ADM_ffVaEncEncoder
*/
ADM_ffVAEncH264Encoder::ADM_ffVAEncH264Encoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,globalHeader)
{
    ADM_info("Creating.\n");
    hwDeviceCtx=NULL;
    swFrame=NULL;
    hwFrame=NULL;
}

/**
    \fn pre-open
*/
bool ADM_ffVAEncH264Encoder::configureContext(void)
{
    ADM_info("Configuring context for VAAPI encoder\n");
    ADM_info("Our display: %#x\n",admLibVA::getDisplay());
    switch(VaEncSettings.profile)
    {
#define SAY(x) case FF_PROFILE_H264_##x: _context->profile=FF_PROFILE_H264_##x; break;
        SAY(CONSTRAINED_BASELINE)
        SAY(MAIN)
        SAY(HIGH)
        default:break;
#undef SAY
    }
    switch(VaEncSettings.rc_mode)
    {
        case ADM_FFVAENC_RC_CRF:
            _context->global_quality=VaEncSettings.quality;
            break;
        case ADM_FFVAENC_RC_VBR:
#ifdef USE_VBR
            _context->bit_rate=VaEncSettings.bitrate*1000;
            _context->rc_max_rate=VaEncSettings.max_bitrate*1000;
            break;
#endif
        case ADM_FFVAENC_RC_CBR:
            _context->bit_rate=VaEncSettings.bitrate*1000;
            _context->rc_max_rate=_context->bit_rate;
            break;
        default:
            ADM_error("Unknown rate control mode %u\n",VaEncSettings.rc_mode);
            return false;
    }
    _context->max_b_frames=VaEncSettings.bframes;
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
    hwFramesCtx->width=source->getInfo()->width;
    hwFramesCtx->height=source->getInfo()->height;
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
    return true;
}

/**
    \fn setup
*/
bool ADM_ffVAEncH264Encoder::setup(void)
{
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName("h264_vaapi"))
    {
        ADM_info("[ffMpeg] Setup failed\n");
        return false;
    }

    ADM_info("[ffMpeg] Setup ok\n");

    return true;
}


/**
    \fn ~ADM_ffVaEncEncoder
*/
ADM_ffVAEncH264Encoder::~ADM_ffVAEncH264Encoder()
{
    ADM_info("[ffVAEncH264] Destroying.\n");
    if(swFrame)
    {
        av_frame_free(&swFrame);
        swFrame=NULL;
    }
    if(hwFrame)
    {
        av_frame_free(&hwFrame);
        hwFrame=NULL;
    }
    if(hwDeviceCtx)
    {
        av_buffer_unref(&hwDeviceCtx);
        hwDeviceCtx=NULL;
    }
}

/**
    \fn getEncoderDelay
*/
uint64_t ADM_ffVAEncH264Encoder::getEncoderDelay(void)
{
    uint64_t inc=source->getInfo()->frameIncrement;
    if(VaEncSettings.profile && VaEncSettings.bframes)
        return inc*2;
    return 0;
}

/**
 */
bool             ADM_ffVAEncH264Encoder::preEncode(void)
{
    uint32_t nb;
    if(source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[ffVAEncH264] Cannot get next image\n");
        return false;
    }

    swFrame=av_frame_alloc();
    if(!swFrame)
    {
        ADM_error("Could not allocate sw frame\n");
        return false;
    }

    swFrame->width=source->getInfo()->width;
    swFrame->height=source->getInfo()->height;
    swFrame->format=AV_PIX_FMT_NV12;

    int err=av_frame_get_buffer(swFrame, 32);
    if(err<0)
    {
        CLEARTEXT(err)
        ADM_warning("get buffer for sw frame failed with error code %d (%s)\n",err,buf);
        return false;
    }

    swFrame->linesize[0] = swFrame->linesize[1] = image->GetPitch(PLANAR_Y);
    swFrame->linesize[2] = 0;
    swFrame->data[2] = NULL;
    image->convertToNV12(swFrame->data[0],swFrame->data[1],swFrame->linesize[0],swFrame->linesize[1]);

    if(hwFrame)
    {
        av_frame_free(&hwFrame);
        hwFrame=NULL;
    }
    hwFrame=av_frame_alloc();
    if(!hwFrame)
    {
        ADM_error("Could not allocate hw frame\n");
        return false;
    }

    hwFrame->width=source->getInfo()->width;
    hwFrame->height=source->getInfo()->height;
    hwFrame->format=AV_PIX_FMT_VAAPI;

    err=av_hwframe_get_buffer(_context->hw_frames_ctx,hwFrame,0);
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

    av_frame_free(&swFrame);
    swFrame=NULL;
    return true;
}

/**
    \fn encode
*/
bool         ADM_ffVAEncH264Encoder::encode (ADMBitstream * out)
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
                ADM_info("[ffVAEncH264] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[ffVAEncH264] Popping delayed bframes (%d)\n",sz);
        goto link;
        return false;
    }

    q=image->_Qp;
    if(!q) q=2;
    aprintf("[CODEC] Flags=%#x, QSCALE=%x, bit_rate=%d, quality=%d, qz=%d, incoming qz=%d\n",
        _context->flags,
        AV_CODEC_FLAG_QSCALE,
        (int)_context->bit_rate,
        hwFrame->quality,
        hwFrame->quality / FF_QP2LAMBDA,
        q);

    hwFrame->reordered_opaque=image->Pts;

    sz=encodeWrapper(hwFrame,out);
    if(sz<0)
    {
        CLEARTEXT(sz)
        ADM_warning("[ffVAEncH264] Error %d (%s) encoding video\n",sz,buf);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
    aprintf("[ffVAEncH264] encoder produces %d bytes\n",sz);
link:
    return postEncode(out,sz);
}

/**
    \fn isDualPass

*/
bool         ADM_ffVAEncH264Encoder::isDualPass(void)
{
    return false;
}

/**
    \fn ffVAEncConfigure
    \brief UI configuration for ffVAEncH264 encoder
*/

bool         ffVAEncConfigure(void)
{
    ffvaenc_encoder *conf=&VaEncSettings;

    diaMenuEntry h264Profile[]={
        {FF_PROFILE_H264_CONSTRAINED_BASELINE,QT_TRANSLATE_NOOP("ffVAEncH264","Baseline"),NULL},
        {FF_PROFILE_H264_MAIN,QT_TRANSLATE_NOOP("ffVAEncH264","Main"),NULL},
        {FF_PROFILE_H264_HIGH,QT_TRANSLATE_NOOP("ffVAEncH264","High"),NULL}
    };
    diaMenuEntry rateControlMode[]={
        {ADM_FFVAENC_RC_CRF,QT_TRANSLATE_NOOP("ffVAEncH264","Constant Rate Factor"),NULL},
        {ADM_FFVAENC_RC_CBR,QT_TRANSLATE_NOOP("ffVAEncH264","Constant Bitrate"),NULL},
#ifdef USE_VBR
        {ADM_FFVAENC_RC_VBR,QT_TRANSLATE_NOOP("ffVAEncH264","Variable Bitrate"),NULL}
#endif
    };

#define PX(x) &(conf->x)

    diaElemMenu profile(PX(profile),QT_TRANSLATE_NOOP("ffVAEncH264","Profile:"),3,h264Profile);
#ifdef USE_VBR
    diaElemMenu rcMode(PX(rc_mode),QT_TRANSLATE_NOOP("ffVAEncH264","Rate Control:"),3,rateControlMode);
    diaElemUInteger maxBitrate(PX(max_bitrate), QT_TRANSLATE_NOOP("ffVAEncH264","Max Bitrate (kbps):"),1,50000);
#else
    diaElemMenu rcMode(PX(rc_mode),QT_TRANSLATE_NOOP("ffVAEncH264","Rate Control:"),2,rateControlMode);
#endif
    diaElemUInteger gopSize(PX(gopsize),QT_TRANSLATE_NOOP("ffVAEncH264","GOP Size:"),1,250);

    if(conf->profile==FF_PROFILE_H264_CONSTRAINED_BASELINE)
        conf->bframes=0;

    diaElemUInteger maxBframes(PX(bframes),QT_TRANSLATE_NOOP("ffVAEncH264","Maximum Consecutive B-Frames:"),0,4);
    diaElemUInteger quality(PX(quality), QT_TRANSLATE_NOOP("ffVAEncH264","Quality:"),1,51);
    diaElemUInteger bitrate(PX(bitrate), QT_TRANSLATE_NOOP("ffVAEncH264","Bitrate (kbps):"),1,50000);
    diaElemFrame rateControl(QT_TRANSLATE_NOOP("ffVAEncH264","Rate Control"));
    diaElemFrame frameControl(QT_TRANSLATE_NOOP("ffVAEncH264","Frame Control"));

    rateControl.swallow(&rcMode);
    rateControl.swallow(&quality);
    rateControl.swallow(&bitrate);
#ifdef USE_VBR
    rateControl.swallow(&maxBitrate);

    rcMode.link(rateControlMode+2,1,&maxBitrate);
    rcMode.link(rateControlMode+2,1,&bitrate);
#endif
    rcMode.link(rateControlMode,1,&quality);
    rcMode.link(rateControlMode+1,1,&bitrate);

    frameControl.swallow(&gopSize);
    frameControl.swallow(&maxBframes);

    profile.link(h264Profile,0,&maxBframes);

    diaElem *diamode[] = {&profile,&rateControl,&frameControl};

    if( diaFactoryRun(QT_TRANSLATE_NOOP("ffVAEncH264","FFmpeg VA-API H.264 Encoder Configuration"),3,diamode))
    {
        return true;
    }
    return false;
}
// EOF
