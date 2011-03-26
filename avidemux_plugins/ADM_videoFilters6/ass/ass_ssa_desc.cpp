const ADM_paramList ass_ssa_param[]={
 {"font_scale",offsetof(ass_ssa,font_scale),"float",ADM_param_float},
 {"line_spacing",offsetof(ass_ssa,line_spacing),"float",ADM_param_float},
 {"subtitleFile",offsetof(ass_ssa,subtitleFile),"char *",ADM_param_string},
 {"fontDirectory",offsetof(ass_ssa,fontDirectory),"char *",ADM_param_string},
 {"extractEmbeddedFonts",offsetof(ass_ssa,extractEmbeddedFonts),"uint32_t",ADM_param_uint32_t},
 {"topMargin",offsetof(ass_ssa,topMargin),"uint32_t",ADM_param_uint32_t},
 {"bottomMargin",offsetof(ass_ssa,bottomMargin),"uint32_t",ADM_param_uint32_t},
{NULL,0,NULL}
};
