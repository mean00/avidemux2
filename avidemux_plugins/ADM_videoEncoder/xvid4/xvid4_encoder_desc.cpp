const ADM_paramList xvid4_encoder_param[]={
 {"params",offsetof(xvid4_encoder,params),"COMPRES_PARAMS",ADM_param_video_encode},
 {"profile",offsetof(xvid4_encoder,profile),"uint32_t",ADM_param_uint32_t},
 {"rdMode",offsetof(xvid4_encoder,rdMode),"uint32_t",ADM_param_uint32_t},
 {"motionEstimation",offsetof(xvid4_encoder,motionEstimation),"uint32_t",ADM_param_uint32_t},
 {"cqmMode",offsetof(xvid4_encoder,cqmMode),"uint32_t",ADM_param_uint32_t},
 {"arMode",offsetof(xvid4_encoder,arMode),"uint32_t",ADM_param_uint32_t},
 {"maxBFrame",offsetof(xvid4_encoder,maxBFrame),"uint32_t",ADM_param_uint32_t},
 {"maxKeyFrameInterval",offsetof(xvid4_encoder,maxKeyFrameInterval),"uint32_t",ADM_param_uint32_t},
 {"nbThreads",offsetof(xvid4_encoder,nbThreads),"uint32_t",ADM_param_uint32_t},
 {"rdOnBFrame",offsetof(xvid4_encoder,rdOnBFrame),"bool",ADM_param_bool},
 {"hqAcPred",offsetof(xvid4_encoder,hqAcPred),"bool",ADM_param_bool},
 {"optimizeChrome",offsetof(xvid4_encoder,optimizeChrome),"bool",ADM_param_bool},
 {"trellis",offsetof(xvid4_encoder,trellis),"bool",ADM_param_bool},
{NULL,0,NULL}
};
