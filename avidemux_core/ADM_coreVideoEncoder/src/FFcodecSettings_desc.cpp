const ADM_paramList FFcodecSettings_param[]={
 {"params",offsetof(FFcodecSettings,params),"COMPRES_PARAMS",ADM_param_video_encode},
 {"lavcSettings",offsetof(FFcodecSettings,lavcSettings),"FFcodecContext",ADM_param_lavcodec_context},
{NULL,0,NULL}
};
