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
#define MMSET(x) memset(&(x),0,sizeof(x))

extern x265_settings x265Settings;
/**
    \fn setup
*/
bool x265Encoder::setup(void)
{
  ADM_info("=============x265, setting up==============\n");
  MMSET(param);

  x265_param_default( &param);
  firstIdr=true;
  image=new ADMImageDefault(getWidth(),getHeight());

  // -------------- preset, tune, idc ------------
  if(!x265Settings.useAdvancedConfiguration)
  {
    char tune[200] = {0};
    strcat(tune, x265Settings.general.tuning);
    x265_param_default_preset(&param, x265Settings.general.preset, tune);
  }
  param.logLevel=x265Settings.level; 

  // Threads..
  switch(x265Settings.general.threads)
  {
    case 0: case 1: case 2:  param.poolNumThreads = x265Settings.general.threads;break;
    case 99:break; //auto
    default: ADM_error("UNKNOWN NB OF THREADS\n");break;
  }
  param.sourceWidth = getWidth();
  param.sourceHeight = getHeight();
  param.internalCsp = X265_CSP_I420;
  param.internalBitDepth = 8;
  param.logLevel=X265_LOG_INFO; //DEBUG; //INFO;

  //Framerate
  int n,d;    
  uint64_t f=source->getInfo()->frameIncrement;
  usSecondsToFrac(f,&n,&d);
  param.fpsNum = d;
  param.fpsDenom = n;

  // -------------- vui------------
  #undef MKPARAM
  #undef MKPARAMD
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.vui.x = x265Settings.vui.y;aprintf("[x265] vui."#x" = %d\n",param.vui.x);}
  #define MKPARAMD(x,y) {param.vui.x = (double)x265Settings.vui.y; aprintf("[x265] vui."#x" = %.2f\n",param.vui.x);}
  #define MKPARAMB(x,y) {param.vui.x = x265Settings.vui.y ;aprintf("[x265] vui."#x" = %s\n",TrueFalse[param.vui.x&1]);}
  MKPARAM (sarWidth,sar_width) 
  MKPARAM (sarHeight,sar_height) 

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
                        param.rc.rateControlMode = X265_RC_ABR;
                        param.rc.bitrate =  bitrate;
                        if(passNumber==1)
                        {
                             param.rc.bStatWrite=1;
                             param.rc.bStatRead=0;
                             param.rc.statFileName=logFile;
 
                        }else
                        {
                             param.rc.bStatWrite=0;
                             param.rc.bStatRead=1;
                             param.rc.statFileName=logFile;
                             if(!ADM_fileExist(logFile))
                             {
                                   ADM_error("Logfile %s does not exist \n",logFile);
                                   return false;
                             }
                        }
                        break;
      case COMPRESS_AQ: param.rc.rateControlMode = X265_RC_CRF;
                        param.rc.rfConstant = x265Settings.general.params.qz;
                        break;
      case COMPRESS_CQ: param.rc.rateControlMode = X265_RC_CQP;
                        param.rc.qp = x265Settings.general.params.qz;
                        break;

      case COMPRESS_CBR:
                        param.rc.rateControlMode = X265_RC_ABR;
                        param.rc.bitrate =  x265Settings.general.params.bitrate;
                        param.rc.qp = 0;
                        param.rc.rfConstant = 0;
                        break;
        default:
                        GUI_Error_HIG("Not coded","this mode has notbeen implemented\n");
                        return false;
                        break;

  }

  if(globalHeader)
      param.bRepeatHeaders=0;
  else
      param.bRepeatHeaders=1;

  if(x265Settings.useAdvancedConfiguration)
  {  

  #undef MKPARAM
  #undef MKPARAMD
  #undef MKPARAMB
  #define MKPARAM(x,y) {param.x = x265Settings.y;aprintf("[x265] "#x" = %d\n",param.x);}
  #define MKPARAMD(x,y) {param.x = (double)x265Settings.y; aprintf("[x265] "#x" = %.2f\n",param.x);}
  #define MKPARAMB(x,y) {param.x = x265Settings.y ;aprintf("[x265] "#x" = %s\n",TrueFalse[param.x&1]);}
    MKPARAM(maxNumReferences,MaxRefFrames);
    MKPARAM(keyframeMin,MinIdr);
    MKPARAM(keyframeMax,MaxIdr);
    MKPARAM(scenecutThreshold,i_scenecut_threshold);
    MKPARAM(bframes,MaxBFrame);

    MKPARAM(bFrameAdaptive,i_bframe_adaptive);
    MKPARAM(bFrameBias,i_bframe_bias);
    MKPARAM(bBPyramid,i_bframe_pyramid);
    MKPARAMB(bEnableLoopFilter,b_deblocking_filter);
    MKPARAMB(interlaceMode,interlaced_mode);
    MKPARAMB(bEnableConstrainedIntra,constrained_intra);
    MKPARAM(lookaheadDepth,lookahead);

    MKPARAMB(bEnableWeightedBiPred,weighted_bipred) 
    MKPARAM (bEnableWeightedPred,weighted_pred) 
    MKPARAM (cbQpOffset,cb_chroma_offset)
    MKPARAM (crQpOffset,cr_chroma_offset)

    MKPARAM (searchMethod,me_method) 
    MKPARAM (searchRange,me_range)
    MKPARAM (subpelRefine,subpel_refine) 
    MKPARAM (bFrameAdaptive,trellis) 
    MKPARAMB(bEnableEarlySkip,fast_pskip) 
    MKPARAMB(bEnableTSkipFast,dct_decimate) 
    MKPARAMD(psyRd,psy_rd)
    MKPARAM (noiseReduction,noise_reduction)

     //---------------- ratecontrol -------------------
  #undef MKPARAM
  #undef MKPARAMD
  #undef MKPARAMB
  #define MKPARAM(x,y)  {param.rc.x = x265Settings.ratecontrol.y;aprintf("[x265] rc."#x" = %d\n",param.rc.x);}
  #define MKPARAMD(x,y) {param.rc.x = (double)x265Settings.ratecontrol.y; aprintf("[x265] rc."#x" = %.2f\n",param.rc.x);}
  #define MKPARAMB(x,y) {param.rc.x = x265Settings.ratecontrol.y ;aprintf("[x265] rc."#x" = %s\n",TrueFalse[param.rc.x&1]);}

      MKPARAM(qpStep,qp_step);
      MKPARAM(rateTolerance,rate_tolerance);
      MKPARAM(ipFactor,ip_factor);
      MKPARAM(pbFactor,pb_factor);
      MKPARAMB(cuTree,cu_tree);
      MKPARAM(aqMode,aq_mode);
      MKPARAMD(aqStrength,aq_strength);
  }
  
  if(!param.bframes)  encoderDelay=0;
  else    
  {
      if(2>=param.maxNumReferences) 
      {
          encoderDelay=f*2*2;
      }
      else
      {
              encoderDelay=2*f*(x265Settings.MaxRefFrames-1);
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
#define PI(x) printf(#x"\t:%d\n",(int)param->x)
#define PD(x) printf(#x"\t:%d\n",(double)param->x)
    printf("*************************************\n");
    printf("***      Encoder Environment      ***\n");
    printf("*************************************\n");
    
    PI(cpuid);
    PI(bEnableWavefront);
    PI(poolNumThreads);
    PI(frameNumThreads);
    PI(logLevel);
    PI(bLogCuStats); 
    PI(bEnablePsnr); 
    PI(bEnableSsim); 
    PI(decodedPictureHashSEI);
    
    printf("*************************************\n");
    printf("** Internal Picture Specification  **\n");
    printf("*************************************\n");
    
    PI(internalBitDepth);
    PI(internalCsp);
    PI(fpsNum);
    PI(fpsDenom);
    PI(sourceWidth);
    PI(sourceHeight);
    PI(levelIdc);
    PI(interlaceMode);
    PI(bRepeatHeaders);
    PI(bEnableAccessUnitDelimiters);
    PI(bEmitHRDSEI);
    
    printf("*************************************\n");
    printf("*** Coding Unit (CU) Definitions  ***\n");
    printf("*************************************\n");
    
    PI(maxCUSize);
    PI(tuQTMaxInterDepth);
    PI(tuQTMaxIntraDepth);
    
    printf("*************************************\n");
    printf("***  GOP Structure and Lookahead  ***\n");
    printf("*************************************\n");
    
    PI(bOpenGOP);
    PI(keyframeMin);
    PI(keyframeMax);
    PI(maxNumReferences);
    PI(bFrameAdaptive);
    PI(bframes);
    PI(bBPyramid);
    PI(lookaheadDepth);
    PI(bFrameBias);
    PI(scenecutThreshold);
    
    printf("*************************************\n");
    printf("***      Intra Coding Tools       ***\n");
    printf("*************************************\n");
    
    PI(bEnableConstrainedIntra);
    PI(bEnableStrongIntraSmoothing);
    
    printf("*************************************\n");
    printf("***      Inter Coding Tools       ***\n");
    printf("*************************************\n");
    
    PI(searchMethod);
    PI(subpelRefine);
    PI(searchRange);
    PI(maxNumMergeCand);
    PI(bEnableWeightedPred);
    PI(bEnableWeightedBiPred);
    
    printf("*************************************\n");
    printf("***        Analysis Tools         ***\n");
    printf("*************************************\n");
    
    PI(bEnableAMP);
    PI(bEnableRectInter);
    PI(bEnableCbfFastMode);
    PI(bEnableEarlySkip);
    PI(rdPenalty);
    PI(rdLevel);
    PD(psyRd);
    
    printf("*************************************\n");
    printf("***         Coding Tools          ***\n");
    printf("*************************************\n");
    
    PI(bEnableSignHiding);
    PI(bEnableTransformSkip);
    PI(bEnableTSkipFast);
    PI(bEnableLoopFilter);
    PI(bEnableSAO);
    PI(saoLcuBoundary);
    PI(saoLcuBasedOptimization);
    PI(cbQpOffset);
    PI(crQpOffset);
    PI(bIntraInBFrames);
    PI(noiseReduction);
    PI(bLossless);
    PI(bCULossless);

#define RI(x) printf(#x"\t:%d\n",(int)param->rc.x)
#define RD(x) printf(#x"\t:%f\n",(double)param->rc.x)
    printf("*************************************\n");
    printf("***         Rate Control          ***\n");
    printf("*************************************\n");
    
    RI(rateControlMode);
    RI(qp);
    RI(bitrate);
    RD(rateTolerance);
    RD(qCompress);
    RD(ipFactor);
    RD(pbFactor);
    RI(qpStep);
    RD(rfConstant);
    RI(aqMode);
    RD(aqStrength);
    RI(vbvMaxBitrate);
    RI(vbvBufferSize);
    RD(vbvBufferInit);
    RI(cuTree);
    RD(rfConstantMax);
    RD(rfConstantMin);
    RI(bStatWrite);
    RI(bStatRead);
    RD(qblur);
    RD(complexityBlur);

#define VI(x) printf(#x"\t:%d\n",(int)param->vui.x)
    printf("*************************************\n");
    printf("***  Video Usability Information  ***\n");
    printf("*************************************\n");
    
    VI(aspectRatioIdc);
    VI(sarWidth);
    VI(sarHeight);
    VI(bEnableOverscanInfoPresentFlag);
    VI(bEnableOverscanAppropriateFlag);
    VI(bEnableVideoSignalTypePresentFlag);
    VI(videoFormat);
    VI(bEnableVideoFullRangeFlag);
    VI(bEnableColorDescriptionPresentFlag);
    VI(colorPrimaries);
    VI(transferCharacteristics);
    VI(matrixCoeffs);
    VI(bEnableChromaLocInfoPresentFlag);
    VI(chromaSampleLocTypeTopField);
    VI(chromaSampleLocTypeBottomField);
    VI(bEnableDefaultDisplayWindowFlag);
    VI(defDispWinLeftOffset);
    VI(defDispWinRightOffset);
    VI(defDispWinTopOffset);
    VI(defDispWinBottomOffset);

}
/**
 * \fn x265LoadProfile
 * @param profile
 */
extern bool  x265_settings_jserialize(const char *file, const x265_settings *key);
extern bool  x265_settings_jdeserialize(const char *file, const ADM_paramList *tmpl,x265_settings *key);
extern "C" 
{
extern const ADM_paramList x265_settings_param[];
}

bool x265LoadProfile(const char *profile)
{
    x265_settings param=x265Settings;
    std::string rootPath;
    ADM_pluginGetPath("x265",1,rootPath);
    std::string fullPath=rootPath+std::string("/")+profile+std::string(".json");
    ADM_info("Trying to load %s\n",fullPath.c_str());
    if(false==x265_settings_jdeserialize(fullPath.c_str(),x265_settings_param,&param))
    {
        ADM_warning("Failed\n");
        return false;     
    }
    ADM_info("Profile loaded ok\n");
    x265Settings=param;
    return true;
}

// EOF



