//int  pyTestAssert <void >
tp_obj zzpy_testAssert(TP)
{
int r=pyTestAssert(); 
return tp_number(r);
}
//int  pyTestCrash <void >
tp_obj zzpy_testCrash(TP)
{
int r=pyTestCrash(); 
return tp_number(r);
}
pyFunc pyHelpers_functions[]={
{"testAssert",zzpy_testAssert},
{"testCrash",zzpy_testCrash},
{NULL,NULL}
};
