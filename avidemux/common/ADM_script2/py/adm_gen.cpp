//int  jsLoadVideo <char * >
tp_obj zzpy_loadVideo(TP)
{
char * p0=(char *)TP_STR().string.val;
int r=jsLoadVideo(p0); 
return tp_number(r);
}
//int  jsClearSegments <void>
tp_obj zzpy_clearSegments(TP)
{
int r=jsClearSegments(); 
return tp_number(r);
}
//int  jsAppendVideo <char * >
tp_obj zzpy_appendVideo(TP)
{
char * p0=(char *)TP_STR().string.val;
int r=jsAppendVideo(p0); 
return tp_number(r);
}
//int  jsAddSegment <int  float   float >
tp_obj zzpy_addSegment(TP)
{
int p0=TP_NUM();
float p1=TP_NUM();
float p2=TP_NUM();
int r=jsAddSegment(p0,p1,p2); 
return tp_number(r);
}
//int  jsSetPostProc <int  int   int >
tp_obj zzpy_setPostProc(TP)
{
int p0=TP_NUM();
int p1=TP_NUM();
int p2=TP_NUM();
int r=jsSetPostProc(p0,p1,p2); 
return tp_number(r);
}
//int  jsGetWidth <void>
tp_obj zzpy_getWidth(TP)
{
int r=jsGetWidth(); 
return tp_number(r);
}
//int  jsGetHeight <void>
tp_obj zzpy_getHeight(TP)
{
int r=jsGetHeight(); 
return tp_number(r);
}
//int  jsGetFps1000 <void>
tp_obj zzpy_getFps1000(TP)
{
int r=jsGetFps1000(); 
return tp_number(r);
}
//int  jsAudioReset <void>
tp_obj zzpy_audioReset(TP)
{
int r=jsAudioReset(); 
return tp_number(r);
}
//int  jsAudioMixer <char * >
tp_obj zzpy_audioMixer(TP)
{
char * p0=(char *)TP_STR().string.val;
int r=jsAudioMixer(p0); 
return tp_number(r);
}
//int  jsClearVideoFilters <void>
tp_obj zzpy_clearVideoFilters(TP)
{
int r=jsClearVideoFilters(); 
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
