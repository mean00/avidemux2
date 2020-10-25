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

#define aprintf(...) {}
#define avsnprintf(...) {}
static const char *TrueFalse[2]={"False","True"};
static void dumpx265Setup(x265_param *param);
#define MMSET(x) memset(&(x),0,sizeof(x))

extern x265_settings x265Settings;

/**
    \fn x265ProbeBitDepth
*/
bool x265ProbeBitDepth(int depth)
{
    static uint32_t cachedQueries=0;

    uint32_t already=1;
    uint32_t match=0;

#define BIT_DEPTH_8_BITS 1
#define BIT_DEPTH_10_BITS 2
#define BIT_DEPTH_12_BITS 4

    switch(depth)
    {
        case 0:
            return true;
        case 8:
            match = BIT_DEPTH_8_BITS;
            break;
        case 10:
            match = BIT_DEPTH_10_BITS;
            break;
        case 12:
            match = BIT_DEPTH_12_BITS;
            break;
        default:
            return false;
    }
    already <<= depth;
    if(cachedQueries & already)
    {
        if((cachedQueries & BIT_DEPTH_8_BITS) && depth == 8)
            return true;
        if((cachedQueries & BIT_DEPTH_10_BITS) && depth == 10)
            return true;
        if((cachedQueries & BIT_DEPTH_12_BITS) && depth == 12)
            return true;
        return false;
    }
    cachedQueries |= already;
    if(x265_api_get(depth))
    {
        cachedQueries |= match;
        return true;
    }
    return false;
}

/**
    \fn setup
*/
bool x265Encoder::setup(void)
{
  ADM_info("=============x265, setting up==============\n");
  MMSET(param);

  if(x265Settings.useAdvancedConfiguration)
  {
        api = x265_api_get(x265Settings.general.output_bit_depth);
  }else if(x265Settings.general.profile == "main10")
  {
        api = x265_api_get(10);
  }else
  {
        api = x265_api_get(8);
  }
  if(!api)
        api = x265_api_get(0);
  ADM_assert(api);

  api->param_default(&param);
  firstIdr=true;
  image=new ADMImageDefault(getWidth(),getHeight());

  // -------------- preset, tune, idc ------------
  if(!x265Settings.useAdvancedConfiguration)
  {
    if(x265Settings.general.tuning == "none")
    {
        api->param_default_preset(&param, x265Settings.general.preset.c_str(), NULL);
    }else
    {
        api->param_default_preset(&param, x265Settings.general.preset.c_str(), x265Settings.general.tuning.c_str());
    }
  }
  param.logLevel=x265Settings.level; 

  // Threads..
#if X265_BUILD < 47
  switch(x265Settings.general.poolThreads)
  {
    case 1: case 2: case 4: param.poolNumThreads = x265Settings.general.poolThreads;break;
    case 0: case 99: break; //auto
    default: ADM_error("UNKNOWN NB OF THREADS\n");break;
  }
#endif

  switch(x265Settings.general.frameThreads)
  {
    case 1: case 2: case 4: param.frameNumThreads = x265Settings.general.frameThreads;break;
    case 0: case 99: break; //auto
    default: ADM_error("UNKNOWN NB OF THREADS\n");break;
  }
  param.sourceWidth = getWidth();
  param.sourceHeight = getHeight();
  param.internalCsp = X265_CSP_I420;
  param.internalBitDepth = api->bit_depth;
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

    MKPARAM(aspectRatioIdc,sar_idc);
    MKPARAM(sarWidth,sar_width);
    MKPARAM(sarHeight,sar_height);

    if(x265Settings.vui.color_primaries != 2 || x265Settings.vui.matrix_coeffs != 2 || x265Settings.vui.transfer_characteristics != 2)
    {
	param.vui.bEnableVideoSignalTypePresentFlag = true;
	param.vui.bEnableColorDescriptionPresentFlag = true;
	MKPARAM(colorPrimaries,color_primaries);
	MKPARAM(matrixCoeffs,matrix_coeffs);
	MKPARAM(transferCharacteristics,transfer_characteristics);
    }

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
                             param.rc.statFileName=strdup(logFile);
 
                        }else
                        {
                             param.rc.bStatWrite=0;
                             param.rc.bStatRead=1;
                             param.rc.statFileName=strdup(logFile);
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
                        GUI_Error_HIG(QT_TRANSLATE_NOOP("x265","Not coded"),QT_TRANSLATE_NOOP("x265","this mode has not been implemented\n"));
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
    MKPARAMB(interlaceMode,interlaced_mode)
    MKPARAM(maxNumReferences,MaxRefFrames)
    MKPARAMB(bOpenGOP,b_open_gop)
    MKPARAM(keyframeMin,MinIdr)
    MKPARAM(keyframeMax,MaxIdr)

    MKPARAM(bframes,MaxBFrame)
    MKPARAM(bFrameAdaptive,i_bframe_adaptive)
    MKPARAM(bBPyramid,i_bframe_pyramid)
    MKPARAM(bFrameBias,i_bframe_bias)

    MKPARAM(lookaheadDepth,lookahead)
    MKPARAM(scenecutThreshold,i_scenecut_threshold)
    MKPARAMB(bEnableConstrainedIntra,constrained_intra)
    MKPARAMB(bIntraInBFrames,b_intra)

    MKPARAM(searchMethod,me_method)
    MKPARAM(subpelRefine,subpel_refine)
    MKPARAM(searchRange,me_range)
    MKPARAM(limitReferences,limit_refs)

    MKPARAM(bEnableWeightedPred,weighted_pred)
    MKPARAMB(bEnableWeightedBiPred,weighted_bipred)
    MKPARAMB(bEnableRectInter,rect_inter)
    MKPARAMB(bEnableAMP,amp_inter)
    MKPARAMB(limitModes,limit_modes)

    MKPARAMB(bEnableLoopFilter,b_deblocking_filter)
    MKPARAMB(bEnableEarlySkip,fast_pskip)
    MKPARAMB(bEnableTSkipFast,dct_decimate)

    MKPARAM (rdLevel,rd_level)
    MKPARAMD(psyRd,psy_rd)
    MKPARAM (rdoqLevel,rdoq_level)
    MKPARAMD(psyRdoq,psy_rdoq)
    MKPARAM (cbQpOffset,cb_chroma_offset)
    MKPARAM (crQpOffset,cr_chroma_offset)

    MKPARAM (noiseReductionIntra,noise_reduction_intra)
    MKPARAM (noiseReductionInter,noise_reduction_inter)

    MKPARAMB(bEnableStrongIntraSmoothing,strong_intra_smoothing)

     //---------------- ratecontrol -------------------
  #undef MKPARAM
  #undef MKPARAMD
  #undef MKPARAMB
  #define MKPARAM(x,y)  {param.rc.x = x265Settings.ratecontrol.y;aprintf("[x265] rc."#x" = %d\n",param.rc.x);}
  #define MKPARAMD(x,y) {param.rc.x = (double)x265Settings.ratecontrol.y; aprintf("[x265] rc."#x" = %.2f\n",param.rc.x);}
  #define MKPARAMB(x,y) {param.rc.x = x265Settings.ratecontrol.y ;aprintf("[x265] rc."#x" = %s\n",TrueFalse[param.rc.x&1]);}

      MKPARAM(qpStep,qp_step);
    
      MKPARAMB(bStrictCbr,strict_cbr);
      
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
    api->param_apply_profile(&param, x265Settings.general.profile.c_str());
  }

  dumpx265Setup(&param);
  ADM_info("Creating x265 encoder\n");
  handle = api->encoder_open (&param);
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
#define PD(x) printf(#x"\t:%f\n",(double)param->x)
#define PS(x) printf(#x"\t:%s\n",param->x)
    printf("*************************************\n");
    printf("***      Encoder Environment      ***\n");
    printf("*************************************\n");
    
    PI(cpuid);
    PI(bEnableWavefront);
#if X265_BUILD >= 47
    PS(numaPools);
#else
    PI(poolNumThreads);
#endif
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
    PI(bIntraRefresh);
    PI(lookaheadDepth);
    PI(bFrameBias);
    PI(scenecutThreshold);
    
    printf("*************************************\n");
    printf("***      Intra Coding Tools       ***\n");
    printf("*************************************\n");
    
    PI(bEnableConstrainedIntra);
    PI(bIntraInBFrames);
    PI(bEnableStrongIntraSmoothing);
    
    printf("*************************************\n");
    printf("***      Inter Coding Tools       ***\n");
    printf("*************************************\n");
    
    PI(searchMethod);
    PI(subpelRefine);
    PI(searchRange);
    PI(maxNumMergeCand);
    PI(limitReferences);
    PI(bEnableWeightedPred);
    PI(bEnableWeightedBiPred);
    PI(bEnableRectInter);
    PI(bEnableAMP);
    PI(limitModes);
    
    printf("*************************************\n");
    printf("***        Analysis Tools         ***\n");
    printf("*************************************\n");
    
#if X265_BUILD < 45
    PI(bEnableCbfFastMode);
#endif
    PI(bEnableEarlySkip);
    PI(rdPenalty);
    PI(rdLevel);
    PD(psyRd);
    PI(rdoqLevel);
    PD(psyRdoq);
    
    printf("*************************************\n");
    printf("***         Coding Tools          ***\n");
    printf("*************************************\n");
    
    PI(bEnableSignHiding);
    PI(bEnableTransformSkip);
    PI(bEnableTSkipFast);
    PI(bEnableLoopFilter);
    PI(bEnableSAO);
    
#if X265_BUILD >= 33
    PI(bSaoNonDeblocked);
#else
    PI(saoLcuBoundary);
#endif
    
#if X265_BUILD < 32
    PI(saoLcuBasedOptimization);
#endif
    
    PI(cbQpOffset);
    PI(crQpOffset);
    
    PI(noiseReductionIntra);
    PI(noiseReductionInter);
    
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
    
    RI(bStrictCbr);
    
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



