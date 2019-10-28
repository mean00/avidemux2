/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_ffVTEnc.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

extern "C"
{
    #include "libavutil/opt.h"
}

ffvtenc VTEncSettings = VT_ENC_CONF_DEFAULT;

/**
    \fn ADM_ffVTEncoder
*/
ADM_ffVTEncoder::ADM_ffVTEncoder(ADM_coreVideoFilter *src, bool globalHeader) : ADM_coreVideoEncoderFFmpeg(src, NULL, globalHeader)
{
    ADM_info("Creating VideoToolbox HW accelerated H.264 encoder...\n");
    frameIncrement=src->getInfo()->frameIncrement;
}

/**
    \fn configureContext
*/
bool ADM_ffVTEncoder::configureContext(void)
{
    switch(VTEncSettings.profile)
    {
#define SAY(x,y) case FF_VT_PROFILE_##x: ADM_assert(!av_opt_set(_context->priv_data,"profile",y,0)); break;
        SAY(BASELINE,"baseline")
        SAY(MAIN,"main")
        SAY(HIGH,"high")
        default:break;
#undef SAY
    };

    _context->bit_rate=VTEncSettings.bitrate*1000;
    _context->rc_max_rate=VTEncSettings.max_bitrate*1000;
    _context->gop_size=VTEncSettings.gopsize;
    _context->max_b_frames=VTEncSettings.bframes;
    _context->pix_fmt=AV_PIX_FMT_YUV420P;

    return true;
}

/**
    \fn setup
*/
bool ADM_ffVTEncoder::setup(void)
{
    if(false== ADM_coreVideoEncoderFFmpeg::setupByName("h264_videotoolbox"))
    {
        ADM_info("[FFmpeg] Encoder setup failed\n");
        return false;
    }

    ADM_info("[FFmpeg] Setup OK\n");

    int w = getWidth();
    int h = getHeight();
    return true;
}

/**
    \fn getEncoderDelay
*/
uint64_t ADM_ffVTEncoder::getEncoderDelay(void)
{
    uint64_t delay=0;
    if(VTEncSettings.bframes)
        delay=frameIncrement*2;
    return delay;
}

/**
    \fn dtor
*/
ADM_ffVTEncoder::~ADM_ffVTEncoder()
{
    ADM_info("[ffVTEncoder] Destroying\n");
}

/**
    \fn encode
*/
bool ADM_ffVTEncoder::encode(ADMBitstream *out)
{
    int sz,q;
again:
    sz=0;
    if(false==preEncode()) // Pop out the frames stored in the queue due to B-frames
    {
        sz=encodeWrapper(NULL,out);
        if (sz<= 0)
        {
            ADM_info("[ffvtenc] Error %d encoding video\n",sz);
            return false;
        }
        ADM_info("[ffvtenc] Popping delayed bframes (%d)\n",sz);
        goto link;
        return false;
    }
    q = image->_Qp;

    if(!q) q=2;
    aprintf("[CODEC] Flags = 0x%x, QSCALE=%x, bit_rate=%d, quality=%d qz=%d incoming qz=%d\n",_context->flags,CODEC_FLAG_QSCALE,
                                     _context->bit_rate,  _frame->quality, _frame->quality/ FF_QP2LAMBDA,q);

    _frame->reordered_opaque = image->Pts;
    _frame->width = image->GetWidth(PLANAR_Y);
    _frame->height = image->GetHeight(PLANAR_Y);
    _frame->format = AV_PIX_FMT_YUV420P;
    sz=encodeWrapper(_frame,out);
    if(sz<0)
    {
        ADM_warning("[ffvtenc] Error %d encoding video\n",sz);
        return false;
    }

    if(sz==0) // no pic, probably pre filling, try again
        goto again;
link:
    return postEncode(out,sz);
}

/**
    \fn isDualPass

*/
bool ADM_ffVTEncoder::isDualPass(void)
{
    return false;
}

/**
    \fn ffVTEncConfigure
    \brief UI configuration for the h264_videotoolbox encoder
*/

bool ffVTEncConfigure(void)
{
    ffvtenc *conf=&VTEncSettings;

    diaMenuEntry vtProfile[]={
        {FF_VT_PROFILE_BASELINE,QT_TRANSLATE_NOOP("ffvtenc","Baseline")},
        {FF_VT_PROFILE_MAIN,QT_TRANSLATE_NOOP("ffvtenc","Main")},
        {FF_VT_PROFILE_HIGH,QT_TRANSLATE_NOOP("ffvtenc","High")}
};

#define PX(x) &(conf->x)

    diaElemMenu profile(PX(profile),QT_TRANSLATE_NOOP("ffvtenc","Profile:"),3,vtProfile);
    diaElemUInteger gopSize(PX(gopsize),QT_TRANSLATE_NOOP("ffvtenc","GOP Size:"),1,250);
    diaElemUInteger maxBframes(PX(bframes),QT_TRANSLATE_NOOP("ffvtenc","Maximum Consecutive B-Frames:"),0,1);
    diaElemUInteger bitrate(PX(bitrate), QT_TRANSLATE_NOOP("ffvtenc","Bitrate (kbps):"),1,50000);
    diaElemUInteger maxBitrate(PX(max_bitrate), QT_TRANSLATE_NOOP("ffvtenc","Max Bitrate (kbps):"),1,50000);
    diaElemFrame rateControl(QT_TRANSLATE_NOOP("ffvtenc","Rate Control"));
    diaElemFrame frameControl(QT_TRANSLATE_NOOP("ffvtenc","Frame Control"));

    rateControl.swallow(&bitrate);
    rateControl.swallow(&maxBitrate);
    frameControl.swallow(&gopSize);
    frameControl.swallow(&maxBframes);

    profile.link(&vtProfile[0],0,&maxBframes);

    diaElem *diamode[] = {&profile,&rateControl,&frameControl};

    if( diaFactoryRun(QT_TRANSLATE_NOOP("ffvtenc","VideoToolbox H.264 Encoder Configuration"),3,diamode))
    {
        if(conf->profile==FF_VT_PROFILE_BASELINE)
            conf->bframes=0;
        return true;
    }
    return false;
}
// EOF
