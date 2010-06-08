//int  scriptTestAssert <void >
tp_obj zzpy_testAssert(TP)
{
int r=scriptTestAssert(); 
return tp_number(r);
}
//int  scriptTestCrash <void >
tp_obj zzpy_testCrash(TP)
{
int r=scriptTestCrash(); 
return tp_number(r);
}
pyFuncs pyHelpers_functions[]={
{"testAssert",zzpy_testAssert},
{"testCrash",zzpy_testCrash},
{NULL,NULL}
};
