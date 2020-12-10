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
#include "vpx/vp8cx.h"
#include "vp9_encoder.h"

#define VP9_MAX_QUANTIZER 63
#define MMSET(x) memset(&(x),0,sizeof(x))

vp9_encoder vp9Settings = VP9_DEFAULT_CONF;

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
    passNumber=0;
    statFd=NULL;
    statBuf=NULL;
    lastScaledPts=0;
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
    d/=num;
    d+=0.49;
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
    uint32_t threads=vp9Settings.autoThreads? ADM_cpu_num_processors() : vp9Settings.nbThreads;
    if(threads>VP9_ENC_MAX_THREADS) threads=VP9_ENC_MAX_THREADS;
    param.g_threads=vp9Settings.nbThreads=threads;

    FilterInfo *info=source->getInfo();
    param.g_timebase.num = info->timeBaseNum & 0x7FFFFFFF;
    param.g_timebase.den = info->timeBaseDen & 0x7FFFFFFF;
    ADM_assert(param.g_timebase.num);
    ADM_assert(param.g_timebase.den);
#define MAX_CLOCK 180000
    if(isStdFrameRate(param.g_timebase.den, param.g_timebase.num))
    {
        int64_t dur=scaleTime(param.g_timebase.num, param.g_timebase.den, info->frameIncrement);
        if(dur<1) dur=1;
        if(dur>MAX_CLOCK) dur=MAX_CLOCK;
        scaledFrameDuration=dur;
    }else
    {
        usSecondsToFrac(info->frameIncrement,&(param.g_timebase.num),&(param.g_timebase.den),MAX_CLOCK);
        scaledFrameDuration=1;
    }

    int speed=(vp9Settings.speed > 18)? 9 : vp9Settings.speed-9;

    param.rc_min_quantizer=vp9Settings.ratectl.qz;
    param.rc_max_quantizer=VP9_MAX_QUANTIZER;
    switch(vp9Settings.ratectl.mode)
    {
        case COMPRESS_CQ:
            param.rc_max_quantizer=vp9Settings.ratectl.qz;
            param.rc_end_usage=VPX_CQ;
            break;
        case COMPRESS_CBR:
            param.rc_target_bitrate=vp9Settings.ratectl.bitrate;
            param.rc_end_usage=VPX_CBR;
            break;
        case COMPRESS_2PASS:
        case COMPRESS_2PASS_BITRATE:
            if(passNumber!=1 && passNumber!=2)
            {
                ADM_error("Invalid pass number %d provided.\n",(int)passNumber);
                return false;
            }
            ADM_info("[vp9Encoder] Starting pass %d\n",passNumber);
            if(passNumber==1)
            {
                param.g_lag_in_frames=0;
                speed=1;
            }else
            {
                int64_t sz=ADM_fileSize(logFile.c_str());
                if(sz<=0)
                {
                    ADM_error("Stats file not found or empty, cannot proceed with the second pass.\n");
                    return false;
                }
                if(sz>(1LL<<30))
                {
                    ADM_error("Stats file size %" PRId64" exceeds one GiB, this cannot be right, not trying to load it into memory.\n",sz);
                    return false;
                }
                statBuf=ADM_alloc(sz);
                if(!statBuf)
                {
                    ADM_error("Allocating memory for stats from the first pass failed.\n");
                    return false;
                }
                statFd=ADM_fopen(logFile.c_str(),"r");
                if(!ADM_fread(statBuf,sz,1,statFd))
                {
                    ADM_error("Reading stats file %s failed.\n",logFile.c_str());
                    fclose(statFd);
                    statFd=NULL;
                    return false;
                }
                fclose(statFd);
                statFd=NULL;
                param.rc_twopass_stats_in.buf=statBuf; // should be freed by vpx_codec_destroy()
                param.rc_twopass_stats_in.sz=sz;
            }
            {
                uint32_t bitrate=0;
                if(vp9Settings.ratectl.mode==COMPRESS_2PASS)
                {
                    uint64_t duration=source->getInfo()->totalDuration;
                    if(vp9Settings.ratectl.finalsize)
                    {
                        if(false==ADM_computeAverageBitrateFromDuration(duration, vp9Settings.ratectl.finalsize, &bitrate))
                            return false;
                    }
                }else
                {
                    bitrate=vp9Settings.ratectl.avg_bitrate;
                }
                if(bitrate)
                {
                    param.rc_target_bitrate=bitrate;
                    param.rc_2pass_vbr_maxsection_pct=100; // max. bitrate = 2*target
                    param.rc_end_usage=VPX_CQ;
                }else
                {
                    param.rc_target_bitrate=0;
                    param.rc_end_usage=VPX_Q;
                }
            }
            break;
        default:
            break;
    }
    param.g_pass=(!passNumber)? VPX_RC_ONE_PASS : (passNumber==1)? VPX_RC_FIRST_PASS : VPX_RC_LAST_PASS;
    param.kf_max_dist=vp9Settings.keyint;

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
        ADM_error("[vp9Encoder] Cannot allocate VPX image.\n");
        return false;
    }

    dline=VPX_DL_GOOD_QUALITY;
    switch(vp9Settings.deadline)
    {
        case 0:
            if(passNumber==1)
                break;
            dline=VPX_DL_REALTIME;
            param.g_lag_in_frames=0;
            break;
        case 1:
            break;
        case 2:
            dline=VPX_DL_BEST_QUALITY;
            break;
        default:
            break;
    }

    if(VPX_CODEC_OK!=vpx_codec_control(&context,VP8E_SET_CPUUSED,speed))
    {
        ADM_warning("[vp9Encoder] Cannot set VP8E_SET_CPUUSED codec control to %d\n",speed);
    }
    if(param.rc_end_usage==VPX_CQ && VPX_CODEC_OK!=vpx_codec_control(&context,VP8E_SET_CQ_LEVEL,vp9Settings.ratectl.qz))
    {
        ADM_warning("[vp9Encoder] Cannot set VP8E_SET_CQ_LEVEL codec control to %u\n",vp9Settings.ratectl.qz);
    }
    if(VPX_CODEC_OK!=vpx_codec_control(&context,VP9E_SET_COLOR_RANGE,vp9Settings.fullrange))
    {
        ADM_warning("[vp9Encoder] Cannot set VP9E_SET_COLOR_RANGE codec control to %d\n",vp9Settings.fullrange);
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
    if(statFd)
        fclose(statFd);
    statFd=NULL;
    vpx_codec_destroy(&context);
}

/**
 *  \fn setPassAndLogFile
 */
bool vp9Encoder::setPassAndLogFile(int pass, const char *name)
{
    ADM_info("Initializing pass %d, log file: %s\n",pass,name);
    logFile=std::string(name);
    passNumber=pass;
    return true;
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
    }
    if(flush)
    {
        ADM_info("Flushing delayed frames\n");
        pts+=scaledFrameDuration;
        er=vpx_codec_encode(&context,NULL,pts,scaledFrameDuration,0,dline);
    }else
    {
        if(image->_range == ADM_COL_RANGE_JPEG)
        {
            if(!vp9Settings.fullrange)
                image->shrinkColorRange();
        }else
        {
            if(vp9Settings.fullrange)
                image->expandColorRange();
        }
        pic->planes[VPX_PLANE_Y] = YPLANE(image);
        pic->planes[VPX_PLANE_U] = UPLANE(image);
        pic->planes[VPX_PLANE_V] = VPLANE(image);
        pic->stride[VPX_PLANE_Y] = image->GetPitch(PLANAR_Y);
        pic->stride[VPX_PLANE_U] = image->GetPitch(PLANAR_U);
        pic->stride[VPX_PLANE_V] = image->GetPitch(PLANAR_V);
        pic->bit_depth = 8;

        pts=image->Pts;
        queueOfDts.push_back(pts);
        uint64_t real=pts;
        pts=scaleTime(param.g_timebase.num, param.g_timebase.den, real);
        if(pts<=lastScaledPts) // either wrong timebase or bogus pts
            pts=lastScaledPts+1;
        lastScaledPts=pts;
        ADM_timeMapping map;
        map.realTS=real;
        map.internalTS=pts;
        mapper.push_back(map);

        er=vpx_codec_encode(&context,pic,pts,scaledFrameDuration,0,dline);
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
    uint32_t encmode=vp9Settings.ratectl.mode;
    if(encmode==COMPRESS_2PASS || encmode==COMPRESS_2PASS_BITRATE)
        return true;
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
        if(passNumber != 1 && pkt->kind != VPX_CODEC_CX_FRAME_PKT)
        {
            const char *msg=packetTypeToString(pkt->kind);
            ADM_info("Got packet of type: %s\n",msg);
            ADM_dealloc(msg);
            msg=NULL;
            continue;
        }
        if(passNumber == 1 && pkt->kind != VPX_CODEC_STATS_PKT)
        {
            const char *msg=packetTypeToString(pkt->kind);
            ADM_warning("Unexpected packet type %s during the first pass.\n",msg);
            ADM_dealloc(msg);
            msg=NULL;
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
        if(passNumber!=1)
        {
            int quantizer=0;
            if(VPX_CODEC_OK==vpx_codec_control(&context,VP8E_GET_LAST_QUANTIZER_64,&quantizer))
            { // The value filled by codec control does not apply to this particular frame, but oh well...
                if(quantizer<=0) quantizer=vp9Settings.ratectl.qz;
                out->out_quantizer=quantizer;
            }
            getRealPtsFromInternal(pkt->data.frame.pts,&out->dts,&out->pts);
        }else
        {
            if(queueOfDts.size())
            {
                lastDts=out->dts=out->pts=queueOfDts.front();
                queueOfDts.erase(queueOfDts.begin());
            }else
            {
                lastDts+=getFrameIncrement();
                out->dts=out->pts=lastDts;
            }
            if(!statFd)
            {
                statFd=ADM_fopen(logFile.c_str(),"wb");
                if(!statFd)
                {
                    ADM_error("Cannot open log file %s for writing.\n",logFile.c_str());
                    return false;
                }
            }
            ADM_fwrite(out->data,out->len,1,statFd);
        }
        if(pkt->data.frame.flags & VPX_FRAME_IS_KEY)
            out->flags=AVI_KEY_FRAME;
        return true;
    }

    return false;
}

// EOF

