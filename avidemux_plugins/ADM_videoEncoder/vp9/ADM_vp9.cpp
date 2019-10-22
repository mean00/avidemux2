/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_vp9.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "vpx/vp8cx.h"
#include "vp9_encoder.h"
#include "vp9_encoder_desc.cpp"
#include "DIA_factory.h"

#define MMSET(x) memset(&(x),0,sizeof(x))

vp9_encoder vp9Settings = VP9_DEFAULT_CONF;

static bool vp9EncoderConfigure(void);

void resetConfigurationData()
{
    vp9_encoder conf = VP9_DEFAULT_CONF;
    memcpy(&vp9Settings, &conf, sizeof(vp9_encoder));
}

ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(vp9Encoder);
ADM_DECLARE_VIDEO_ENCODER_MAIN("vp9",
                               "VP9 (libvpx)",
                               "libvpx based VP9 Encoder",
                                vp9EncoderConfigure, // configuration dialog
                                ADM_UI_ALL,
                                1,0,0,
                                vp9_encoder_param, // conf template
                                &vp9Settings, // conf var
                                NULL, // setProfile
                                NULL  // getProfile
);

/**
 *  \fn vp9Encoder
 */
vp9Encoder::vp9Encoder(ADM_coreVideoFilter *src, bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("Creating VP9 encoder\n");
    MMSET(context);
    MMSET(param);
    iface=NULL;
    pic=NULL;
    flush=false;
}

static void dumpParams(vpx_codec_enc_cfg_t *cfg)
{
    int i;
    printf("\n");
#define PI(x) printf(#x":\t%d\n",(int)cfg->x);
    PI(g_usage)
    PI(g_threads)
    PI(g_profile)
    PI(g_w)
    PI(g_h)
    PI(g_bit_depth)
    PI(g_input_bit_depth)
    PI(g_timebase.num)
    PI(g_timebase.den)
    PI(g_error_resilient)
    PI(g_pass)
    PI(g_lag_in_frames)
    printf("**********************************\n");
    printf("********   rate control   ********\n");
    printf("**********************************\n");
    PI(rc_dropframe_thresh)
    PI(rc_resize_allowed)
    PI(rc_resize_up_thresh)
    PI(rc_resize_down_thresh)
    PI(rc_end_usage)
#define PP(x) printf(#x":\t%p\n",cfg->x);
    PP(rc_twopass_stats_in.buf)
    PI(rc_twopass_stats_in.sz)
    PI(rc_target_bitrate)
    PI(rc_min_quantizer)
    PI(rc_max_quantizer)
    PI(rc_undershoot_pct)
    PI(rc_overshoot_pct)
    PI(rc_buf_sz)
    PI(rc_buf_initial_sz)
    PI(rc_buf_optimal_sz)
    PI(rc_2pass_vbr_bias_pct)
    PI(rc_2pass_vbr_minsection_pct)
    PI(rc_2pass_vbr_maxsection_pct)
#if VPX_ENCODER_ABI_VERSION >= 14
    PI(rc_2pass_vbr_corpus_complexity)
#endif
    printf("**********************************\n");
    printf("******  temporal layering  *******\n");
    printf("**********************************\n");
    PI(ts_number_layers)
#define LOOPI(x,y,z) for(i=0; i<(z); i++) { printf(#x", "#y" %d:\t%d\n",i,cfg->x[i]); }
    LOOPI(ts_target_bitrate,layer,VPX_TS_MAX_LAYERS)
    LOOPI(ts_rate_decimator,layer,VPX_TS_MAX_LAYERS)
    PI(ts_periodicity)
    LOOPI(ts_layer_id,sequence_number,VPX_TS_MAX_PERIODICITY)
    printf("**********************************\n");
    printf("******  keyframe settings  *******\n");
    printf("**********************************\n");
    PI(kf_mode)
    PI(kf_min_dist)
    PI(kf_max_dist)
    printf("\n");
}

static const char *packetTypeToString(int type)
{
    char msg[32]={0};
#define REPORT(x) case x: snprintf(msg,32,#x);break;
    switch(type)
    {
        REPORT(VPX_CODEC_CX_FRAME_PKT)
        REPORT(VPX_CODEC_STATS_PKT)
        REPORT(VPX_CODEC_FPMB_STATS_PKT)
        REPORT(VPX_CODEC_PSNR_PKT)
        REPORT(VPX_CODEC_CUSTOM_PKT)
        default: snprintf(msg,32,"unknown packet type"); break;
    }
    return ADM_strdup(msg);
}

static int64_t scaleTime(uint32_t num, uint32_t den, uint64_t time)
{
    if(time==ADM_NO_PTS)
        return (int64_t)time;
    ADM_assert(!(time>>62));
    ADM_assert(num);
    double d=time;
    d/=1000.;
    d*=den;
    d/=1000.;
    d+=d/2.;
    d/=num;
    return (int64_t)d;
}

/**
 * \fn setup
 */
bool vp9Encoder::setup(void)
{
    vpx_codec_err_t ret;

    image=new ADMImageDefault(getWidth(),getHeight());
    if(!image)
    {
        ADM_error("Cannot allocate image.\n");
        return false;
    }

    iface=vpx_codec_vp9_cx();
    if(!iface)
    {
        ADM_error("No VP9 interface available.\n");
        return false;
    }
    ADM_info("VP9 interface available at %p\n",iface);

    ret=vpx_codec_enc_config_default(iface,&param,0);
    if(ret!=VPX_CODEC_OK)
    {
        ADM_error("[vp9Encoder] Cannot set default configuration, error %d: %s.\n",(int)ret,vpx_codec_err_to_string(ret));
        return false;
    }
    ADM_info("Initial default config:\n");
    dumpParams(&param);

    param.g_w=getWidth();
    param.g_h=getHeight();
    param.g_threads=vp9Settings.nbThreads;
    usSecondsToFrac(getFrameIncrement(),&(param.g_timebase.num),&(param.g_timebase.den));
    ticks=param.g_timebase.num;
    //param.g_timebase.num=1;
    //param.g_timebase.den=1000000;
    param.g_pass=VPX_RC_ONE_PASS;
    param.rc_min_quantizer=vp9Settings.qMin;
    param.rc_max_quantizer=vp9Settings.qMax;
    param.rc_end_usage=VPX_Q;

    ADM_info("Trying to init encoder with the following configuration:\n");
    dumpParams(&param);

    ret=vpx_codec_enc_init(&context,iface,&param,0);
    if(ret!=VPX_CODEC_OK)
    {
        ADM_error("[vp9Encoder] Init failed with error %d: %s\n",(int)ret,vpx_codec_err_to_string(ret));
        return false;
    }

    uint32_t alignment=16;

    pic=vpx_img_alloc(pic,VPX_IMG_FMT_I420,param.g_w,param.g_h,alignment);
    if(!pic)
    {
        ADM_error("[vp9Encoder] Cannot allocate image.\n");
        return false;
    }

    switch(vp9Settings.deadline)
    {
        case 0:
            dline=VPX_DL_REALTIME;
            break;
        case 1:
            dline=VPX_DL_GOOD_QUALITY;
            break;
        case 2:
            dline=VPX_DL_BEST_QUALITY;
            break;
        default:
            dline=VPX_DL_GOOD_QUALITY;
            break;
    }
    return true;
}

/**
 *  \fn ~vp9Encoder
 */
vp9Encoder::~vp9Encoder()
{
    ADM_info("[vp9] Destroying.\n");
    if(pic)
    {
        vpx_img_free(pic);
        pic=NULL;
    }
    vpx_codec_destroy(&context);
}

/**
    \fn encode
*/
bool vp9Encoder::encode(ADMBitstream *out)
{
    // 1 fetch a frame...
    uint32_t nb;
    uint64_t pts;
    vpx_codec_err_t er;

    // update
again:
    if(!flush && source->getNextFrame(&nb,image)==false)
    {
        ADM_warning("[vp9] Cannot get next image\n");
        flush=true;
    }else
    {
        pic->planes[VPX_PLANE_Y] = YPLANE(image);
        pic->planes[VPX_PLANE_U] = VPLANE(image);
        pic->planes[VPX_PLANE_V] = UPLANE(image);
        pic->stride[VPX_PLANE_Y] = image->GetPitch(PLANAR_Y);
        pic->stride[VPX_PLANE_U] = image->GetPitch(PLANAR_V);
        pic->stride[VPX_PLANE_V] = image->GetPitch(PLANAR_U);
        pic->bit_depth = 8;

        pts=image->Pts;
        queueOfDts.push_back(pts);
        uint64_t real=pts;
        pts=scaleTime(ticks, param.g_timebase.den, real);
        ADM_timeMapping map;
        map.realTS=real;
        map.internalTS=pts;
        mapper.push_back(map);
    }

    if(flush)
    {
        ADM_info("Flushing delayed frames\n");
        pts+=ticks;
        er=vpx_codec_encode(&context,NULL,pts,ticks,0,dline);
    }else
    {
        er=vpx_codec_encode(&context,pic,pts,ticks,0,dline);
    }
    if(er!=VPX_CODEC_OK)
    {
        ADM_error("Encoding error %d: %s\n",(int)er,vpx_codec_err_to_string(er));
        return false;
    }

    out->flags=0;

    if(!postAmble(out))
    {
        if(flush) return false;
        goto again;
    }

    return true;
}

/**
    \fn isDualPass

*/
bool vp9Encoder::isDualPass(void)
{
    return false;
}

/**
    \fn postAmble
    \brief update after a frame has been succesfully encoded
*/
bool vp9Encoder::postAmble(ADMBitstream *out)
{
    const vpx_codec_cx_pkt *pkt;
    const void *iter=NULL;

    while((pkt=vpx_codec_get_cx_data(&context,&iter)))
    {
        if(pkt->kind != VPX_CODEC_CX_FRAME_PKT)
        {
            ADM_info("Got packet of type: %s\n",packetTypeToString(pkt->kind));
            continue;
        }
        packetQueue.push_back(pkt);
    }

    if(packetQueue.size())
    {
        pkt=packetQueue.front();
        packetQueue.erase(packetQueue.begin());
        memcpy(out->data,pkt->data.frame.buf,pkt->data.frame.sz);
        out->len=pkt->data.frame.sz;
        getRealPtsFromInternal(pkt->data.frame.pts,&out->dts,&out->pts);
        if(pkt->data.frame.flags & VPX_FRAME_IS_KEY)
            out->flags=AVI_KEY_FRAME;
        return true;
    }

    return false;
}

/**
 *  \fn vp9EncoderConfigure
 *  \brief Configuration UI for VP9 encoder
 */
bool vp9EncoderConfigure(void)
{
    vp9_encoder *cfg = &vp9Settings;

    diaMenuEntry dltype[]={
        {REALTIME,QT_TRANSLATE_NOOP("vp9encoder","Speed (slow)")},
        {GOOD_QUALITY,QT_TRANSLATE_NOOP("vp9encoder","Balanced (very slow)")},
        {BEST_QUALITY,QT_TRANSLATE_NOOP("vp9encoder","Quality (extremely slow)")}
    };
#define PX(x) &(cfg->x)
    diaElemMenu qual(PX(deadline),QT_TRANSLATE_NOOP("vp9encoder","Speed / Quality"),3,dltype);
    diaElemUInteger conc(PX(nbThreads),QT_TRANSLATE_NOOP("vp9encoder","Threads"),1,8);
    diaElemUInteger minq(PX(qMin),QT_TRANSLATE_NOOP("vp9encoder","Min. Quantizer"),0,63);
    diaElemUInteger maxq(PX(qMax),QT_TRANSLATE_NOOP("vp9encoder","Max. Quantizer"),0,63);

    diaElem *dialog[] = {&qual,&conc,&minq,&maxq};
    if(diaFactoryRun(QT_TRANSLATE_NOOP("vp9encoder","libvpx VP9 Encoder Configuration"),4,dialog))
    {
        if(cfg->qMin > cfg->qMax)
        {
            uint32_t swap=cfg->qMax;
            cfg->qMax=cfg->qMin;
            cfg->qMin=swap;
        }
        return true;
    }
    return false;
}

// EOF

