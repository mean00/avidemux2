/***************************************************************************
                          \fn ADM_x264
                          \brief Front end for x264 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2002/2009 by mean/gruntster
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
#include "ADM_x264.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#if 1
#define aprintf(...) {}
#define avsnprintf(...) {}
#else
#define aprintf printf
#define avsnprintf vsnprintf
#endif
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
  ADM_info("x264, setting up");
  
  firstIdr=true;
  image=new ADMImageDefault(getWidth(),getHeight());
  MMSET(param);
  x264_param_default( &param);
  param.pf_log=logger;
  switch(x264Settings.threads)
  {
    case 0: case 1: case 2:  param.i_threads = x264Settings.threads;break;
    case 99:break; //auto
    default: ADM_error("UNKNOWN NB OF THREADS\n");break;
  }
  param.i_width = getWidth();
  param.i_height = getHeight();
  param.i_csp = X264_CSP_I420;
 
    //Framerate
    int n,d;    
    uint64_t f=source->getInfo()->frameIncrement;
    usSecondsToFrac(f,&n,&d);
    param.i_fps_num = n;
    param.i_fps_den = d;
    if(!x264Settings.MaxBFrame)  encoderDelay=0;
    else    
    {
        if(2>=x264Settings.MaxRefFrames) encoderDelay=f*2*2;
        else
                encoderDelay=2*f*(x264Settings.MaxRefFrames-1);
    }
#define MKPARAM(x,y) {param.x = x264Settings.y;printf("[x264] "#x" = %d\n",param.x);}
#define MKPARAMF(x,y) {param.x = (float)x264Settings.y / 100; printf("[x264] "#x" = %.2f\n",param.x);}

#if 0
  if (zparam->AR_AsInput) {
    param.vui.i_sar_width = video_body->getPARWidth();
    param.vui.i_sar_height = video_body->getPARHeight();
  } else {
    MKPARAM(vui.i_sar_width , AR_Num);	// FIXME
    MKPARAM(vui.i_sar_height, AR_Den);
  }
  if(zparam->idc)
  {
    MKPARAM(i_level_idc,idc);
    printf("[x264] *** Forcing level = %d\n",param.i_level_idc);
  }
  // KeyframeBoost ?
  // BframeReduction ?
  // PartitionDecision ?
  MKPARAMF(rc.f_qcompress,BitrateVariability);

  // update for Sadarax dialog
  MKPARAM(rc.i_vbv_max_bitrate,vbv_max_bitrate);
  MKPARAM(rc.i_vbv_buffer_size,vbv_buffer_size);
  MKPARAMF(rc.f_vbv_buffer_init,vbv_buffer_init);
  
  MKPARAM (analyse.b_fast_pskip,fastPSkip);
  MKPARAM (analyse.b_dct_decimate,DCTDecimate);
  MKPARAM (b_interlaced,interlaced);
      
  //
  MKPARAM(analyse.i_direct_mv_pred,DirectMode+1);
  MKPARAM(rc.i_qp_min,MinQp);
  MKPARAM(rc.i_qp_max,MaxQp);
  MKPARAM(rc.i_qp_step,QpStep);
  MKPARAM(i_scenecut_threshold,SceneCut);
  MKPARAM(i_bframe_bias,Bias);
  MKPARAM( b_bframe_pyramid,BasReference );
  MKPARAM(analyse. b_bidir_me,BidirME );
  MKPARAM( b_bframe_adaptive, Adaptative);
  MKPARAM( analyse.b_weighted_bipred, Weighted);
  MKPARAM(analyse.i_subpel_refine,PartitionDecision+1);
#endif
  MKPARAM(i_frame_reference,MaxRefFrames);
  
  MKPARAM(i_keyint_min,MinIdr);
  MKPARAM(i_keyint_max,MaxIdr);
  MKPARAM(i_bframe,MaxBFrame);

  MKPARAM( b_cabac , CABAC);
  MKPARAM( analyse.i_trellis, Trellis);

#if 0  
#define MIN_RDO 6
  if(zparam->PartitionDecision+1>=MIN_RDO)
  {
      int rank,parity;
      rank=((zparam->PartitionDecision+1-MIN_RDO)>>1)+MIN_RDO;
      parity=(zparam->PartitionDecision+1-MIN_RDO)&1;
      
      param.analyse.i_subpel_refine=rank;
      param.analyse.b_bframe_rdo=parity;
  }
  MKPARAM(analyse.b_chroma_me,ChromaME);
  MKPARAM(b_deblocking_filter,DeblockingFilter);
  MKPARAM(i_deblocking_filter_alphac0,Strength );
  MKPARAM(i_deblocking_filter_beta, Threshold);
  
  MKPARAM(analyse.i_me_method,Method);
  MKPARAM(analyse.i_me_range,Range);
  MKPARAM(i_bframe_bias,Bias);
  MKPARAM( b_bframe_pyramid,BasReference );
  MKPARAM(analyse. b_bidir_me,BidirME );
  MKPARAM( b_bframe_adaptive, Adaptative);
  MKPARAM( analyse.b_weighted_bipred, Weighted);
#endif
//  MKPARAM(PartitionDecision,Method);
  MKPARAM(analyse.b_transform_8x8,_8x8);
  
#define MES(x,y) if(x264Settings.x) {param.analyse.inter |=X264_ANALYSE_##y;printf("[x264] "#x" is on\n");}
  param.analyse.inter=0;
  MES(  _8x8P,  PSUB16x16);
  MES(  _8x8B,  BSUB16x16);
  MES(  _4x4,   PSUB8x8);
  MES(  _8x8I,  I8x8);
  MES(  _4x4I,  I4x4);


  param.i_log_level=X264_LOG_INFO; //INFO;
  
  
  switch(x264Settings.params.mode)
  {
      
      case COMPRESS_AQ: param.rc.i_rc_method = X264_RC_CRF;
                        param.rc.f_rf_constant = x264Settings.params.qz;
                        break;
      case COMPRESS_CQ: param.rc.i_rc_method = X264_RC_CQP;
                        param.rc.i_qp_constant = x264Settings.params.qz;
                        break;

      case COMPRESS_CBR:
                        param.rc.i_rc_method = X264_RC_ABR;
                        param.rc.i_bitrate =  x264Settings.params.bitrate*1000;
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
  dumpx264Setup(&param);
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

