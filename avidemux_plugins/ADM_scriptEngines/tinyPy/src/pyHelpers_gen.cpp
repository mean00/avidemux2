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
//int  pyTestSub <char>
tp_obj zzpy_testSub(TP)
{
char * p0=(char *)TP_STR().string.val;
int r=pyTestSub(p0); 
return tp_number(r);
}
pyFunc pyHelpers_functions[]={
{"testAssert",zzpy_testAssert},
{"testCrash",zzpy_testCrash},
{"testSub",zzpy_testSub},
{NULL,NULL}
};
