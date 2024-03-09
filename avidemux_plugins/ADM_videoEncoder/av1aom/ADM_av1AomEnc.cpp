/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_av1AomEnc.h"
#include "av1aom_encoder.h"

#define MMSET(x) memset(&(x),0,sizeof(x))

av1aom_encoder encoderSettings = AV1_DEFAULT_CONF;

/**
 *  \fn av1AomEncoder
 */
av1AomEncoder::av1AomEncoder(ADM_coreVideoFilter *src, bool globalHeader) : ADM_coreVideoEncoder(src)
{
    ADM_info("Creating libaom AV1 encoder\n");
    MMSET(context);
    MMSET(param);
    iface = NULL;
    pic = NULL;
    flush = false;
    passNumber = 0;
    globalStreamHeader = globalHeader;
    extraDataLen = 0;
    extraData = NULL;
    statFd = NULL;
    statBuf = NULL;
    lastScaledPts = ADM_NO_PTS;
}

static void dumpParams(aom_codec_enc_cfg_t *cfg)
{
    int i;
    printf("\n");
#define PI(x) printf(#x":\t%d\n",(int)cfg->x);
    PI(g_usage)
    PI(g_threads)
    PI(g_profile)
    PI(g_w)
    PI(g_h)
    PI(g_limit)
    PI(g_forced_max_frame_width)
    PI(g_forced_max_frame_height)
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
    PI(rc_resize_mode)
    PI(rc_resize_denominator)
    PI(rc_resize_kf_denominator)
    PI(rc_superres_mode)
    PI(rc_superres_denominator)
    PI(rc_superres_kf_denominator)
    PI(rc_superres_qthresh)
    PI(rc_superres_kf_qthresh)
    PI(rc_end_usage)
#define PP(x) printf(#x":\t%p\n",cfg->x);
    PP(rc_twopass_stats_in.buf)
    PI(rc_twopass_stats_in.sz)
    PP(rc_firstpass_mb_stats_in.buf)
#undef PP
    PI(rc_firstpass_mb_stats_in.sz)
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
    printf("**********************************\n");
    printf("******  keyframe settings  *******\n");
    printf("**********************************\n");
    PI(fwd_kf_enabled)
    PI(kf_mode)
    PI(kf_min_dist)
    PI(kf_max_dist)
    PI(sframe_dist)
    PI(sframe_mode)
    printf("*********************************\n");
    printf("******  tile coding mode  *******\n");
    printf("*********************************\n");
    PI(large_scale_tile)
    PI(monochrome)
    PI(full_still_picture_hdr)
    PI(save_as_annexb)
    PI(tile_width_count)
    PI(tile_height_count)
#define LOOPI(x,y,b,m) { \
    int nb=cfg->b; if(nb<0) nb=0; if(nb>m) nb=m; \
    for(i=0; i<nb; i++) { \
        printf(#x", "#y" %d:\t%d\n",i,cfg->x[i]); \
    } \
}
    LOOPI(tile_widths, tile_width, tile_width_count, MAX_TILE_WIDTHS)
    LOOPI(tile_heights, tile_height, tile_height_count, MAX_TILE_HEIGHTS)
#undef LOOPI
    printf("\n");
#if AOM_ENCODER_ABI_VERSION >= (10 + AOM_CODEC_ABI_VERSION + /*AOM_EXT_PART_ABI_VERSION=*/3)
    PI(use_fixed_qp_offsets)
    // skipping deprecated and ignored fixed_qp_offsets array
#undef PI
    printf("**********************************\n");
    printf("****  encoder config options  ****\n");
    printf("**********************************\n");
#define PI(x) printf(#x":\t%d\n",(int)cfg->encoder_cfg.x);
    PI(init_by_cfg_file)
    PI(super_block_size)
    PI(max_partition_size)
    PI(min_partition_size)
    PI(disable_ab_partition_type)
    PI(disable_rect_partition_type)
    PI(disable_1to4_partition_type)
    PI(disable_flip_idtx)
    PI(disable_cdef)
    PI(disable_lr)
    PI(disable_obmc)
    PI(disable_warp_motion)
    PI(disable_global_motion)
    PI(disable_dist_wtd_comp)
    PI(disable_diff_wtd_comp)
    PI(disable_inter_intra_comp)
    PI(disable_masked_comp)
    PI(disable_one_sided_comp)
    PI(disable_palette)
    PI(disable_intrabc)
    PI(disable_cfl)
    PI(disable_smooth_intra)
    PI(disable_filter_intra)
    PI(disable_dual_filter)
    PI(disable_intra_angle_delta)
    PI(disable_intra_edge_filter)
    PI(disable_tx_64x64)
    PI(disable_smooth_inter_intra)
    PI(disable_inter_inter_wedge)
    PI(disable_inter_intra_wedge)
    PI(disable_paeth_intra)
    PI(disable_trellis_quant)
    PI(disable_ref_frame_mv)
    PI(reduced_reference_set)
    PI(reduced_tx_type_set)
#endif
#undef PI
    printf("\n");
}

static const char *packetTypeToString(int type)
{
    char msg[32] = {0};
#define REPORT(x) case x: snprintf(msg,32,#x);break;
    switch(type)
    {
        REPORT(AOM_CODEC_CX_FRAME_PKT)
        REPORT(AOM_CODEC_STATS_PKT)
        REPORT(AOM_CODEC_FPMB_STATS_PKT)
        REPORT(AOM_CODEC_PSNR_PKT)
        REPORT(AOM_CODEC_CUSTOM_PKT)
        default: snprintf(msg, 32, "unknown packet type"); break;
    }
    return ADM_strdup(msg);
}

static int64_t scaleTime(uint32_t num, uint32_t den, uint64_t time)
{
    if(time == ADM_NO_PTS)
        return (int64_t)time;
    ADM_assert(!(time>>62));
    ADM_assert(num);
    double d = time;
    d /= 1000.;
    d *= den;
    d /= 1000.;
    d /= num;
    d += 0.49;
    return (int64_t)d;
}

/**
 * \fn setup
 */
bool av1AomEncoder::setup(void)
{
    aom_codec_err_t ret;

    lastScaledPts = ADM_NO_PTS;
    flush = false;
    outQueue.clear();

    image = new ADMImageDefault(getWidth(), getHeight());

    iface = aom_codec_av1_cx();
    if(!iface)
    {
        ADM_error("No AV1 encoder interface available.\n");
        return false;
    }

    int usage = (encoderSettings.usage > AOM_USAGE_REALTIME) ? AOM_USAGE_GOOD_QUALITY : encoderSettings.usage;
    ret = aom_codec_enc_config_default(iface, &param, usage);

    if(ret != AOM_CODEC_OK)
    {
        ADM_error("[av1AomEncoder] Cannot create default configuration, error %d: \"%s\".\n",(int)ret, aom_codec_err_to_string(ret));
        return false;
    }
    ADM_info("Initial default config:\n");
    dumpParams(&param);

    param.g_w = getWidth();
    param.g_h = getHeight();
    uint32_t threads = encoderSettings.autoThreads? ADM_cpu_num_processors() : encoderSettings.nbThreads;
    if(threads > AV1_ENC_MAX_THREADS) threads = AV1_ENC_MAX_THREADS;
    param.g_threads = encoderSettings.nbThreads = threads;

    FilterInfo *info = source->getInfo();
    param.g_timebase.num = info->timeBaseNum & 0x7FFFFFFF;
    param.g_timebase.den = info->timeBaseDen & 0x7FFFFFFF;
    ADM_assert(param.g_timebase.num);
    ADM_assert(param.g_timebase.den);
#define MAX_CLOCK 180000
    if(isStdFrameRate(param.g_timebase.den, param.g_timebase.num))
    {
        int64_t dur = scaleTime(param.g_timebase.num, param.g_timebase.den, info->frameIncrement);
        if(dur < 1) dur = 1;
        if(dur > MAX_CLOCK) dur = MAX_CLOCK;
        scaledFrameDuration = dur;
    }else
    {
        usSecondsToFrac(info->frameIncrement, &(param.g_timebase.num), &(param.g_timebase.den), MAX_CLOCK);
        scaledFrameDuration = 1;
    }

    int speed = encoderSettings.speed;
    if (speed > 11) speed = 11;
    if (speed > 9)
    {
        if (usage != AOM_USAGE_REALTIME)
        {
            speed = 9;
        } else if (aom_codec_version_major() == 3)
        {
#if 0 /* we now require v3.2.0 via cmake */
            if (aom_codec_version_minor() < 2) // 10 && AOM_USAGE_REALTIME available since 3.2.0
                speed = 9;
            else
#endif
            if (aom_codec_version_minor() < 7)
                speed = 10;
        }
    }

    switch(encoderSettings.ratectl.mode)
    {
        case COMPRESS_CQ:
            if(param.rc_min_quantizer > encoderSettings.ratectl.qz)
                ADM_warning("Requested minimum quantizer %d exceeds min value %d for good quality.\n",
                    encoderSettings.ratectl.qz, param.rc_min_quantizer);
            if(param.rc_max_quantizer < encoderSettings.ratectl.qz)
                ADM_warning("Requested maximum quantizer %d exceeds max value %d for good quality.\n",
                    encoderSettings.ratectl.qz, param.rc_max_quantizer);
            param.rc_max_quantizer = param.rc_min_quantizer = encoderSettings.ratectl.qz;
            param.rc_end_usage = AOM_Q;
            break;
        case COMPRESS_CBR:
            param.rc_target_bitrate = encoderSettings.ratectl.bitrate;
            param.rc_end_usage = AOM_CBR;
            break;
        case COMPRESS_2PASS:
        case COMPRESS_2PASS_BITRATE:
            if(passNumber != 1 && passNumber != 2)
            {
                ADM_error("Invalid pass number %d provided.\n",(int)passNumber);
                return false;
            }
            ADM_info("[av1AomEncoder] Starting pass %d\n",passNumber);
            if(passNumber != 1)
            {
                int64_t sz = ADM_fileSize(logFile.c_str());
                if(sz <= 0)
                {
                    ADM_error("Stats file not found or empty, cannot proceed with the second pass.\n");
                    return false;
                }
                if(sz > (1LL<<30))
                {
                    ADM_error("Stats file size %" PRId64" exceeds one GiB, this cannot be right, not trying to load it into memory.\n", sz);
                    return false;
                }
                statBuf = ADM_alloc(sz);
                if(!statBuf)
                {
                    ADM_error("Allocating memory for stats from the first pass failed.\n");
                    return false;
                }
                statFd = ADM_fopen(logFile.c_str(), "r");
                if(!ADM_fread(statBuf, sz, 1, statFd))
                {
                    ADM_error("Reading stats file %s failed.\n", logFile.c_str());
                    fclose(statFd);
                    statFd = NULL;
                    return false;
                }
                fclose(statFd);
                statFd = NULL;
                param.rc_twopass_stats_in.buf = statBuf;
                param.rc_twopass_stats_in.sz = sz;
            }
            {
                uint32_t bitrate=0;
                if(encoderSettings.ratectl.mode == COMPRESS_2PASS)
                {
                    uint64_t duration = source->getInfo()->totalDuration;
                    if(encoderSettings.ratectl.finalsize)
                    {
                        if(false == ADM_computeAverageBitrateFromDuration(duration, encoderSettings.ratectl.finalsize, &bitrate))
                            return false;
                    }
                }else
                {
                    bitrate=encoderSettings.ratectl.avg_bitrate;
                }
                if(bitrate)
                {
                    param.rc_target_bitrate = bitrate;
                    param.rc_2pass_vbr_maxsection_pct = 100; // max. bitrate = 2*target
                    param.rc_end_usage = AOM_CQ;
                }else
                {
                    param.rc_target_bitrate = 0;
                    param.rc_end_usage = AOM_Q;
                }
            }
            break;
        default:
            break;
    }
#ifndef AOM_RC_SECOND_PASS
    #define AOM_RC_SECOND_PASS AOM_RC_LAST_PASS
#endif
    param.g_pass = (!passNumber) ? AOM_RC_ONE_PASS : (passNumber == 1)? AOM_RC_FIRST_PASS : AOM_RC_SECOND_PASS;
    param.kf_max_dist = encoderSettings.keyint;

    ADM_info("Trying to init encoder with the following configuration:\n");
    dumpParams(&param);

    ret = aom_codec_enc_init(&context, iface, &param, 0);
    if(ret != AOM_CODEC_OK)
    {
        ADM_error("[av1aom] Init failed with error %d: \"%s\"\n", (int)ret, aom_codec_err_to_string(ret));
        return false;
    }

    uint32_t alignment=16;

    pic = aom_img_alloc(pic, AOM_IMG_FMT_I420, param.g_w, param.g_h, alignment);
    if(!pic)
    {
        ADM_error("[av1aom] Cannot allocate AOM image.\n");
        return false;
    }

    pic->range = encoderSettings.fullrange ? AOM_CR_FULL_RANGE : AOM_CR_STUDIO_RANGE;
    pic->bit_depth = 8;

    if(AOM_CODEC_OK != aom_codec_control(&context, AOME_SET_CPUUSED, speed))
    {
        ADM_warning("[av1aom] Cannot set AOME_SET_CPUUSED codec control to %d\n", speed);
    }
    if((param.rc_end_usage == AOM_CQ || param.rc_end_usage == AOM_Q ) &&
        AOM_CODEC_OK != aom_codec_control(&context, AOME_SET_CQ_LEVEL, encoderSettings.ratectl.qz))
    {
        ADM_warning("[av1aom] Cannot set AOME_SET_CQ_LEVEL codec control to %u\n", encoderSettings.ratectl.qz);
    }
    if(AOM_CODEC_OK != aom_codec_control(&context, AV1E_SET_COLOR_RANGE, encoderSettings.fullrange))
    {
        ADM_warning("[av1aom] Cannot set AV1E_SET_COLOR_RANGE codec control to %d\n", encoderSettings.fullrange);
    }
    unsigned int colCount = ((encoderSettings.tiling >> 0) & 0xFFFF);
    if(AOM_CODEC_OK != aom_codec_control(&context, AV1E_SET_TILE_COLUMNS, colCount))
    {
        ADM_warning("[av1aom] Cannot set AV1E_SET_TILE_COLUMNS codec control to %u\n", colCount);
    }
    unsigned int rowCount = ((encoderSettings.tiling >> 16) & 0xFFFF);
    if(AOM_CODEC_OK != aom_codec_control(&context, AV1E_SET_TILE_ROWS, rowCount))
    {
        ADM_warning("[av1aom] Cannot set AV1E_SET_TILE_ROWS codec control to %u\n", rowCount);
    }
    
    if(globalStreamHeader)
    {
        aom_fixed_buf_t *hdr = aom_codec_get_global_headers(&context);
        if(!hdr)
        {
            ADM_warning("libaom unable to generate global header.\n");
        }else
        { // TODO: sanity check
            extraDataLen = hdr->sz;
            extraData = new uint8_t[extraDataLen];
            memcpy(extraData, hdr->buf, extraDataLen);
            free(hdr->buf);
            hdr->buf = NULL;
            free(hdr);
            hdr = NULL;
        }
    }

    return true;
}

/**
 *  \fn ~av1AomEncoder
 */
av1AomEncoder::~av1AomEncoder()
{
    ADM_info("[av1aom] Destroying.\n");
    for(int i=0; i < outQueue.size(); i++)
    {
        ADMBitstream *s = outQueue[i];
        if(!s) continue;
        ADM_dealloc(s->data);
        delete s;
        s = NULL;
    }
    if(pic)
    {
        aom_img_free(pic);
        pic = NULL;
    }
    if(statFd)
        fclose(statFd);
    statFd = NULL;

    aom_codec_destroy(&context);

    ADM_dealloc(statBuf);
    statBuf = NULL;
    if(extraData)
    {
        delete [] extraData;
        extraData = NULL;
    }
}

/**
 *  \fn setPassAndLogFile
 */
bool av1AomEncoder::setPassAndLogFile(int pass, const char *name)
{
    ADM_info("Initializing pass %d, log file: %s\n", pass, name);
    logFile = name;
    passNumber = pass;
    return true;
}

/**
    \fn encode
*/
bool av1AomEncoder::encode(ADMBitstream *out)
{
    uint32_t nb;
    uint64_t pts;
    aom_codec_err_t er;

again:
    out->flags = 0;
    // fetch a frame...
    if(!flush && !source->getNextFrame(&nb, image))
    {
        ADM_warning("[av1aom] Cannot get next image\n");
        flush = true;
    }
    if(flush)
    {
        ADM_info("Flushing delayed frames\n");
        pts = lastScaledPts + scaledFrameDuration; // fake
        lastScaledPts = pts;

        er = aom_codec_encode(&context, NULL, pts, scaledFrameDuration, 0);
    }else
    {
        pic->planes[AOM_PLANE_Y] = YPLANE(image);
        pic->planes[AOM_PLANE_U] = UPLANE(image);
        pic->planes[AOM_PLANE_V] = VPLANE(image);
        pic->stride[AOM_PLANE_Y] = image->GetPitch(PLANAR_Y);
        pic->stride[AOM_PLANE_U] = image->GetPitch(PLANAR_U);
        pic->stride[AOM_PLANE_V] = image->GetPitch(PLANAR_V);

        pts = image->Pts;
        queueOfDts.push_back(pts);
        uint64_t real = pts;
        pts = scaleTime(param.g_timebase.num, param.g_timebase.den, real);
        if(lastScaledPts != ADM_NO_PTS && pts <= lastScaledPts) // either wrong timebase or bogus pts
            pts = lastScaledPts + 1;
        lastScaledPts = pts;
        ADM_timeMapping map;
        map.realTS = real;
        map.internalTS = pts;
        mapper.push_back(map);

        er = aom_codec_encode(&context, pic, pts, scaledFrameDuration, 0);
    }
    if(er != AOM_CODEC_OK)
    {
        ADM_error("Encoding error %d: %s\n", (int)er, aom_codec_err_to_string(er));
        return false;
    }
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
bool av1AomEncoder::isDualPass(void)
{
    switch(encoderSettings.ratectl.mode)
    {
        case COMPRESS_2PASS:
        case COMPRESS_2PASS_BITRATE:
            return true;
        default:break;
    }
    return false;
}

/**
    \fn postAmble
    \brief Collect compressed frames
*/
bool av1AomEncoder::postAmble(ADMBitstream *out)
{
    const aom_codec_cx_pkt_t *pkt = NULL;
    aom_codec_iter_t iter = NULL;
    ADMBitstream *stream = NULL;

    while((pkt = aom_codec_get_cx_data(&context, &iter)) != NULL)
    {
        if((passNumber == 1 && pkt->kind != AOM_CODEC_STATS_PKT) ||
            passNumber != 1 && pkt->kind != AOM_CODEC_CX_FRAME_PKT)
        {
            const char *msg = packetTypeToString(pkt->kind);
            ADM_warning("Skipping packet of unexpected type \"%s\".\n", msg);
            ADM_dealloc(msg);
            msg = NULL;
            continue;
        }
        if(passNumber != 1 && pkt->kind == AOM_CODEC_CX_FRAME_PKT)
        {
            stream = new ADMBitstream(pkt->data.frame.sz);
            if(pkt->data.frame.sz) // can this be taken for granted and check skipped?
            {
                stream->data = (uint8_t *)ADM_alloc(stream->bufferSize);
                stream->len = stream->bufferSize;
                memcpy(stream->data, pkt->data.frame.buf, stream->len);
            }
            getRealPtsFromInternal(pkt->data.frame.pts, &stream->dts, &stream->pts);

            if(pkt->data.frame.flags & AOM_FRAME_IS_KEY) // FIXME What is about AOM_FRAME_IS_DELAYED_RANDOM_ACCESS_POINT?
                stream->flags = AVI_KEY_FRAME; // else P

            int qz = 0;
            if(AOM_CODEC_OK == aom_codec_control(&context, AOME_GET_LAST_QUANTIZER_64, &qz))
            {
                if(qz <= 0)
                    qz = encoderSettings.ratectl.qz;
                stream->out_quantizer = qz;
            }
            outQueue.push_back(stream);
            continue;
        }
        if(passNumber == 1 && pkt->kind == AOM_CODEC_STATS_PKT)
        {
            stream = new ADMBitstream();
            if(queueOfDts.size())
            {
                lastDts = stream->dts = stream->pts = queueOfDts.front();
                queueOfDts.erase(queueOfDts.begin());
            }else
            {
                lastDts += getFrameIncrement();
                stream->dts = stream->pts = lastDts;
            }

            outQueue.push_back(stream); // during the first pass, we need only dts and pts

            if(!pkt->data.twopass_stats.sz) continue;

            if(!statFd)
            {
                statFd = ADM_fopen(logFile.c_str(), "wb");
                if(!statFd)
                {
                    ADM_error("Cannot open log file %s for writing.\n", logFile.c_str());
                    return false;
                }
            }
            ADM_fwrite(pkt->data.twopass_stats.buf, pkt->data.twopass_stats.sz, 1, statFd);
        }
    }

    if(outQueue.size())
    {
        stream = outQueue.front();
        ADM_assert(stream);
        outQueue.erase(outQueue.begin());
        out->pts = stream->pts;
        out->dts = stream->dts;
        if(passNumber == 1)
        {
            delete stream;
            stream = NULL;
            return true;
        }
        ADM_assert(stream->len <= out->bufferSize);
        memcpy(out->data, stream->data, stream->len);
        out->len = stream->len;
        out->flags = stream->flags;
        out->out_quantizer = stream->out_quantizer;
        // clean up
        ADM_dealloc(stream->data);
        delete stream;
        stream = NULL;
        return true;
    }

    return false;
}

// EOF

