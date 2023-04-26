/***************************************************************************
                          \fn ADM_x264
                          \brief Front end for x264 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2002/2011 by mean/gruntster
    email                : fixounet@free.fr/gruntster@razorbyte.au
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
#include "ADM_x264.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_coreUtils.h"
#include <climits>

#if 1
    #define aprintf(...) {}
    #define avsnprintf(...) {}
#else
    #define aprintf ADM_info
    #define avsnprintf vsnprintf
#endif
static const char *TrueFalse[2]={"False","True"};
static void dumpx264Setup(x264_param_t *param);
extern "C" 
{
static void        logger( void *cooki, int i_level, const char *psz, va_list list)
{
    static char buffer[2048];
    avsnprintf(buffer,2048,psz,list);
    aprintf(">>%s\n",buffer);
}
}
#define MMSET(x) memset(&(x),0,sizeof(x))

extern x264_encoder x264Settings;
/**
    \fn setup
*/
bool x264Encoder::setup(void)
{
  ADM_info("=============x264, setting up==============\n");
  MMSET(param);

  x264_param_default( &param);
  param.pf_log=logger;
  firstIdr=true;
  image=new ADMImageDefault(getWidth(),getHeight());

  // -------------- preset, tune, idc ------------
  if(!x264Settings.useAdvancedConfiguration)
  {
        std::string tune;
        if(x264Settings.general.tuning != "none")
            tune = x264Settings.general.tuning;
        if(x264Settings.general.fast_decode)
        {
            tune += ",";
            tune += "fastdecode";
        }
        if(x264Settings.general.zero_latency)
        {
            tune += ",";
            tune += "zerolatency";
        }
        if(tune.empty())
            x264_param_default_preset(&param, x264Settings.general.preset.c_str(), NULL);
        else
            x264_param_default_preset(&param, x264Settings.general.preset.c_str(), tune.c_str());
  }else
  {
        param.b_bluray_compat=x264Settings.general.blueray_compatibility;
        param.b_fake_interlaced=x264Settings.general.fake_interlaced;
  }
  param.i_level_idc=x264Settings.level; 

  // Threads..
  switch(x264Settings.general.threads)
  {
    case 1: case 2: case 4: param.i_threads = x264Settings.general.threads;break;
    case 0: case 99: break; //auto
    default: ADM_error("UNKNOWN NB OF THREADS\n");break;
  }
  param.i_width = getWidth();
  param.i_height = getHeight();
  param.i_csp = X264_CSP_I420;
  param.i_log_level=X264_LOG_INFO; //DEBUG; //INFO;

  //Framerate
  int n,d;
  uint64_t f=source->getInfo()->frameIncrement;
  usSecondsToFrac(f,&n,&d,INT_MAX);
  param.i_fps_num = d;
  param.i_fps_den = n;
  n = source->getInfo()->timeBaseNum & 0x7FFFFFFF;
  d = source->getInfo()->timeBaseDen & 0x7FFFFFFF;
  ADM_assert(d);
  param.i_timebase_num = n;
  param.i_timebase_den = d;
#if 0
  bool constantFps = (n * 1000 * 1000 + (d / 2))/ d >= f;
  param.b_vfr_input = !constantFps;
#else
// We do pseudo cfr ...
  param.b_vfr_input=0;
#endif

  // -------------- vui------------
  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.vui.x = x264Settings.vui.y;aprintf("[x264] vui."#x" = %d\n",param.vui.x);}
  #define MKPARAMF(x,y) {param.vui.x = (float)x264Settings.vui.y; aprintf("[x264] vui."#x" = %.2f\n",param.vui.x);}
  #define MKPARAMB(x,y) {param.vui.x = x264Settings.vui.y ;aprintf("[x264] vui."#x" = %s\n",TrueFalse[param.vui.x&1]);}
  MKPARAM (i_sar_width,sar_width) 
  MKPARAM (i_sar_height,sar_height) 
  MKPARAM (i_overscan,overscan)
  MKPARAM (i_vidformat,vidformat)
  MKPARAMB(b_fullrange,fullrange)
  MKPARAM (i_colorprim,colorprim)
  MKPARAM (i_transfer,transfer)
  MKPARAM (i_colmatrix,colmatrix)
  MKPARAM (i_chroma_loc,chroma_loc)

  // -------------- rate control------------
  switch(x264Settings.general.params.mode)
  {
      case COMPRESS_2PASS:
      case COMPRESS_2PASS_BITRATE:
                        uint32_t bitrate;
                        if(passNumber!=1 && passNumber!=2)
                        {
                                ADM_error("No pass number specified! (%d)\n",(int)passNumber);
                                return false;
                         }
                        ADM_info("Starting pass :%d\n",passNumber);
                        if(x264Settings.general.params.mode==COMPRESS_2PASS)
                        {
                            uint64_t duration=source->getInfo()->totalDuration; // in us
                            ADM_info("Source duration :%s\n",ADM_us2plain(duration));
                            ADM_info("Target size     :%d\n",(int)x264Settings.general.params.finalsize);
                            uint32_t avg;
                            if(false==ADM_computeAverageBitrateFromDuration(duration, 
                                            x264Settings.general.params.finalsize,
                                            &avg))
                            {
                                ADM_error("[x264] No source duration!\n");
                                return false;
                            }
                            bitrate=(uint32_t)avg;
                        }
                        else
                                bitrate=x264Settings.general.params.avg_bitrate;
                        ADM_info("Using average bitrate of %d kb/s\n",(int)bitrate);
                        param.rc.i_rc_method = X264_RC_ABR;
                        param.rc.i_bitrate =  bitrate;
                        if(passNumber==1)
                        {
                             param.rc.b_stat_write=1;
                             param.rc.b_stat_read=0;
                             param.rc.psz_stat_out=logFile;
//#warning FIXME
#if 0
                             if(x264Settings.)
                                    x264_param_apply_fastfirstpass(&_param);
#endif
 
                        }else
                        {
                             param.rc.b_stat_write=0;
                             param.rc.b_stat_read=1;
                             param.rc.psz_stat_in=logFile;
                             if(!ADM_fileExist(logFile))
                             {
                                   ADM_error("Logfile %s does not exist \n",logFile);
                                   return false;
                             }
                        }
                        break;
      case COMPRESS_AQ: param.rc.i_rc_method = X264_RC_CRF;
                        param.rc.f_rf_constant = x264Settings.general.params.qz;
                        break;
      case COMPRESS_CQ: param.rc.i_rc_method = X264_RC_CQP;
                        param.rc.i_qp_constant = x264Settings.general.params.qz;
                        break;

      case COMPRESS_CBR:
                        param.rc.i_rc_method = X264_RC_ABR;
                        param.rc.i_bitrate =  x264Settings.general.params.bitrate;
                        param.rc.i_qp_constant = 0;
                        param.rc.f_rf_constant = 0;
                        break;
        default:
                        GUI_Error_HIG(QT_TRANSLATE_NOOP("x264","Not coded"),QT_TRANSLATE_NOOP("x264","this mode has not been implemented\n"));
                        return false;
                        break;

  }

  if(globalHeader)
      param.b_repeat_headers=0;
  else
      param.b_repeat_headers=1;

  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.x = x264Settings.y;aprintf("[x264] "#x" = %d\n",param.x);}
  #define MKPARAMF(x,y) {param.x = (float)x264Settings.y; aprintf("[x264] "#x" = %.2f\n",param.x);}
  #define MKPARAMB(x,y) {param.x = x264Settings.y ;aprintf("[x264] "#x" = %s\n",TrueFalse[param.x&1]);}

#ifdef SIMPLE_MODE_EX
  MKPARAMB(b_interlaced,interlaced)
  MKPARAMB(b_tff,tff)
  MKPARAM(i_keyint_min,MinIdr)
  MKPARAM(i_keyint_max,MaxIdr)
  MKPARAM(i_scenecut_threshold,i_scenecut_threshold)
  MKPARAMB(b_intra_refresh,intra_refresh)
#endif

  if(x264Settings.useAdvancedConfiguration)
  {
    MKPARAM(i_frame_reference,MaxRefFrames);
#ifndef SIMPLE_MODE_EX
    MKPARAMB(b_interlaced,interlaced)
    MKPARAMB(b_tff,tff)
    MKPARAM(i_keyint_min,MinIdr)
    MKPARAM(i_keyint_max,MaxIdr)
    MKPARAM(i_scenecut_threshold,i_scenecut_threshold)
    MKPARAMB(b_intra_refresh,intra_refresh)
#endif
    MKPARAM(i_bframe,MaxBFrame);
    MKPARAM(i_bframe_adaptive,i_bframe_adaptive);
    MKPARAM(i_bframe_bias,i_bframe_bias);
    MKPARAM(i_bframe_pyramid,i_bframe_pyramid);
    MKPARAMB(b_deblocking_filter,b_deblocking_filter);
    if(param.b_deblocking_filter)
    {
      MKPARAM(i_deblocking_filter_alphac0,i_deblocking_filter_alphac0);
      MKPARAM(i_deblocking_filter_beta,i_deblocking_filter_beta);
    }
    MKPARAMB(b_cabac,cabac);
    MKPARAMB(b_constrained_intra,constrained_intra);
    MKPARAMB(b_fake_interlaced,fake_interlaced);

    // -------------- analyze------------
  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.analyse.x = x264Settings.analyze.y;aprintf("[x264] analyse."#x" = %d\n",param.analyse.x);}
  #define MKPARAMF(x,y) {param.analyse.x = (float)x264Settings.analyze.y; aprintf("[x264] analyse."#x" = %.2f\n",param.analyse.x);}
  #define MKPARAMB(x,y) {param.analyse.x = x264Settings.analyze.y ;aprintf("[x264] analyse."#x" = %s\n",TrueFalse[param.analyse.x&1]);}
  #define MKFLAGS(fieldout,fieldin,mask) {if(x264Settings.analyze.fieldin) param.analyse.fieldout|=mask;}
     MKPARAMB(b_transform_8x8,b_8x8)
     MKPARAMB(b_weighted_bipred,weighted_bipred) 
     MKPARAM (i_weighted_pred,weighted_pred) 
     MKPARAM (i_direct_mv_pred,direct_mv_pred)
     MKPARAM (i_chroma_qp_offset,chroma_offset)

     MKPARAM (i_me_method,me_method) 
     MKPARAM (i_me_range,me_range)
     MKPARAM (i_mv_range,mv_range) 
     MKPARAM (i_mv_range_thread,mv_range_thread)
     MKPARAM (i_subpel_refine,subpel_refine) 
     MKPARAMB(b_chroma_me,chroma_me) 
     MKPARAMB(b_mixed_references,mixed_references) 
     MKPARAM (i_trellis,trellis) 
     MKPARAMB(b_fast_pskip,fast_pskip) 
     MKPARAMB(b_dct_decimate,dct_decimate) 
     MKPARAMB(b_psy,psy) 
     MKPARAMF(f_psy_rd,psy_rd) 
     MKPARAMF(f_psy_trellis,psy_trellis)
     MKPARAM (i_noise_reduction,noise_reduction)
     MKPARAM (i_luma_deadzone[0],inter_luma) 
     MKPARAM (i_luma_deadzone[1],intra_luma) 

     MKFLAGS(inter,b_i4x4,X264_ANALYSE_I4x4)
     MKFLAGS(inter,b_i8x8,X264_ANALYSE_I8x8)
     MKFLAGS(inter,b_p16x16,X264_ANALYSE_PSUB16x16)
     MKFLAGS(inter,b_p8x8,X264_ANALYSE_PSUB8x8)
     MKFLAGS(inter,b_b16x16,X264_ANALYSE_BSUB16x16)

     //---------------- ratecontrol -------------------
  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y)  {param.rc.x = x264Settings.ratecontrol.y;aprintf("[x264] rc."#x" = %d\n",param.rc.x);}
  #define MKPARAMF(x,y) {param.rc.x = (float)x264Settings.ratecontrol.y; aprintf("[x264] rc."#x" = %.2f\n",param.rc.x);}
  #define MKPARAMB(x,y) {param.rc.x = x264Settings.ratecontrol.y ;aprintf("[x264] rc."#x" = %s\n",TrueFalse[param.rc.x&1]);}

      MKPARAM(i_qp_min,qp_min);
      MKPARAM(i_qp_max,qp_max);
      MKPARAM(i_qp_step,qp_step);
      MKPARAM(f_rate_tolerance,rate_tolerance);
      MKPARAM(f_ip_factor,ip_factor);
      MKPARAM(f_pb_factor,pb_factor);
      MKPARAMB(b_mb_tree,mb_tree);
      MKPARAM(i_lookahead,lookahead);
      MKPARAM(i_aq_mode,aq_mode);
      MKPARAMF(f_aq_strength,aq_strength);
      
      MKPARAM(i_vbv_max_bitrate,vbv_max_bitrate)
      MKPARAM(i_vbv_buffer_size,vbv_buffer_size)
              
      param.rc.f_vbv_buffer_init=((float)x264Settings.ratecontrol.vbv_buffer_init)/100.;
  }
  
  if(!param.i_bframe)  encoderDelay=0;
  else    
  {
      if(2>=param.i_frame_reference) 
      {
          encoderDelay=f*2*2;
      }
      else
      {
              encoderDelay=2*f*(x264Settings.MaxRefFrames-1);
      }
  }

 //
  if(true==x264Settings.general.fast_first_pass)
  {
    if(passNumber==1)
    {
        switch(x264Settings.general.params.mode)
        {
            case COMPRESS_2PASS:
            case COMPRESS_2PASS_BITRATE:
                     ADM_info("Appling fast first pass settings\n");
                     x264_param_apply_fastfirstpass(&param);
                    break;
            default:
                    break;
        }
    }
  }

  if(!x264Settings.useAdvancedConfiguration)
  {
    x264_param_apply_profile(&param, x264Settings.general.profile.c_str());
    if (x264Settings.general.profile == std::string("high10"))
    {
        outputBitDepth = 10;
    }
  }

  setConstraintsByLevel();
  
#if X264_BUILD >= 153
  if (outputBitDepth > 8)
  {
    param.i_bitdepth = outputBitDepth;
    param.i_csp |= X264_CSP_HIGH_DEPTH;
    highBitDepthImage = new ADMImageRefWrittable(getWidth(),getHeight());
    int pitch = ADM_IMAGE_ALIGN(getWidth());
    highBitDepthImage->_planeStride[0] = pitch*2;
    highBitDepthImage->_planeStride[1] = pitch;
    highBitDepthImage->_planeStride[2] = pitch;
    highBitDepthBuffers.setSize(pitch*2*getHeight() + 2*((getHeight()/2) * pitch) + 64);
    highBitDepthImage->_planes[0] = highBitDepthBuffers.at(0);
    highBitDepthImage->_planes[1] = highBitDepthBuffers.at(pitch*2*getHeight());
    highBitDepthImage->_planes[2] = highBitDepthBuffers.at(pitch*2*getHeight() + ((getHeight()/2) * pitch));
  }
#endif
  
  dumpx264Setup(&param);
  ADM_info("Creating x264 encoder\n");
  handle = x264_encoder_open (&param);
  if (!handle)
  {
    ADM_error("Cannot initialize x264\n");
    return 0;
  }

  
  ADM_info("x264, setup ok\n");
  if (globalHeader)
  {
      ADM_info("Creating global header\n");
      return createHeader ();
  }else
      ADM_info("No need for global header\n");
    
  return true;
}
/**
    \fn setConstraintsByLevel
    \brief Copied from Handbrake code
*/
bool x264Encoder::setConstraintsByLevel(void)
{
    if(param.i_level_idc < 0)
        return true;
    if(param.i_width <= 0 || param.i_height <= 0)
    {
        ADM_warning("Invalid dimensions: %d:%d\n",param.i_width,param.i_height);
        return false;
    }
    const x264_level_t *level = NULL;
    int val = param.i_level_idc;
    for(int i=0; x264_levels[i].level_idc; i++)
    {
        if(x264_levels[i].level_idc == val)
        {
            level = &x264_levels[i];
            break;
        }
    }
    if(!level)
    {
        ADM_warning("Invalid level %d\n",val);
        return false;
    }
    if(level->frame_only && (param.b_interlaced || param.b_fake_interlaced))
    {
        ADM_warning("Interlaced flag not supported for level %d, disabling",val);
        param.b_interlaced = param.b_fake_interlaced = 0;
    }
    // Calculate frame dimensions and rate (in macroblocks)
    int mbW = (param.i_width + 15) >> 4;
    int mbH = (param.i_height + 15) >> 4;
    if(param.b_interlaced || param.b_fake_interlaced)
    {
        // interlaced: encoded height must divide cleanly by 32
        mbH = (mbH + 1) & ~1;
    }
    int nbMb = mbW * mbH;
    int mbRate;
    if(param.i_fps_den <= 0 || param.i_fps_num <= 0)
        mbRate = 0;
    else
        mbRate = (int64_t)nbMb * param.i_fps_num / param.i_fps_den;
    // Sanitize frame_reference
    if(param.i_keyint_max != 1)
    {
        int decBufSize = level->dpb / nbMb;
        if(decBufSize < 1) decBufSize = 1;
        if(decBufSize > 16) decBufSize = 16;
        if(decBufSize < param.i_frame_reference)
        {
            ADM_warning("Number of ref frames %d too high for the IDC level, setting to %d\n",param.i_frame_reference,decBufSize);
            param.i_frame_reference = decBufSize;
        }
        // Now disable B-frames and pyramid if necessary.
        if(decBufSize < 2)
        {
            if(param.i_bframe)
            {
                ADM_warning("B-frames forbidden by the IDC level, disabling.\n");
                param.i_bframe = 0;
            }
        }else if(decBufSize < 4 && param.i_bframe_pyramid != X264_B_PYRAMID_NONE)
        {
            ADM_warning("B-frame pyramid forbidden by the IDC level, disabling.\n");
            param.i_bframe_pyramid = X264_B_PYRAMID_NONE;
        }
    }
    // Sanitize VBV depending on profile, simple mode only
    if(!x264Settings.useAdvancedConfiguration && x264Settings.general.profile != std::string("high444"))
    {
        int codedPicBufFactor = 4;
        if(x264Settings.general.profile == std::string("high"))
            codedPicBufFactor = 5;
        if(x264Settings.general.profile == std::string("high10"))
            codedPicBufFactor = 12;
        int maxVbvBitrate = (level->bitrate * codedPicBufFactor) >> 2;
        if(!param.rc.i_vbv_max_bitrate || param.rc.i_vbv_max_bitrate > maxVbvBitrate)
        {
            param.rc.i_vbv_max_bitrate = maxVbvBitrate;
        }
        int maxVbvBufSize = (level->cpb * codedPicBufFactor) >> 2;
        if(!param.rc.i_vbv_buffer_size || param.rc.i_vbv_buffer_size > maxVbvBitrate)
        {
            param.rc.i_vbv_buffer_size = maxVbvBitrate;
        }
    }
    // Log warnings about constraints violation which are impossible to correct
    if(level->frame_size < nbMb)
        ADM_warning("Too many macroblocks per frame for the IDC level: %d (max: %d)\n",nbMb,level->frame_size);
    if(level->mbps < mbRate)
    {
        ADM_warning("Framerate %d/%d too high for IDC level (mb/s: %d, max: %d)\n",param.i_fps_num,param.i_fps_den,mbRate,level->mbps);
    }
    return true;
}
/**
    \fn dumpx264Setup
*/
static void dumpx264Setup(x264_param_t *param)
{
#define STR(x) #x
#define MKSTRING(x) STR(x)
#define PI(x) printf("%-24s : %d\n",MKSTRING(x),(int)param->x);
    PI(cpu)
    PI(i_threads)
    PI(b_sliced_threads)
    PI(b_deterministic)
    PI(i_sync_lookahead)

    PI(i_width)
    PI(i_height)
    PI(i_width)
    PI(i_level_idc)
    PI(i_frame_total)

#define VI(x) printf("%-24s : %d\n",MKSTRING(x),(int)param->vui.x);
    VI(i_sar_height)
    VI(i_sar_width)
    VI(i_overscan)
    VI(i_vidformat)
    VI(b_fullrange)
    VI(i_colorprim)
    VI(i_transfer)
    VI(i_colmatrix)
    VI(i_chroma_loc)

    PI(i_fps_num)
    PI(i_fps_den)

    PI(i_timebase_num)
    PI(i_timebase_den)
    PI(b_vfr_input)

    PI(i_frame_reference)
    PI(i_keyint_max)
    PI(i_keyint_min)
    PI(i_scenecut_threshold)
    PI(b_intra_refresh)

    PI(i_bframe)
    PI(i_bframe_adaptive)
    PI(i_bframe_bias)
    PI(i_bframe_pyramid)

    PI(b_deblocking_filter)
    PI(i_deblocking_filter_alphac0)
    PI(i_deblocking_filter_beta)

    PI(b_cabac)
    PI(i_cabac_init_idc)

    PI(b_interlaced)
    PI(b_tff)
    PI(b_fake_interlaced)
    PI(b_bluray_compat)
    PI(b_constrained_intra)

#define AI(x) printf("%-24s : %d\n",MKSTRING(x),(int)param->analyse.x);
#define AF(x) printf("%-24s : %f\n",MKSTRING(x),(float)param->analyse.x);
    printf("*************************************\n");
    printf("*********     Analyse       *********\n");
    printf("*************************************\n");

    AI(intra)
    AI(inter)

    AI(b_transform_8x8)
    AI(i_weighted_pred)
    AI(b_weighted_bipred)
    AI(i_chroma_qp_offset)

    AI(i_me_method)
    AI(i_me_range)
    AI(i_mv_range)
    AI(i_mv_range_thread)
    AI(i_subpel_refine)
    AI(b_chroma_me)
    AI(b_mixed_references)
    AI(i_trellis)
    AI(b_fast_pskip)

    AI(b_dct_decimate)
    AI(i_noise_reduction)
    AF(f_psy_rd)
    AF(f_psy_trellis)
    AI(b_psy)

    PI(b_aud)
    PI(b_repeat_headers)
    PI(b_annexb)

    AI(i_luma_deadzone[0])
    AI(i_luma_deadzone[1])

    PI(i_sps_id)

    PI(i_slice_max_size)
    PI(i_slice_max_mbs)
    PI(i_slice_count)

#define RI(x) printf("%-24s : %d\n",MKSTRING(x),(int)param->rc.x);
#define RF(x) printf("%-24s : %f\n",MKSTRING(x),(float)param->rc.x);
    printf("*************************************\n");
    printf("*********     RC            *********\n");
    printf("*************************************\n");
    RI(i_rc_method)
    RI(i_qp_constant)
    RF(f_rf_constant)
    RI(i_qp_min)
    RI(i_qp_max)
    RI(i_qp_step)

    RI(i_bitrate)
    RI(i_qp_constant)
    RF(f_rate_tolerance)
    RI(i_vbv_max_bitrate)
    RI(i_vbv_buffer_size)
    RF(f_vbv_buffer_init)
    RF(f_ip_factor)
    RF(f_pb_factor)

    RI(i_aq_mode)
    RF(f_aq_strength)
    RI(b_mb_tree)
    RI(i_lookahead)

}
/**
 * \fn x264LoadProfile
 * @param profile
 */
extern bool  x264_encoder_jserialize(const char *file, const x264_encoder *key);
extern bool  x264_encoder_jdeserialize(const char *file, const ADM_paramList *tmpl,x264_encoder *key);
extern "C" 
{
extern const ADM_paramList x264_encoder_param[];
}

bool x264LoadProfile(const char *profile)
{
    x264_encoder param=x264Settings;
    std::string rootPath;
    ADM_pluginGetPath("x264",1,rootPath);
    std::string fullPath=rootPath+std::string("/")+profile+std::string(".json");
    ADM_info("Trying to load %s\n",fullPath.c_str());
    if(false==x264_encoder_jdeserialize(fullPath.c_str(),x264_encoder_param,&param))
    {
        ADM_warning("Failed\n");
        return false;     
    }
    ADM_info("Profile loaded ok\n");
    x264Settings=param;
    return true;
}

// EOF


