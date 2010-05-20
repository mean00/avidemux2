#include <stdio.h>
#include <stdarg.h>
#include "../include/ADM_tinypy.h"

typedef enum
{
    JS_LOG_NORMAL,
    JS_LOG_ERROR
}JS_LOG_TYPE;

bool jsLog(JS_LOG_TYPE type, const char *prf,...)
{
         static char print_buffer[1024];

                va_list         list;
                va_start(list,  prf);
                vsnprintf(print_buffer,1023,prf,list);
                va_end(list);
                print_buffer[1023]=0; // ensure the string is terminated
                printf("%s",print_buffer);
                return true;

}

void one(void)
{
        tinyPy py;
        py.init();
        py.execString("print('hi there')");
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
