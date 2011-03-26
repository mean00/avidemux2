const ADM_paramList ps_muxer_param[]={
 {"muxingType",offsetof(ps_muxer,muxingType),"uint32_t",ADM_param_uint32_t},
 {"acceptNonCompliant",offsetof(ps_muxer,acceptNonCompliant),"bool",ADM_param_bool},
 {"muxRatekBits",offsetof(ps_muxer,muxRatekBits),"uint32_t",ADM_param_uint32_t},
 {"videoRatekBits",offsetof(ps_muxer,videoRatekBits),"uint32_t",ADM_param_uint32_t},
 {"bufferSizekBytes",offsetof(ps_muxer,bufferSizekBytes),"uint32_t",ADM_param_uint32_t},
{NULL,0,NULL}
};
