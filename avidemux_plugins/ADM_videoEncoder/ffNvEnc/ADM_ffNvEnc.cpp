/***************************************************************************
                          \fn ADM_ffNvEnc
                          \brief Front end for libavcodec nvenc encoders
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
#include "ADM_ffNvEnc.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
//#define USE_NV12 
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

#define NV_MX_LOOKAHEAD 31

extern "C"
{
    #include "libavutil/opt.h"
}

ffnvenc_encoder NvEncSettings = NVENC_CONF_DEFAULT;

/**
        \fn ADM_ffNvEncEncoder
*/
ADM_ffNvEncEncoder::ADM_ffNvEncEncoder(ADM_coreVideoFilter *src,bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src,NULL,globalHeader)
{
    //targetColorSpace=ADM_COLOR_YUV422P;
#ifdef H265_ENCODER
    ADM_info("(HEVC) Creating.\n");
#else
    ADM_info("(H264) Creating.\n");
#endif
    nv12=NULL;
    frameIncrement=src->getInfo()->frameIncrement;

}

/**
    \fn configureContext
    \brief pre-open
*/
bool ADM_ffNvEncEncoder::configureContext(void)
{
    _context->bit_rate = -1;
    _context->rc_max_rate = -1;

    _context->gop_size = NvEncSettings.gopsize;
    _context->max_b_frames =
#ifdef H265_ENCODER
        NvEncSettings.bframes;
#else
        (NvEncSettings.profile != NV_FF_PROFILE_BASELINE)? NvEncSettings.bframes : 0;
#endif

    if(_context->max_b_frames > 1)
    {
        switch(NvEncSettings.b_ref_mode)
        {
            case NV_FF_BFRAME_REF_DISABLED:
                break;
            case NV_FF_BFRAME_REF_EACH:
#ifdef H265_ENCODER
                av_dict_set(&_options,"b_ref_mode","each",0);
#else
                ADM_warning("b_ref_mode %u (\"each\") is invalid for h264_nvenc, ignoring.\n",NvEncSettings.b_ref_mode);
#endif
                break;
            case NV_FF_BFRAME_REF_MIDDLE:
                av_dict_set(&_options,"b_ref_mode","middle",0);
                break;
            default:
                ADM_warning("b_ref_mode %u is invalid, ignoring.\n",NvEncSettings.b_ref_mode);
                break;
        }
    }

    switch(NvEncSettings.preset)
    {
#define MIAOU(x,y) case NV_FF_PRESET_##x: av_dict_set(&_options,"preset",y,0); break;
        MIAOU(HP,"hp")
        MIAOU(HQ,"hq")
        MIAOU(BD,"bd")
        MIAOU(LL,"ll")
        MIAOU(LLHP,"llhp")
        MIAOU(LLHQ,"llhq")
        MIAOU(LOSSLESS,"lossless")
        MIAOU(LOSSLESSHP,"losslesshp")
        default:break;
#undef MIAOU
    }

#define OPTION_BUFFER_SIZE 64
    char buf[OPTION_BUFFER_SIZE];

    switch(NvEncSettings.rc_mode)
    {
        case NV_FF_RC_AUTO:
            _context->bit_rate=NvEncSettings.bitrate*1000;
            break;
        case NV_FF_RC_CONSTQP:
            _context->qmin = _context->qmax = NvEncSettings.quality;
            av_dict_set(&_options,"rc","constqp",0);
            snprintf(buf, OPTION_BUFFER_SIZE, "%d", NvEncSettings.quality);
            av_dict_set(&_options,"qp",buf,0);
            break;
        case NV_FF_RC_CBR:
            _context->bit_rate = _context->rc_max_rate = NvEncSettings.bitrate*1000;
            av_dict_set(&_options,"rc","cbr",0);
            break;
        case NV_FF_RC_VBR:
            _context->bit_rate=NvEncSettings.bitrate*1000;
            _context->rc_max_rate=NvEncSettings.max_bitrate*1000;
            av_dict_set(&_options,"rc","vbr",0);
            snprintf(buf, OPTION_BUFFER_SIZE, "%d", NvEncSettings.quality); // actually a float
            av_dict_set(&_options,"cq",buf,0);
            break;
        default:
            ADM_warning("Unsupported mode %d\n",NvEncSettings.rc_mode);
            break;
    }

    switch(NvEncSettings.profile)
    {
#define MIAOU(x,y) case NV_FF_PROFILE_##x: av_dict_set(&_options,"profile",y,0); break;
#ifdef H265_ENCODER
        MIAOU(MAIN,"main")
        MIAOU(MAIN10,"main10")
#else
        MIAOU(BASELINE,"baseline")
        MIAOU(MAIN,"main")
        MIAOU(HIGH,"high")
#endif
        default:break;
#undef MIAOU
    };

    if(NvEncSettings.lookahead)
    {
        int range = NvEncSettings.lookahead;
        const int maxr = (_context->gop_size > NV_MX_LOOKAHEAD - _context->max_b_frames)?
            NV_MX_LOOKAHEAD - _context->max_b_frames : _context->gop_size;
        if(range > maxr)
        {
            ADM_warning("Specified lookahead value %d exceeds maximum %d, clamping down.\n",range,maxr);
            range = maxr;
        }
        snprintf(buf, OPTION_BUFFER_SIZE, "%d", range);
        av_dict_set(&_options,"rc-lookahead",buf,0);
        // set sufficient delay else lavc will disable lookahead
        snprintf(buf, OPTION_BUFFER_SIZE, "%d", range+5);
        av_dict_set(&_options,"delay",buf,0);
    }

    if(NvEncSettings.spatial_aq)
    {
        snprintf(buf, OPTION_BUFFER_SIZE, "%d", NvEncSettings.aq_strength);
        av_dict_set(&_options,"spatial-aq","1",0);
        av_dict_set(&_options,"aq-strength",buf,0);
    }

    if(NvEncSettings.temporal_aq)
        av_dict_set(&_options,"temporal-aq","1",0);

    if(NvEncSettings.temporal_aq)
        av_dict_set(&_options,"weighted_pred","1",0);

#ifdef USE_NV12    
    _context->pix_fmt=  AV_PIX_FMT_NV12;        
#else    
    _context->pix_fmt=AV_PIX_FMT_YUV420P;
#endif    
    
    return true;             
}

/**
    \fn setup
*/
bool ADM_ffNvEncEncoder::setup(void)
{
#define STR(x) #x
#define MKSTRING(x) STR(x)
#ifdef H265_ENCODER
#   define LAVC_ENCODER_NAME hevc_nvenc
#else
#   define LAVC_ENCODER_NAME h264_nvenc
#endif
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName(MKSTRING(LAVC_ENCODER_NAME)))
    {
        ADM_info("[ffMpeg] Setup failed\n");
        return false;
    }
#undef LAVC_ENCODER_NAME
    ADM_info("[ffMpeg] Setup ok\n");
#ifdef USE_NV12
    int w= getWidth();
    int h= getHeight();
    
    w=(w+31)&~31;
    h=(h+15)&~15;
    nv12=new uint8_t[(w*h)/2]; 
    nv12Stride=w;
#endif
    return true;
}

/**
    \fn getEncoderDelay
*/
uint64_t ADM_ffNvEncEncoder::getEncoderDelay(void)
{
    uint64_t delay=0;
    if(NvEncSettings.bframes)
        delay = frameIncrement * ((NvEncSettings.b_ref_mode == NV_FF_BFRAME_REF_DISABLED)? 2 : 3); // untested with NV_FF_BFRAME_REF_EACH
    return delay;
}

/**
    \fn ~ADM_ffNvEncEncoder
*/
ADM_ffNvEncEncoder::~ADM_ffNvEncEncoder()
{
    ADM_info("[ffNvEncEncoder] Destroying.\n");
    if(nv12)
    {
        delete [] nv12;
        nv12=NULL;
    }

}

/**
    \fn getFourcc
*/
const char *ADM_ffNvEncEncoder::getFourcc(void)
{
#ifdef H265_ENCODER
    return "HEVC";
#else
    return "H264";
#endif
}

/**
    \fn encode
*/
bool         ADM_ffNvEncEncoder::encode (ADMBitstream * out)
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
                ADM_info("[ffnvenc] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[ffnvenc] Popping delayed bframes (%d)\n",sz);
        goto link;
        return false;
    }
    q=image->_Qp;

    if(!q) q=2;
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame->quality, _frame->quality/ FF_QP2LAMBDA,q);

    _frame->reordered_opaque=image->Pts;
    _frame->width=image->GetWidth(PLANAR_Y);
    _frame->height=image->GetHeight(PLANAR_Y);

// convert to nv12
#ifdef USE_NV12
    image->interleaveUVtoNV12(nv12,nv12Stride);
    _frame->data[0] = image->GetReadPtr(PLANAR_Y);
    _frame->data[1] = nv12;
    _frame->data[2] = NULL;

    _frame->linesize[0]=image->GetPitch(PLANAR_Y);
    _frame->linesize[1]=nv12Stride;
    _frame->linesize[2]=0;
    _frame->format=  AV_PIX_FMT_NV12;    
#else
    _frame->format=  AV_PIX_FMT_YUV420P;    
#endif        
    sz=encodeWrapper(_frame,out);
    if(sz<0)
    {
        ADM_warning("[ffnvenc] Error %d encoding video\n",sz);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    return postEncode(out,sz);
}

/**
    \fn ffNvEncConfigure
    \brief Show configuration dialog
*/
bool ffNvEncConfigure(void)
{
    diaMenuEntry meRcMode[]={
        {NV_FF_RC_AUTO,QT_TRANSLATE_NOOP("ffnvenc","Controlled by Preset")},
        {NV_FF_RC_CONSTQP,QT_TRANSLATE_NOOP("ffnvenc","Constant Quantizer")},
        {NV_FF_RC_CBR,QT_TRANSLATE_NOOP("ffnvenc","Constant Bitrate")},
        {NV_FF_RC_VBR,QT_TRANSLATE_NOOP("ffnvenc","Variable Bitrate")}
    };

    diaMenuEntry mePreset[]={
        {NV_FF_PRESET_DEFAULT,QT_TRANSLATE_NOOP("ffnvenc","Default")},
        {NV_FF_PRESET_SLOW,QT_TRANSLATE_NOOP("ffnvenc","Slow")},
        {NV_FF_PRESET_MEDIUM,QT_TRANSLATE_NOOP("ffnvenc","Medium")},
        {NV_FF_PRESET_MEDIUM,QT_TRANSLATE_NOOP("ffnvenc","Fast")},
        {NV_FF_PRESET_HP,QT_TRANSLATE_NOOP("ffnvenc","High Performance")},
        {NV_FF_PRESET_HQ,QT_TRANSLATE_NOOP("ffnvenc","High Quality")},
        {NV_FF_PRESET_BD,QT_TRANSLATE_NOOP("ffnvenc","BluRay")},
        {NV_FF_PRESET_LL,QT_TRANSLATE_NOOP("ffnvenc","Low Latency")},
        {NV_FF_PRESET_LLHP,QT_TRANSLATE_NOOP("ffnvenc","Low Latency (HP)")},
        {NV_FF_PRESET_LLHQ,QT_TRANSLATE_NOOP("ffnvenc","Low Latency (HQ)")},
        {NV_FF_PRESET_LOSSLESS,QT_TRANSLATE_NOOP("ffnvenc","Lossless")},
        {NV_FF_PRESET_LOSSLESSHP,QT_TRANSLATE_NOOP("ffnvenc","Lossless (HP)")}
    };

    diaMenuEntry meProfile[]={
#ifdef H265_ENCODER
        {NV_FF_PROFILE_MAIN,QT_TRANSLATE_NOOP("ffnvenc","Main")},
        {NV_FF_PROFILE_MAIN10,QT_TRANSLATE_NOOP("ffnvenc","Main10")}
#else
        {NV_FF_PROFILE_BASELINE,QT_TRANSLATE_NOOP("ffnvenc","Baseline")},
        {NV_FF_PROFILE_MAIN,QT_TRANSLATE_NOOP("ffnvenc","Main")},
        {NV_FF_PROFILE_HIGH,QT_TRANSLATE_NOOP("ffnvenc","High")}
#endif
    };

    diaMenuEntry meBframeRef[]={
        {NV_FF_BFRAME_REF_DISABLED,QT_TRANSLATE_NOOP("ffnvenc","Disabled")},
#ifdef H265_ENCODER
        {NV_FF_BFRAME_REF_EACH,QT_TRANSLATE_NOOP("ffnvenc","Each")},
#endif
        {NV_FF_BFRAME_REF_MIDDLE,QT_TRANSLATE_NOOP("ffnvenc","Middle")}
    };

    ffnvenc_encoder *conf=&NvEncSettings;

#define PX(x) &conf->x
#define MZ(x) sizeof(x)/sizeof(diaMenuEntry)
    diaElemMenu rcmode(PX(rc_mode),QT_TRANSLATE_NOOP("ffnvenc","RC Mode:"),MZ(meRcMode),meRcMode);
    diaElemMenu qzPreset(PX(preset),QT_TRANSLATE_NOOP("ffnvenc","Preset:"),MZ(mePreset),mePreset);
    diaElemMenu profile(PX(profile),QT_TRANSLATE_NOOP("ffnvenc","Profile:"),MZ(meProfile),meProfile);
    diaElemMenu bFrameRef(PX(b_ref_mode),QT_TRANSLATE_NOOP("ffnvenc","Use B-Frames as References:"),MZ(meBframeRef),meBframeRef);

    diaElemUInteger qual(PX(quality),QT_TRANSLATE_NOOP("ffnvenc","Quality:"),0,51);
    diaElemUInteger bitrate(PX(bitrate),QT_TRANSLATE_NOOP("ffnvenc","Bitrate (kbps):"),1,50000);
    diaElemUInteger maxBitrate(PX(max_bitrate),QT_TRANSLATE_NOOP("ffnvenc","Max Bitrate (kbps):"),1,50000);

    diaElemUInteger gopSize(PX(gopsize),QT_TRANSLATE_NOOP("ffnvenc","GOP Size:"),8,1000);
    diaElemUInteger maxBframes(PX(bframes),QT_TRANSLATE_NOOP("ffnvenc","Maximum Consecutive B-Frames:"),0,4);

    diaElemUInteger lookAhead(PX(lookahead),QT_TRANSLATE_NOOP("ffnvenc","Lookahead:"),0,NV_MX_LOOKAHEAD);
    diaElemUInteger aqStrength(PX(aq_strength),QT_TRANSLATE_NOOP("ffnvenc","AQ Strength:"),1,15);

    diaElemToggle spatAq(PX(spatial_aq),QT_TRANSLATE_NOOP("ffnvenc","Spatial AQ"));
    diaElemToggle tempAq(PX(temporal_aq),QT_TRANSLATE_NOOP("ffnvenc","Temporal AQ"));
    diaElemToggle wPred(PX(weighted_pred),QT_TRANSLATE_NOOP("ffnvenc","Weighted Prediction"));
#ifdef H265_ENCODER
    diaElemReadOnlyText hintBasic(QT_TRANSLATE_NOOP("ffnvenc","Even with HEVC encoding support present, "
        "lossless presets and B-frames may be unavailable with older hardware"),NULL);
#else
    diaElemReadOnlyText hintBasic(QT_TRANSLATE_NOOP("ffnvenc","Even with H.264 encoding support present, "
        "lossless presets may be unavailable with older hardware"),NULL);
#endif
    diaElemReadOnlyText hintAdvanced(QT_TRANSLATE_NOOP("ffnvenc","Lookahead and Adaptive Quantization may be unavailable with older hardware"),NULL);

    diaElemFrame rateControl(QT_TRANSLATE_NOOP("ffnvenc","Rate Control"));
    diaElemFrame frameControl(QT_TRANSLATE_NOOP("ffnvenc","Frame Control"));

    rateControl.swallow(&rcmode);
    rateControl.swallow(&qzPreset);
    rateControl.swallow(&qual);
    rateControl.swallow(&bitrate);
    rateControl.swallow(&maxBitrate);

    rcmode.link(meRcMode,1,&bitrate);
    rcmode.link(meRcMode+1,1,&qual);
    rcmode.link(meRcMode+2,1,&bitrate);
    rcmode.link(meRcMode+3,1,&qual);
    rcmode.link(meRcMode+3,1,&bitrate);
    rcmode.link(meRcMode+3,1,&maxBitrate);

#ifndef H265_ENCODER
    profile.link(meProfile+1,1,&maxBframes);
    profile.link(meProfile+2,1,&maxBframes);
#endif
    spatAq.link(1,&aqStrength);

    frameControl.swallow(&gopSize);
    frameControl.swallow(&maxBframes);

    /* First Tab : basic settings */
    diaElem *basics[]={&profile,&rateControl,&frameControl,&hintBasic};
    diaElemTabs tabBasic(QT_TRANSLATE_NOOP("ffnvenc","Basic Settings"),4,basics);

    /* 2nd Tab : advanced*/
    diaElem *advanced[]={&bFrameRef,&wPred,&spatAq,&tempAq,&aqStrength,&lookAhead,&hintAdvanced};
    diaElemTabs tabAdvanced(QT_TRANSLATE_NOOP("ffnvenc","Advanced Settings"),7,advanced);

    diaElemTabs *tabs[]={&tabBasic,&tabAdvanced};
    if(diaFactoryRunTabs(
#ifdef H265_ENCODER
            QT_TRANSLATE_NOOP("ffnvenc","NVENC HEVC configuration"),
#else
            QT_TRANSLATE_NOOP("ffnvenc","NVENC H.264 configuration"),
#endif
            2,tabs))
        return true;
    return false;
}
// EOF
