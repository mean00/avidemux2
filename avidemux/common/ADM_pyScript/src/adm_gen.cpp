//int  loadVideo <char * >
tp_obj zzpy_loadVideo(TP)
{
char * p0=(char *)TP_STR().string.val;
int r=py_loadVideo(p0); 
return tp_number(r);
}
//int  clearSegments <void>
tp_obj zzpy_clearSegments(TP)
{
int r=py_clearSegments(); 
return tp_number(r);
}
//int  appendVideo <char * >
tp_obj zzpy_appendVideo(TP)
{
char * p0=(char *)TP_STR().string.val;
int r=py_appendVideo(p0); 
return tp_number(r);
}
//int  addSegment <int  float   float >
tp_obj zzpy_addSegment(TP)
{
int p0=TP_NUM();
float p1=TP_NUM();
float p2=TP_NUM();
int r=py_addSegment(p0,p1,p2); 
return tp_number(r);
}
//int  setPostProc <int  int   int >
tp_obj zzpy_setPostProc(TP)
{
int p0=TP_NUM();
int p1=TP_NUM();
int p2=TP_NUM();
int r=py_setPostProc(p0,p1,p2); 
return tp_number(r);
}
//int  getWidth <void>
tp_obj zzpy_getWidth(TP)
{
int r=py_getWidth(); 
return tp_number(r);
}
//int  getHeight <void>
tp_obj zzpy_getHeight(TP)
{
int r=py_getHeight(); 
return tp_number(r);
}
//int  getFps1000 <void>
tp_obj zzpy_getFps1000(TP)
{
int r=py_getFps1000(); 
return tp_number(r);
}
//int  audioReset <void>
tp_obj zzpy_audioReset(TP)
{
int r=py_audioReset(); 
return tp_number(r);
}
//int  audioMixer <char * >
tp_obj zzpy_audioMixer(TP)
{
char * p0=(char *)TP_STR().string.val;
int r=py_audioMixer(p0); 
return tp_number(r);
}
//int  clearVideoFilters <void>
tp_obj zzpy_clearVideoFilters(TP)
{
int r=py_clearVideoFilters(); 
return tp_number(r);
}
pyFuncs adm_functions[]={
{"loadVideo",zzpy_loadVideo},
{"clearSegments",zzpy_clearSegments},
{"appendVideo",zzpy_appendVideo},
{"addSegment",zzpy_addSegment},
{"setPostProc",zzpy_setPostProc},
{"getWidth",zzpy_getWidth},
{"getHeight",zzpy_getHeight},
{"getFps1000",zzpy_getFps1000},
{"audioReset",zzpy_audioReset},
{"audioMixer",zzpy_audioMixer},
{"clearVideoFilters",zzpy_clearVideoFilters},
{NULL,NULL}
};
