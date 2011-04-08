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
extern bool ADM_computeAverageBitrateFromDuration(uint64_t duration, uint32_t sizeInMB, uint32_t *avgInKbits);;
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


  // Threads..
  switch(x264Settings.general.threads)
  {
    case 0: case 1: case 2:  param.i_threads = x264Settings.general.threads;break;
    case 99:break; //auto
    default: ADM_error("UNKNOWN NB OF THREADS\n");break;
  }
  param.i_width = getWidth();
  param.i_height = getHeight();
  param.i_csp = X264_CSP_I420;
  param.i_log_level=X264_LOG_INFO; //DEBUG; //INFO;
  param.i_level_idc=x264Settings.level; 
    //Framerate
    int n,d;    
    uint64_t f=source->getInfo()->frameIncrement;
    usSecondsToFrac(f,&n,&d);
    param.i_fps_num = d;
    param.i_fps_den = n;
    if(!x264Settings.MaxBFrame)  encoderDelay=0;
    else    
    {
        if(2>=x264Settings.MaxRefFrames) encoderDelay=f*2*2;
        else
                encoderDelay=2*f*(x264Settings.MaxRefFrames-1);
    }
#define MKPARAM(x,y) {param.x = x264Settings.y;aprintf("[x264] "#x" = %d\n",param.x);}
#define MKPARAMF(x,y) {param.x = (float)x264Settings.y / 100; aprintf("[x264] "#x" = %.2f\n",param.x);}
#define MKPARAMB(x,y) {param.x = x264Settings.y ;aprintf("[x264] "#x" = %s\n",TrueFalse[param.x&1]);}
  MKPARAM(i_frame_reference,MaxRefFrames);
  MKPARAM(i_keyint_min,MinIdr);
  MKPARAM(i_keyint_max,MaxIdr);
  MKPARAM(i_bframe,MaxBFrame);

  MKPARAM(i_bframe_adaptive,i_bframe_adaptative);
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
  // -------------- analyze------------
#undef MKPARAM
#undef MKPARAMF
#undef MKPARAMB
#define MKPARAM(x,y) {param.analyse.x = x264Settings.analyze.y;aprintf("[x264] analyse."#x" = %d\n",param.analyse.x);}
#define MKPARAMF(x,y) {param.analyse.x = (float)x264Settings.analyze.y / 100; aprintf("[x264] analyse."#x" = %.2f\n",param.analyse.x);}
#define MKPARAMB(x,y) {param.analyse.x = x264Settings.analyze.y ;aprintf("[x264] analyse."#x" = %s\n",TrueFalse[param.analyse.x&1]);}
#define MKFLAGS(fieldout,fieldin,mask) {if(x264Settings.analyze.fieldin) param.analyse.fieldout|=mask;}
   MKPARAMB(b_transform_8x8,b_8x8)
   MKPARAMB(b_weighted_bipred,weighted_bipred) 
   MKPARAM (i_weighted_pred,weighted_pred) 
   MKPARAM (i_direct_mv_pred,direct_mv_pred) 
   MKPARAM (i_me_method,me_method) 
   MKPARAM (i_subpel_refine,subpel_refine) 
   MKPARAMB(b_chroma_me,chroma_me) 
   MKPARAMB(b_mixed_references,mixed_references) 
   MKPARAM (i_trellis,trellis) 
   MKPARAMB(b_fast_pskip,fast_pskip) 
   MKPARAMB(b_dct_decimate,dct_decimate) 
   MKPARAMB(b_psy,psy) 

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
#define MKPARAMF(x,y) {param.rc.x = (float)x264Settings.ratecontrol.y / 100; aprintf("[x264] rc."#x" = %.2f\n",param.rc.x);}
#define MKPARAMB(x,y) {param.rc.x = x264Settings.ratecontrol.y ;aprintf("[x264] rc."#x" = %s\n",TrueFalse[param.rc.x&1]);}

    MKPARAMB(b_mb_tree,mb_tree);
    MKPARAM(i_lookahead,lookahead);
  // -------------------------
  

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
#warning FIXME
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
    return createHeader ();
  return true;
}
/**
    \fn dumpx264Setup
*/
void dumpx264Setup(x264_param_t *param)
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
    PI(b_constrained_intra);

#define AI(x) printf(#x"\t:%d\n",(int)param->analyse.x);
    printf("*************************************\n");
    printf("*********     Analyse       *********\n");
    printf("*************************************\n");

    AI(intra);
    AI(inter);

    AI(b_transform_8x8);
    AI(i_weighted_pred);
    AI(b_weighted_bipred);
    AI(i_weighted_pred);
    AI(i_chroma_qp_offset);
    
    AI(i_me_method);
    AI(i_me_range);
    AI(i_mv_range_thread);
    AI(i_subpel_refine);
    AI(b_chroma_me);
    AI(b_mixed_references);
    AI(i_trellis);
    AI(b_fast_pskip);

    AI(b_dct_decimate);
    AI(i_noise_reduction);
    AI(f_psy_rd);
    AI(f_psy_trellis);
    AI(b_psy);

    PI(b_aud);
    PI(b_repeat_headers);
    PI(b_annexb);
    PI(b_vfr_input);
    

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
// EOF


