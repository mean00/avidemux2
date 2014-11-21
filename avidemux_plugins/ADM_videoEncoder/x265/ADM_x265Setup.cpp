/***************************************************************************
                          \fn ADM_x265
                          \brief Front end for x265 HEVC encoder
                             -------------------
    
    copyright            : (C) 2002/2014 by mean/gruntster
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
#include "ADM_x265.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "ADM_coreUtils.h"

#if 1
    #define aprintf(...) {}
    #define avsnprintf(...) {}
#else
    #define aprintf ADM_info
    #define avsnprintf vsnprintf
#endif
static const char *TrueFalse[2]={"False","True"};
static void dumpx265Setup(x265_param *param);
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

extern x265_encoder x265Settings;
/**
    \fn setup
*/
bool x265Encoder::setup(void)
{
  ADM_info("=============x265, setting up==============\n");
  MMSET(param);

  x265_param_default( &param);
  param.pf_log=logger;
  firstIdr=true;
  image=new ADMImageDefault(getWidth(),getHeight());

  // -------------- preset, tune, idc ------------
  if(!x265Settings.useAdvancedConfiguration)
  {
    char tune[200] = {0};
    strcat(tune, x265Settings.general.tuning);
    if(x265Settings.general.fast_decode) 
    {
      strcat(tune, ",");
      strcat(tune, "fastdecode");
    }
    if(x265Settings.general.zero_latency)
    {
      strcat(tune, ",");
      strcat(tune, "zero_latency");
    }
    x265_param_default_preset(&param, x265Settings.general.preset, tune);
  }
  param.i_level_idc=x265Settings.level; 

  // Threads..
  switch(x265Settings.general.threads)
  {
    case 0: case 1: case 2:  param.i_threads = x265Settings.general.threads;break;
    case 99:break; //auto
    default: ADM_error("UNKNOWN NB OF THREADS\n");break;
  }
  param.i_width = getWidth();
  param.i_height = getHeight();
  param.i_csp = X265_CSP_I420;
  param.i_log_level=X265_LOG_INFO; //DEBUG; //INFO;

  //Framerate
  int n,d;    
  uint64_t f=source->getInfo()->frameIncrement;
  usSecondsToFrac(f,&n,&d);
  param.i_fps_num = d;
  param.i_fps_den = n;

  // -------------- vui------------
  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.vui.x = x265Settings.vui.y;aprintf("[x265] vui."#x" = %d\n",param.vui.x);}
  #define MKPARAMF(x,y) {param.vui.x = (float)x265Settings.vui.y; aprintf("[x265] vui."#x" = %.2f\n",param.vui.x);}
  #define MKPARAMB(x,y) {param.vui.x = x265Settings.vui.y ;aprintf("[x265] vui."#x" = %s\n",TrueFalse[param.vui.x&1]);}
  MKPARAM (i_sar_width,sar_width) 
  MKPARAM (i_sar_height,sar_height) 

  // -------------- rate control------------
  switch(x265Settings.general.params.mode)
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
                        if(x265Settings.general.params.mode==COMPRESS_2PASS)
                        {
                            uint64_t duration=source->getInfo()->totalDuration; // in us
                            ADM_info("Source duration :%s\n",ADM_us2plain(duration));
                            ADM_info("Target size     :%d\n",(int)x265Settings.general.params.finalsize);
                            uint32_t avg;
                            if(false==ADM_computeAverageBitrateFromDuration(duration, 
                                            x265Settings.general.params.finalsize,
                                            &avg))
                            {
                                ADM_error("[x265] No source duration!\n");
                                return false;
                            }
                            bitrate=(uint32_t)avg;
                        }
                        else
                                bitrate=x265Settings.general.params.avg_bitrate;
                        ADM_info("Using average bitrate of %d kb/s\n",(int)bitrate);
                        param.rc.i_rc_method = X265_RC_ABR;
                        param.rc.i_bitrate =  bitrate;
                        if(passNumber==1)
                        {
                             param.rc.b_stat_write=1;
                             param.rc.b_stat_read=0;
                             param.rc.psz_stat_out=logFile;
#warning FIXME
#if 0
                             if(x265Settings.)
                                    x265_param_apply_fastfirstpass(&_param);
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
      case COMPRESS_AQ: param.rc.i_rc_method = X265_RC_CRF;
                        param.rc.f_rf_constant = x265Settings.general.params.qz;
                        break;
      case COMPRESS_CQ: param.rc.i_rc_method = X265_RC_CQP;
                        param.rc.i_qp_constant = x265Settings.general.params.qz;
                        break;

      case COMPRESS_CBR:
                        param.rc.i_rc_method = X265_RC_ABR;
                        param.rc.i_bitrate =  x265Settings.general.params.bitrate;
                        param.rc.i_qp_constant = 0;
                        param.rc.f_rf_constant = 0;
                        break;
        default:
                        GUI_Error_HIG("Not coded","this mode has notbeen implemented\n");
                        return false;
                        break;

  }

  if(globalHeader)
      param.b_repeat_headers=0;
  else
      param.b_repeat_headers=1;

  // We do pseudo cfr ...
  param.b_vfr_input=0;

  if(x265Settings.useAdvancedConfiguration)
  {  

  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.x = x265Settings.y;aprintf("[x265] "#x" = %d\n",param.x);}
  #define MKPARAMF(x,y) {param.x = (float)x265Settings.y; aprintf("[x265] "#x" = %.2f\n",param.x);}
  #define MKPARAMB(x,y) {param.x = x265Settings.y ;aprintf("[x265] "#x" = %s\n",TrueFalse[param.x&1]);}
    MKPARAM(i_frame_reference,MaxRefFrames);
    MKPARAM(i_keyint_min,MinIdr);
    MKPARAM(i_keyint_max,MaxIdr);
    MKPARAM(i_scenecut_threshold,i_scenecut_threshold);
    MKPARAMB(b_intra_refresh,intra_refresh);
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
    MKPARAMB(b_interlaced,interlaced);
    MKPARAMB(b_constrained_intra,constrained_intra);
    MKPARAMB(b_tff,tff);
    MKPARAMB(b_fake_interlaced,fake_interlaced);

    // -------------- analyze------------
  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.analyse.x = x265Settings.analyze.y;aprintf("[x265] analyse."#x" = %d\n",param.analyse.x);}
  #define MKPARAMF(x,y) {param.analyse.x = (float)x265Settings.analyze.y; aprintf("[x265] analyse."#x" = %.2f\n",param.analyse.x);}
  #define MKPARAMB(x,y) {param.analyse.x = x265Settings.analyze.y ;aprintf("[x265] analyse."#x" = %s\n",TrueFalse[param.analyse.x&1]);}
  #define MKFLAGS(fieldout,fieldin,mask) {if(x265Settings.analyze.fieldin) param.analyse.fieldout|=mask;}
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

     MKFLAGS(inter,b_i4x4,X265_ANALYSE_I4x4)
     MKFLAGS(inter,b_i8x8,X265_ANALYSE_I8x8)
     MKFLAGS(inter,b_p16x16,X265_ANALYSE_PSUB16x16)
     MKFLAGS(inter,b_p8x8,X265_ANALYSE_PSUB8x8)
     MKFLAGS(inter,b_b16x16,X265_ANALYSE_BSUB16x16)

     //---------------- ratecontrol -------------------
  #undef MKPARAM
  #undef MKPARAMF
  #undef MKPARAMB
  #define MKPARAM(x,y)  {param.rc.x = x265Settings.ratecontrol.y;aprintf("[x265] rc."#x" = %d\n",param.rc.x);}
  #define MKPARAMF(x,y) {param.rc.x = (float)x265Settings.ratecontrol.y; aprintf("[x265] rc."#x" = %.2f\n",param.rc.x);}
  #define MKPARAMB(x,y) {param.rc.x = x265Settings.ratecontrol.y ;aprintf("[x265] rc."#x" = %s\n",TrueFalse[param.rc.x&1]);}

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
              encoderDelay=2*f*(x265Settings.MaxRefFrames-1);
      }
  }

 //
  if(true==x265Settings.general.fast_first_pass)
  {
    if(passNumber==1)
    {
        switch(x265Settings.general.params.mode)
        {
            case COMPRESS_2PASS:
            case COMPRESS_2PASS_BITRATE:
                     ADM_info("Appling fast first pass settings\n");
                     x265_param_apply_fastfirstpass(&param);
                    break;
            default:
                    break;
        }
    }
  }

  if(!x265Settings.useAdvancedConfiguration)
  {
    x265_param_apply_profile(&param, x265Settings.general.profile);
  }

  dumpx265Setup(&param);
  ADM_info("Creating x265 encoder\n");
  handle = x265_encoder_open (&param);
  if (!handle)
  {
    ADM_error("Cannot initialize x265\n");
    return 0;
  }

  
  ADM_info("x265, setup ok\n");
  if (globalHeader)
  {
      ADM_info("Creating global header\n");
      return createHeader ();
  }else
      ADM_info("No need for global header\n");
    
  return true;
}
/**
    \fn dumpx265Setup
*/
void dumpx265Setup(x265_param *param)
{
#define PI(x) printf(#x"\t:%d\n",(int)param->x);
    PI(cpu);
    PI(i_threads);
    PI(b_sliced_threads);
    PI(b_deterministic);
    PI(i_sync_lookahead);

    PI(i_width); 
    PI(i_height); 
    PI(i_width); 
    PI(i_level_idc); 
    PI(i_frame_total);

#define VI(x) printf(#x"\t:%d\n",(int)param->vui.x);

    VI(i_sar_height);
    VI(i_sar_width);
    VI(i_overscan);
    VI(i_vidformat);
    VI(b_fullrange);
    VI(i_colorprim);
    VI(i_transfer);
    VI(i_colmatrix);
    VI(i_chroma_loc);

    PI(i_fps_num);
    PI(i_fps_den);

    PI(i_timebase_num);
    PI(i_timebase_den);

        
    PI(i_frame_reference);
    PI(i_keyint_max);
    PI(i_keyint_min);
    PI(i_scenecut_threshold);
    PI(b_intra_refresh);

    PI(i_bframe);
    PI(i_bframe_adaptive);
    PI(i_bframe_bias);
    PI(i_bframe_pyramid);

    PI(b_deblocking_filter);
    PI(i_deblocking_filter_alphac0);
    PI(i_deblocking_filter_beta);

    PI(b_cabac);
    PI(i_cabac_init_idc);

    PI(b_interlaced);
    PI(b_tff);
    PI(b_fake_interlaced);
    PI(b_constrained_intra);

#define AI(x) printf(#x"\t:%d\n",(int)param->analyse.x);
#define AF(x) printf(#x"\t:%f\n",(float)param->analyse.x);
    printf("*************************************\n");
    printf("*********     Analyse       *********\n");
    printf("*************************************\n");

    AI(intra);
    AI(inter);

    AI(b_transform_8x8);
    AI(i_weighted_pred);
    AI(b_weighted_bipred);
    AI(i_chroma_qp_offset);
    
    AI(i_me_method);
    AI(i_me_range);
    AI(i_mv_range);
    AI(i_mv_range_thread);
    AI(i_subpel_refine);
    AI(b_chroma_me);
    AI(b_mixed_references);
    AI(i_trellis);
    AI(b_fast_pskip);

    AI(b_dct_decimate);
    AI(i_noise_reduction);
    AF(f_psy_rd);
    AF(f_psy_trellis);
    AI(b_psy);

    PI(b_aud);
    PI(b_repeat_headers);
    PI(b_annexb);
    PI(b_vfr_input);
    
    AI(i_luma_deadzone[0]);
    AI(i_luma_deadzone[1]);

    PI(i_sps_id);

    PI(i_slice_max_size);
    PI(i_slice_max_mbs);
    PI(i_slice_count);

#define RI(x) printf(#x"\t:%d\n",(int)param->rc.x)
#define RF(x) printf(#x"\t:%f\n",(float)param->rc.x)
    printf("*************************************\n");
    printf("*********     RC            *********\n");
    printf("*************************************\n");
    RI(i_rc_method);
    RI(i_qp_constant);
    RF(f_rf_constant);
    RI(i_qp_min);
    RI(i_qp_max);
    RI(i_qp_step);

   
    RI(i_bitrate);
    RI(i_qp_constant);
    RF(f_rate_tolerance);
    RI(i_vbv_max_bitrate);
    RI(i_vbv_buffer_size);
    RF(f_vbv_buffer_init);
    RF(f_ip_factor);
    RF(f_pb_factor);

    RI(i_aq_mode);
    RF(f_aq_strength);
    RI(b_mb_tree);
    RI(i_lookahead);

}
/**
 * \fn x265LoadProfile
 * @param profile
 */
extern bool  x265_encoder_jserialize(const char *file, const x265_encoder *key);
extern bool  x265_encoder_jdeserialize(const char *file, const ADM_paramList *tmpl,x265_encoder *key);
extern "C" 
{
extern const ADM_paramList x265_encoder_param[];
}

bool x265LoadProfile(const char *profile)
{
    x265_encoder param=x265Settings;
    std::string rootPath;
    ADM_pluginGetPath("x265",1,rootPath);
    std::string fullPath=rootPath+std::string("/")+profile+std::string(".json");
    ADM_info("Trying to load %s\n",fullPath.c_str());
    if(false==x265_encoder_jdeserialize(fullPath.c_str(),x265_encoder_param,&param))
    {
        ADM_warning("Failed\n");
        return false;     
    }
    ADM_info("Profile loaded ok\n");
    x265Settings=param;
    return true;
}

// EOF



