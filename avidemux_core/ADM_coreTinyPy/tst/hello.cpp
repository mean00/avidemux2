#include <stdio.h>
#include <stdarg.h>
#include "../include/ADM_tinypy.h"

tp_obj my_superfunc(TP)
{
        printf("My super func!!!\n");
}
void one(void)
{
        tinyPy py;
        py.init();
        pyFuncs myfuncs[]={{"myfunc",my_superfunc},{NULL,NULL}};
        py.registerFuncs("myfuncs",myfuncs);
        py.execString("print('starting')");
        py.execString("myfunc()");
        py.execString("print('ending')");
        py.execString("i=30\nprint(i)");
        printf("The end\n");
}
int main(int argc, char **argv)
{
        one();
        one();
        one();
        one();
        printf("The REAL end\n");
        return 0;

}
