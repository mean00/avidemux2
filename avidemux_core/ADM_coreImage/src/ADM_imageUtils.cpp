/***************************************************************************
    copyright            : (C) 2003-2005 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_bitmap.h"
#include "avifmt.h"
#include "avifmt2.h"
#include "ADM_bitstream.h"

#include "DIA_coreToolkit.h"

#if 0
#include "ADM_encoderConf.h"
#include "ADM_codecs/ADM_codec.h"
#include "ADM_lavcodec.h"
#include "ADM_codecs/ADM_ffmpeg.h"
#endif
#define QT_TR_NOOP(x) x // No translation in core*
static uint8_t tinyAverage(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t l)
{

uint8_t *s1,*s2,*d1;
int a1,a2;
        s1=src1;
        s2=src2;
        
        d1=dst;
          for(int y=0;y<l;y++)
                {
                        a1=*s1;
                        a2=*s2;
                        a1=a1+a2;
                        a1>>=1;
                        if(a1<0) a1=0;
                        if(a1>255) a1=255;
                        *d1=a1;                         

                        s1++;
                        s2++;
                        d1++;
                }
        
        return 1;
}
#ifdef ADM_CPU_X86
static uint8_t tinyAverageMMX(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t l)
{
int delta;
uint32_t ww,rr;
uint8_t *s1,*s2,*d1;
int a1,a2;
        s1=src1;
        s2=src2;
        
        d1=dst;
        ww=l>>2;
        rr=l&3;


#ifdef GCC_2_95_X
         __asm__(
                         "pxor %mm7,%mm7"
                ::
                 );
#else
         __asm__(
                         "pxor %%mm7,%%mm7"
                ::
                 );
#endif

          for(int y=0;y<ww;y++)
          {
                __asm__(
                        "movd           (%0),%%mm0 \n"
                        "movd           (%1),%%mm1 \n"
                        "punpcklbw      %%mm7,%%mm0 \n"
                        "punpcklbw      %%mm7,%%mm1 \n"
                        "paddw           %%mm1,%%mm0 \n"
                        "psrlw          $1,%%mm0 \n"
                        "packuswb       %%mm0,  %%mm0\n"
                        "movd           %%mm0,(%2) \n"

                : : "r" (s1),"r" (s2),"r"(d1)
                );
                        s1+=4;
                        s2+=4;
                        d1+=4;
                }
        __asm__(
                        "emms\n"
                ::
                );
        if(rr) tinyAverage(d1, s1, s2,rr);
        return 1;
}


#endif

bool ADMImage::merge(ADMImage *src1,ADMImage *src2)
{
#ifdef ADM_CPU_X86
        if(CpuCaps::hasMMX())
        {
                tinyAverageMMX(YPLANE(this),YPLANE(src1),YPLANE(src2),(_width*_height*3)>>1);
                return 1;
        }
#endif
        tinyAverage(YPLANE(this),YPLANE(src1),YPLANE(src2),(_width*_height*3)>>1);
        return 1;


}
/* 3000 * 3000 max size, using uint32_t is safe... */
static uint32_t computeDiff(uint8_t  *s1,uint8_t *s2,uint32_t noise,uint32_t l)
{
uint32_t df=0;
uint32_t delta;
uint32_t ww,hh;


        for(int x=0;x<l;x++)
        {
                delta=abs(*s1-*s2);
                if(delta>noise)
                        df+=delta;
                s1++;
                s2++;

        }
        return df;
}
#ifdef ADM_CPU_X86
static uint64_t FUNNY_MANGLE(noise64);
static uint32_t computeDiffMMX(uint8_t  *s1,uint8_t *s2,uint32_t noise,uint32_t l)
{
uint32_t df=0;
uint32_t delta;
uint32_t ll,rr;
uint64_t noise2=(uint64_t )noise;

uint32_t result=0;
        noise64=noise2+(noise2<<16)+(noise2<<32)+(noise2<<48);
        ll=l>>2;
        rr=l&3;

#ifdef GCC_2_95_X
         __asm__(
                         "pxor %mm7,%mm7\n"
                         "pxor %mm3,%mm3\n"
                         "movq "Mangle(noise64)", %mm6\n"
                :: 
                 );
#else
         __asm__(
                         "pxor %%mm7,%%mm7\n"
                         "pxor %%mm3,%%mm3\n"
                         "movq "Mangle(noise64)", %%mm6\n"
                :: 
                 );
#endif

          for(int y=0;y<ll;y++)
          {
                __asm__(
                        "movd           (%0),  %%mm0 \n"
                        "movd           (%1),  %%mm1 \n"
                        "punpcklbw      %%mm7, %%mm0 \n"
                        "punpcklbw      %%mm7, %%mm1 \n"
                        
                        "movq           %%mm0, %%mm2 \n"
                        "psubusw        %%mm1, %%mm2 \n"
                        "psubusw        %%mm0, %%mm1 \n"
                        "por            %%mm1, %%mm2 \n" // SAD  
                                           
                        "movq           %%mm2, %%mm0 \n"
                        "pcmpgtw        %%mm6, %%mm2 \n" // Threshold against noise
                        "pand           %%mm2, %%mm0 \n" // MM0 is the 4 diff, time to pack

                        "movq           %%mm0, %%mm1 \n" // MM0 is a b c d and we want
                        "psrlq          $16,  %%mm1 \n"  // mm3+=a+b+c+d

                        "movq           %%mm0, %%mm2 \n"
                        "psrlq          $32,  %%mm2 \n"

                        "movq           %%mm0, %%mm4 \n"
                        "psrlq          $48,  %%mm4 \n"

                        "paddw          %%mm1, %%mm0 \n"
                        "paddw          %%mm2, %%mm4 \n"
                        "paddw          %%mm4, %%mm0 \n"

                        "psllq          $48,  %%mm0 \n"
                        "psrlq          $48,  %%mm0 \n"

                        "paddw          %%mm0, %%mm3 \n" /* PADDQ is SSE2 */

                : : "r" (s1),"r" (s2)
                );
                        s1+=4;
                        s2+=4;
                        
         }
        // Pack result
#if 1        
                __asm__(
                       
                        "movd           %%mm3,(%0)\n"
                        "emms\n"
                :: "r"(&result)
                );
#endif                
        if(rr) result+=computeDiff(s1, s2, noise,rr);
        return result;
}


#endif

uint32_t ADMImage::lumaDiff(ADMImage *src1,ADMImage *src2,uint32_t noise)
{

#ifdef ADM_CPU_X86
uint32_t r1,r2;
        if(CpuCaps::hasMMX())
        {
                return computeDiffMMX(YPLANE(src1),YPLANE(src2),noise,src1->_width*src1->_height);                
        }
#endif
        return computeDiff(YPLANE(src1),YPLANE(src2),noise,src1->_width*src1->_height);
}

//******************************************************************************
// so srcR=2*src-srcP
static uint8_t tinySubstract(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t l)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int a1,a2;
        s1=src1;
        s2=src2;
        
        d1=dst;
       

          for(int y=0;y<l;y++)
               
                {
                        a1=*s1;
                        a2=*s2;
                        a1=2*a1-a2;
                        if(a1<0) a1=0;
                        if(a1>255) a1=255;
                        *d1=a1;                         

                        s1++;
                        s2++;
                        d1++;
                }
        return 1;
}
#ifdef ADM_CPU_X86
static uint8_t tinySubstractMMX(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t l)
{
int delta;
uint32_t ww,hh;
uint8_t *s1,*s2,*d1;
int ll,rr;
        ll=l>>2;
        rr=l&3;
        s1=src1;
        s2=src2;
        
        d1=dst;
      
#ifdef GCC_2_95_X
        __asm__(
                         "pxor %mm7,%mm7"
                ::
                 );
#else
        __asm__(
                         "pxor %%mm7,%%mm7"
                ::
                 );
#endif
        for(int x=0;x<ll;x++)
                {
                        __asm__(
                        "movd           (%0),%%mm0 \n"
                        "movd           (%1),%%mm1 \n"
                       
                        "punpcklbw      %%mm7,%%mm0 \n"
                        "punpcklbw      %%mm7,%%mm1 \n"
                      
                        
                        "paddw          %%mm0,%%mm0 \n"
                       
                        
                        "psubusw        %%mm1,%%mm0 \n" // mm1=sum                       
                        "packuswb       %%mm0,  %%mm0\n"
                        "movd           %%mm0,(%2) \n"                       
                        :: "r"(s1),"r"(s2),"r"(d1)
                        );
                        s1+=4;
                        s2+=4;
                        d1+=4;
                }
                 __asm__(                       
                        "emms\n"
                :: 
                );
        if(rr) tinySubstractMMX(d1, s1, s2,rr);
        return 1;
}
#endif

bool ADMImage::substract(ADMImage *src1,ADMImage *src2)
{

#ifdef ADM_CPU_X86
uint32_t r1,r2;
        if(CpuCaps::hasMMX())
        {
                 return tinySubstractMMX(YPLANE(this),YPLANE(src1),YPLANE(src2),src1->_width*src1->_height);                
        }
#endif
        return tinySubstract(YPLANE(this),YPLANE(src1),YPLANE(src2),src1->_width*src1->_height);
}
  
 /**
  *		\fn  copyLeftSideTo
  * 	\brief Copy half the image (left part) to dest
  * 	@param dest : Image to copy to 
  */
 bool ADMImage::copyLeftSideTo(ADMImage *dest)
 {
		uint8_t *src,*dst;
		uint32_t stride;
 
			ADM_assert(_width==dest->_width);
			ADM_assert(_height==dest->_height);
		
	 		dst=YPLANE(dest);
		    src=YPLANE(this);
		    stride=_width;
		    for(uint32_t y=0;y<_height;y++)   // We do both u & v!
		    {
		        memcpy(dst,src,stride>>1);
		        dst+=stride;
		        src+=stride;
		    }
		        // U
		    dst=UPLANE(dest);
		    src=UPLANE(this);
		    stride=_width>>1;
		    uint32_t h2=_height>>1;
		    for(uint32_t y=0;y<h2;y++)   // We do both u & v!
		    {
		        memcpy(dst,src,stride>>1);
		        dst+=stride;
		        src+=stride;
		    }
		        // V
		    dst=VPLANE(dest);
		    src=VPLANE(this);
		    for(uint32_t y=0;y<h2;y++)   // We do both u & v!
		    {
		        memcpy(dst,src,stride>>1);
		        dst+=stride;
		        src+=stride;
		    }
		    return 1;
 }
//EOF
