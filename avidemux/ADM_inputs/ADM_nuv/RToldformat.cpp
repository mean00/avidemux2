/*
	Simple wrapper to allow support for both original RTjpeg stream
		and MythTv ones


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADM_assert.h"
#include <math.h>
#include "config.h"
#include "ADM_default.h"

#ifndef UNUSED_ARG
	#define UNUSED_ARG(x) if(x) {}
#endif


#if !defined(_I386_TYPES_H) || defined(ADM_BIG_ENDIAN)

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int8_t __s8;
typedef int16_t __s16;
typedef int32_t __s32;
#endif

#include "ADM_rtGlob.h"
extern "C" {
	#include "RTjpeg.h"
};

 	int 		baseRT::SetSize(int *w, int *h)
	{
		UNUSED_ARG(w);
		UNUSED_ARG(h);
		return 1;
	};
 	int 		baseRT::SetFormat(int *format)
	{
		UNUSED_ARG(format);
		return 1;
	};
 	int 		baseRT::InitLong(char  *data, int w, int h)
	{
		UNUSED_ARG(w);
		UNUSED_ARG(h);
		UNUSED_ARG(data);
		return 1;
	};
 	 void	 baseRT::Decompress(int8_t *sp, uint8_t **planes)
	 {
	 	UNUSED_ARG(sp);
		UNUSED_ARG(planes);
	 	return ;
	};

baseRT::baseRT(void) {};
baseRT::~baseRT(void) {};
baseRTold::baseRTold(void) {};
baseRTold::~baseRTold(void) {};

int baseRTold::SetSize(int *w, int *h)
 {
UNUSED_ARG(w);
UNUSED_ARG(h);
return 1;}
int baseRTold::SetFormat(int *w) {
UNUSED_ARG(w);
return 1;}
int baseRTold::InitLong(char  *data, int w, int h) 
{
 	RTjpeg_init_decompress ( (uint32_t *)data, w,h);
	return 1;
}

 void	 baseRTold::Decompress(int8_t *sp, uint8_t **planes)
 {
 	RTjpeg_decompressYUV420 (sp, planes[0] );	

 }
