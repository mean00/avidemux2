	#include <stdio.h>
	#include <stdlib.h>
	#include <iconv.h>
        int main(int argc, char **argv)
        {
	char *in,*out;
	size_t sin, sout, sz;
	iconv_t _conv;
	sin=1;
	sout=4;
        #ifdef ICONV_NEED_CONST
 	sz=iconv(_conv,(const char **)&in,&sin,&out,&sout);
        #else
 	sz=iconv(_conv,&in,&sin,&out,&sout);
        #endif
        return 0;
        }
