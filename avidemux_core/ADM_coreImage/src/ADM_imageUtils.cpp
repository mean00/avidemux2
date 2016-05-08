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
#include "ADM_bitstream.h"
#include "DIA_coreToolkit.h"


#define ADM_CLEAR_MM7()         __asm__ volatile( "pxor %%mm7,%%mm7"      ::    )
#define ADM_EMMS()              __asm__ volatile( "emms\n"                ::    )

//#undef ADM_CPU_X86

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

        ADM_CLEAR_MM7();
          for(int y=0;y<ww;y++)
          {
                __asm__ volatile(
                        "movd           (%0),%%mm0 \n"
                        "movd           (%1),%%mm1 \n"
                        "punpcklbw      %%mm7,%%mm0 \n"
                        "punpcklbw      %%mm7,%%mm1 \n"
                        "paddw           %%mm1,%%mm0 \n"
                        "psrlw          $1,%%mm0 \n"
                        "packuswb       %%mm0,  %%mm0\n"
                        "movd           %%mm0,(%2) \n"

                : : "r" (s1),"r" (s2),"r"(d1)
                :"memory"
                );
                        s1+=4;
                        s2+=4;
                        d1+=4;
                }
       ADM_EMMS();
        if(rr) tinyAverage(d1, s1, s2,rr);
        return 1;
}


#endif
/**

*/
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
/**
    \fn blendC
    \brief Blend src1 and src2 into target (one plane)
*/
typedef bool blendFunction(int width, int height, uint8_t *target,uint32_t stride,    uint8_t *src1,uint32_t stride1,               uint8_t *src2,uint32_t stride2);

static bool blendC(int width, int height,
                    uint8_t *target,uint32_t stride,
                    uint8_t *src1,  uint32_t stride1,
                    uint8_t *src2,  uint32_t stride2)
{
          for(int y=0;y<height;y++)
                {
                    uint8_t *s1=src1,*s2=src2,*d=target;
                    unsigned int a;
                    for(int x=0;x<width;x++)
                    {
                        a=((unsigned int)*s1)+((unsigned int)*s2);
                        a>>=1;
                        *d=(uint8_t)a;
                        s1++;
                        s2++;
                        d++;
                    }
                    src1+=stride1;src2+=stride2;target+=stride;
                }
        return true;
}
#ifdef ADM_CPU_X86
static bool blendMMX(int width, int height,
                    uint8_t *target,uint32_t stride,
                    uint8_t *src1,  uint32_t stride1,
                    uint8_t *src2,  uint32_t stride2)
{
uint32_t ww,rr;
uint8_t *s1,*s2,*d1;
        int a1,a2;
      
        ww=width>>2;
        rr=width&3;


         ADM_CLEAR_MM7();

          for(int y=0;y<height;y++)
          {
                s1=src1;
                s2=src2;
                d1=target;
                if(rr)
                {
                    blendC(rr,height,d1+(ww<<2), stride,s1+(ww<<2),stride1,s2+(ww<<2),stride2);
                }

                for(int x=0;x<ww;x++)
                {
                    __asm__ volatile(
                            "movd           (%0),%%mm0 \n"
                            "movd           (%1),%%mm1 \n"
                            "punpcklbw      %%mm7,%%mm0 \n"
                            "punpcklbw      %%mm7,%%mm1 \n"
                            "paddw           %%mm1,%%mm0 \n"
                            "psrlw          $1,%%mm0 \n"
                            "packuswb       %%mm0,  %%mm0\n"
                            "movd           %%mm0,(%2) \n"

                    : : "r" (s1),"r" (s2),"r"(d1)
                    :"memory"
                    );
                    s1+=4;
                    s2+=4;
                    d1+=4;
                }
                src1+=stride1;
                src2+=stride2;
                target+=stride;
           }
        ADM_EMMS();
        return true;
}
/**
    \fn blendSSE
*/
static bool blendSSE(int width, int height,
                    uint8_t *target,uint32_t stride,
                    uint8_t *src1,  uint32_t stride1,
                    uint8_t *src2,  uint32_t stride2)
{
uint32_t ww,rr;
uint8_t *s1,*s2,*d1;
        int a1,a2;

        ww=width>>3;
        rr=width&7;

          for(int y=0;y<height;y++)
          {
                int count=ww;
                s1=src1;
                s2=src2;
                d1=target;
                if(rr)
                {
                    blendC(rr,height,d1+(ww<<3), stride,s1+(ww<<3),stride1,s2+(ww<<3),stride2);
                }

                    
                __asm__ volatile(
                        
                        "1: \n"
                        "movq           (%0),%%mm0  \n"
                        "movq           (%1),%%mm1  \n"
                        "pavgb          %%mm1,%%mm0 \n"
                        "movq           %%mm0,(%2)  \n"
                        "add           $8,%0      \n"
                        "add           $8,%1      \n"
                        "add           $8,%2      \n"
                        "sub           $1,%3      \n"
                        "jnz           1b        \n"
                        

                : : "r" (s1),"r" (s2),"r"(d1),"r"(count)
                :"memory"
                );
                src1+=stride1;
                src2+=stride2;
                target+=stride;
           }
        ADM_EMMS();
        return true;
}

#endif
/**
    \fn blend
    \brief Blend src1 and src2 into our image
*/
bool ADMImage::blend(ADMImage *src1,ADMImage *src2)
{
    blendFunction *myBlend=blendC;
#ifdef ADM_CPU_X86
    if(CpuCaps::hasMMX())
            myBlend=blendMMX;
    if(CpuCaps::hasSSE())
            myBlend=blendSSE;
#endif
    ADM_assert(src1->_width==src2->_width);
    ADM_assert(_width==src2->_width);
    ADM_assert(src1->_height==src2->_height);
    for(int x=0;x<3;x++)
    {
        ADM_PLANE plane=(ADM_PLANE)x;
        myBlend(GetWidth(plane),GetHeight(plane),
                                    GetWritePtr(plane),GetPitch(plane),
                                    src1->GetReadPtr(plane),src1->GetPitch(plane),
                                    src2->GetReadPtr(plane),src2->GetPitch(plane)
                                );
    }
    return true;
}
/**

*/
/* 3000 * 3000 max size, using uint32_t is safe... */
static uint32_t computeDiff(uint8_t  *s1,uint8_t *s2,uint32_t noise,int w,int h, int stride1, int stride2)
{
uint32_t df=0;
uint32_t delta;

    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
        {
                delta=abs((int)(s1[x])-(int)(s2[x]));
                if(delta>noise)
                        df+=delta;
        }
        s1+=stride1;
        s2+=stride2;
    }
    return df;
}
/**
 * 
 * @param s1
 * @param s2
 * @param noise
 * @param w
 * @param h
 * @param stride1
 * @param stride2
 * @return 
 */
static uint32_t smallDiff(uint8_t  *s1,uint8_t *s2,uint32_t noise, int count)
{
uint32_t df=0;
uint32_t delta;
    for(int x=0;x<count;x++)
    {
            delta=abs((int)(s1[x])-(int)(s2[x]));
            if(delta>noise)
                    df+=delta;
    }
    return df;
}
#ifdef ADM_CPU_X86
static uint64_t __attribute__((used)) FUNNY_MANGLE(noise64);
/**
 * \fn computeDiffMMX
 * @param s1
 * @param s2
 * @param noise
 * @param w
 * @param l
 * @param pitch1
 * @param pitch2
 * @return 
 */
static uint32_t computeDiffMMX(uint8_t  *s1,uint8_t *s2,uint32_t noise,uint32_t w,uint32_t l, uint32_t pitch1, uint32_t pitch2)
{
uint32_t mod4,leftOver;
uint64_t noise2=(uint64_t )noise;

uint32_t result=0,tmpResult;
        noise64=noise2+(noise2<<16)+(noise2<<32)+(noise2<<48);
        
        leftOver=w&7;

         __asm__ volatile(
                         "pxor %%mm7,%%mm7\n"                       
                         "movq "Mangle(noise64)", %%mm6\n"
                :::  "memory"
                 );

          for(int y=0;y<l;y++)
          {
                mod4=w>>3;
                if(leftOver)
                    result+=smallDiff(s1+mod4*8,s2+mod4*8,noise,leftOver);
                uint8_t *tmpS1=s1;
                uint8_t *tmpS2=s2;
                
                __asm__ volatile(
                        "pxor           %%mm3,%%mm3\n"
                        "1:"
                // LEFT
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
                        "pand           %%mm2, %%mm0 \n" //
                        "movq           %%mm0, %%mm5 \n" //  %mm5 is the  A1 A2 A3 A4, we want the sum later
                // RIGHT
                        "movd           4(%0),  %%mm0 \n"
                        "movd           4(%1),  %%mm1 \n"
                        "punpcklbw      %%mm7, %%mm0 \n"
                        "punpcklbw      %%mm7, %%mm1 \n"
                        
                        "movq           %%mm0, %%mm2 \n"
                        "psubusw        %%mm1, %%mm2 \n"
                        "psubusw        %%mm0, %%mm1 \n"
                        "por            %%mm1, %%mm2 \n" // SAD  
                                           
                        "movq           %%mm2, %%mm0 \n"
                        "pcmpgtw        %%mm6, %%mm2 \n" // Threshold against noise
                        "pand           %%mm2, %%mm0 \n" // mm0 is B1 B2 B3 B4
                
                        "paddW          %%mm5, %%mm0 \n"
                
                // PACK
                        "movq           %%mm0, %%mm1 \n" // MM0 is a b c d and we want
                        "psrlq          $16,  %%mm1 \n"  // mm3+=a+b+c+d

                        "movq           %%mm0, %%mm2 \n"
                        "psrlq          $32,  %%mm2 \n"

                        "movq           %%mm0, %%mm4 \n"
                        "psrlq          $48,  %%mm4 \n"

                        "paddw          %%mm1, %%mm0 \n"
                        "paddw          %%mm2, %%mm4 \n"
                        "paddw          %%mm4, %%mm0 \n" // MM0 is the sum

                        "psllq          $48,  %%mm0 \n"
                        "psrlq          $48,  %%mm0 \n" // Only keep 16 bits

                        "paddw          %%mm0, %%mm3 \n" /* PADDQ is SSE2 */                        
                        "add            $8,%0      \n"
                        "add            $8,%1      \n"
                        "sub            $1,%2      \n"
                        "jnz            1b         \n"

                : "=r" (tmpS1),"=r" (tmpS2),"=r"(mod4)
                : "0"(tmpS1),"1"(tmpS2),"2"(mod4)
                : "memory","0","1","2"
                );
                __asm__ volatile(
                       
                        "movd           %%mm3,(%0)\n"
                        "emms\n"
                :: "r"(&tmpResult)
                );
                result+=tmpResult;
                s1+=pitch1;
                s2+=pitch2;
         }
        // Pack result
        return result;
}


#endif

uint32_t ADMImage::lumaDiff(ADMImage *src1,ADMImage *src2,uint32_t noise)
{

#ifdef ADM_CPU_X86
uint32_t r1,r2;
        if(CpuCaps::hasMMX())
        {
                uint32_t a= computeDiffMMX(YPLANE(src1),YPLANE(src2),noise,  
                    src1->GetWidth(PLANAR_Y),src1->GetHeight(PLANAR_Y),
                    src1->GetPitch(PLANAR_Y),src2->GetPitch(PLANAR_Y));                
#if 0
                uint32_t b= computeDiff(YPLANE(src1),YPLANE(src2),noise,
                    src1->GetWidth(PLANAR_Y),src1->GetHeight(PLANAR_Y),
                    src1->GetPitch(PLANAR_Y),src2->GetPitch(PLANAR_Y));
                printf("MMX = %u, native =%u\n",a,b);
#endif
                return a;
                
        }
#endif
        return computeDiff(YPLANE(src1),YPLANE(src2),noise,
                src1->GetWidth(PLANAR_Y),src1->GetHeight(PLANAR_Y),
                src1->GetPitch(PLANAR_Y),src2->GetPitch(PLANAR_Y));
}

//******************************************************************************
// so srcR=2*src-srcP
static uint8_t tinySubstract(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t l)
{
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
      
        ADM_CLEAR_MM7();
        for(int x=0;x<ll;x++)
        {
                        __asm__ volatile(
                        "movd           (%0),%%mm0 \n"
                        "movd           (%1),%%mm1 \n"
                       
                        "punpcklbw      %%mm7,%%mm0 \n"
                        "punpcklbw      %%mm7,%%mm1 \n"
                      
                        
                        "paddw          %%mm0,%%mm0 \n"
                       
                        
                        "psubusw        %%mm1,%%mm0 \n" // mm1=sum                       
                        "packuswb       %%mm0,  %%mm0\n"
                        "movd           %%mm0,(%2) \n"                       
                        :: "r"(s1),"r"(s2),"r"(d1)
                        :"memory"
                        );
                        s1+=4;
                        s2+=4;
                        d1+=4;
        }
        ADM_EMMS();
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
    uint32_t srcStride,dstStride;
    uint32_t len=this->_width;

        ADM_assert(_width==dest->_width);
        ADM_assert(_height==dest->_height);

        dst=dest->GetWritePtr(PLANAR_Y);
        src=this->GetWritePtr(PLANAR_Y);
        srcStride=this->GetPitch(PLANAR_Y);
        dstStride=dest->GetPitch(PLANAR_Y);
        for(uint32_t y=0;y<_height;y++)   // We do both u & v!
        {
            memcpy(dst,src,len>>1);
            dst+=dstStride;
            src+=srcStride;
        }
        len>>=1;
                    // U
        dst=dest->GetWritePtr(PLANAR_U);
        src=this->GetWritePtr(PLANAR_U);
        srcStride=this->GetPitch(PLANAR_U);
        dstStride=dest->GetPitch(PLANAR_U);

        uint32_t h2=_height>>1;
        for(uint32_t y=0;y<h2;y++)   // We do both u & v!
        {
            memcpy(dst,src,len>>1);
            dst+=dstStride;
            src+=srcStride;
        }
                    // V
        dst=dest->GetWritePtr(PLANAR_V);
        src=this->GetWritePtr(PLANAR_V);
        srcStride=this->GetPitch(PLANAR_V);
        dstStride=dest->GetPitch(PLANAR_V);
        for(uint32_t y=0;y<h2;y++)   // We do both u & v!
        {
            memcpy(dst,src,len>>1);
            dst+=dstStride;
            src+=srcStride;
        }
        return 1;
}


/**
    \fn copyPlane
*/
bool ADMImage::copyPlane(ADMImage *s, ADMImage *d, ADM_PLANE plane)
{
        uint8_t *src=s->GetReadPtr(plane);
        uint8_t *dst=d->GetWritePtr(plane);
        uint32_t sPitch=s->GetPitch(plane);
        uint32_t dPitch=d->GetPitch(plane);
        uint32_t w=s->_width;  
        uint32_t h=s->_height;
        if(plane!=PLANAR_Y) 
        {
            w>>=1;
            h>>=1;
        }
        BitBlit(dst,dPitch,src,sPitch,w,h);
        return true;
}
/**
    \fn convertFromYUV444
*/
static inline void yuv444_C(uint8_t *src,uint8_t *dst,int w,int h,int s)
{
    src+=2;
    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
                dst[x]=src[4*x];
        dst+=s;
        src+=4*w;
    }
}
#ifdef ADM_CPU_X86
static inline void yuv444_MMX(uint8_t *src,uint8_t *dst,int w,int h,int s)
{
static uint64_t __attribute__((used)) FUNNY_MANGLE(mask) = 0x00ff000000ff0000LL;

    __asm__ volatile(" movq "Mangle(mask)", %%mm7\n" ::);
    __asm__ volatile(" pxor %%mm6,%%mm6\n" ::);
    
    int step=w/8;
    int left=w-8*step;
    uint8_t *xsrc=src;
    uint8_t *xdst=dst;

    for(int y=0;y<h;y++)
    {
        xsrc=src;
        xdst=dst;
        for(int x=0;x<step;x++)
        {
                        __asm__ volatile(
                        "movq           (%0),%%mm0 \n"
                        "pand           %%mm7,%%mm0\n"
                        "movq           8(%0),%%mm1 \n"
                        "pand           %%mm7,%%mm1\n"

                        "movq           16(%0),%%mm2 \n"
                        "pand           %%mm7,%%mm2\n"
                        "movq           24(%0),%%mm3 \n"
                        "pand           %%mm7,%%mm3\n"

                        "packuswb       %%mm1,%%mm0\n"
                        "packuswb       %%mm3,%%mm2\n"
                        "psrlw          $8,%%mm0\n"
                        "psrlw          $8,%%mm2\n"
                        "packuswb       %%mm2,%%mm0\n"

                        "movq           %%mm0,(%1) \n"                       
                        
                        
                        :: "r"(xsrc),"r"(xdst)
                        :"memory"
                        );
                        xsrc+=32;
                        xdst+=8;
            }
        for(int i=0;i<left;i++)
             xdst[i]=xsrc[4*i];
        dst+=s;
        src+=4*w;
    }
     ADM_EMMS();

}
#endif
#ifdef ADM_CPU_X86
static inline void YUV444_chroma_MMX(uint8_t *src,uint8_t *dst,uint8_t *dst2,int w,int h,int s,int s2)
{
    int step=w/4;
    int left=w-4*step;
    uint8_t *xsrc=src;
    uint8_t *xdst=dst;
    uint8_t *xdst2=dst2;
    

    for(int y=0;y<h;y++)
    {
        xsrc=src;
        xdst=dst;
        xdst2=dst2;
        for(int x=0;x<step;x++)
        {
                        __asm__ volatile(
                        "movq           (%0),%%mm0 \n"
                        "movq           8(%0),%%mm1 \n"
                        "movq           16(%0),%%mm2 \n"
                        "movq           24(%0),%%mm3 \n"
        
                        "movq           %%mm0,%%mm4\n"
                        "movq           %%mm1,%%mm5\n"
                        "movq           %%mm2,%%mm6\n"
                        "movq           %%mm3,%%mm7\n"

                        "punpcklbw       %%mm2,%%mm0\n"
                        "punpcklbw       %%mm3,%%mm1\n"
                        "punpcklbw       %%mm1,%%mm0\n"
                        
                        "movd           %%mm0,(%1) \n"                       

                        "psrlw          $8,%%mm4\n"
                        "psrlw          $8,%%mm5\n"
                        "psrlw          $8,%%mm6\n"
                        "psrlw          $8,%%mm7\n"

                        "punpcklbw       %%mm6,%%mm4\n"
                        "punpcklbw       %%mm7,%%mm5\n"
                        "punpcklbw       %%mm5,%%mm4\n"


                        "movd           %%mm4,(%2) \n"                       
                        :: "r"(xsrc),"r"(xdst),"r"(xdst2)
                        :"memory"
                        );
                        xsrc+=32;
                        xdst+=4;
                        xdst2+=4;
            }
        for(int i=0;i<left;i++)
        {
             xdst[i]=xsrc[8*i];
             xdst2[i]=xsrc[8*i+1];
        }
        dst+=s;
        dst2+=s2;
        src+=4*w*4;
    }
     ADM_EMMS();

}
#endif
/**
    \fn YUV444_chroma_C
*/
static inline void YUV444_chroma_C(uint8_t *src,uint8_t *dst,int w,int h,int s)
{
    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
                dst[x]=src[8*x];
        dst+=s;
        src+=4*w*2*2;
    }
}


#ifdef ADM_CPU_X86
/**
 * \fn uv_to_nv12_mmx
 * \brief unpack nv12 interleaved uv into planar uv
 * @param w
 * @param h
 * @param upitch
 * @param vpitch
 * @param dstu
 * @param dstv
 * @param srcPitch
 * @param src
 */
static void uv_to_nv12_mmx(int w, int h,int upitch, int vpitch, uint8_t *srcv, uint8_t *srcu,int strideUV, uint8_t *dst)
{
        int mod8=w>>3;
        int leftOver=w&7;
        int x;
        for(int y=0;y<h;y++)
        {
                uint8_t *ddst=dst;                
                uint8_t *u=srcu;
                uint8_t *v=srcv;
                dst+=strideUV;
                srcu+=upitch;
                srcv+=vpitch;   
                      __asm__ volatile(
                        "mov            %4,%3      \n" // local copy
                        "1:\n"
                        "movq           (%1),%%mm0   \n" // U
                        "movq           (%2),%%mm1   \n" // V
                        "movq           %%mm0,%%mm2  \n"                       
                        "movq           %%mm1,%%mm3  \n"   

                        "punpcklbw      %%mm1,%%mm0  \n"
                        "punpckhbw      %%mm3,%%mm2  \n"
                        "movq           %%mm0,(%0)   \n"                       
                        "movq           %%mm2,8(%0)  \n"     
                        
                        "add            $16,%0       \n"
                        "add            $8,%1        \n"
                        "add            $8,%2        \n"
                        "sub            $1,%3        \n"
                        "jnz            1b           \n"
                        
                        :
                        : "r"(ddst),"r"(u),"r"(v),"r"(x),"r"(mod8)
                        : "memory"
                        );
                if(leftOver)
                {
                    x=mod8*8;
                    ddst+=x*2;
                    v+=x;
                    u+=x;
                    for(;x<w;x++)
                    {
                        ddst[0]=*(u++);
                        ddst[1]=*(v++);
                        ddst+=2;
                    }
                }
        }
        ADM_EMMS();
        return ;
}

/**
 * \fn nv12_to_uv_mmx
 * @param w
 * @param h
 * @param upitch
 * @param vpitch
 * @param dstu
 * @param dstv
 * @param srcPitch
 * @param src
 */

static void nv12_to_u_v_mmx_one_line(int w8, uint8_t *dstu, uint8_t *dstv, uint8_t *src)
{
      __asm__ volatile(


                  "1:\n"                  
                  "movq           (%4),%%mm0   \n"
                  "movq           8(%4),%%mm1  \n"                              
                  "movq           %%mm0,%%mm2  \n"                       
                  "movq           %%mm1,%%mm3  \n"   

                  "psllw          $8,%%mm0    \n"                              
                  "psrlw          $8,%%mm0    \n"    

                  "psllw          $8,%%mm1    \n"                              
                  "psrlw          $8,%%mm1    \n"    


                  "packuswb       %%mm1,%%mm0 \n"

                  "psrlw          $8,%%mm2    \n"                              
                  "psrlw          $8,%%mm3    \n"    

                  "packuswb       %%mm3,%%mm2 \n"

                  "movq           %%mm0,(%6)  \n"                       
                  "movq           %%mm2,(%5)  \n"     

                  "add            $16,%4\n"
                  "add            $8,%5\n"
                  "add            $8,%6\n"
                  "sub            $1,%7\n"
                  "jnz            1b\n"

                  : "=r"(src),"=r"(dstu),"=r"(dstv),"=r"(w8) //00..3
                  : "0"(src),"1"(dstu),"2"(dstv),"3"(w8)     //4..7
                  : "memory"
                  );
}

static void nv12_to_uv_mmx(int w, int h,int upitch, int vpitch, uint8_t *dstu, uint8_t *dstv,int srcPitch, uint8_t *src)
{
        int mod8=w>>3;
        int leftOver=w&7;
        int x;
        for(int y=0;y<h;y++)
        {
                uint8_t *ssrc=src;                
                uint8_t *u=dstu;
                uint8_t *v=dstv;
                src+=srcPitch;
                dstu+=upitch;
                dstv+=vpitch;                        

                nv12_to_u_v_mmx_one_line(mod8,u,v,ssrc);
                if(leftOver)
                {
                    int c=mod8*8;
                    ssrc+=mod8*16;
                    u+=mod8*8;
                    v+=mod8*8;
                    for(;c<w;c++)
                    {
                        *u++=ssrc[1];
                        *v++=ssrc[0];
                        ssrc+=2;
                    }
                }
        }
        ADM_EMMS();
}

#endif
static void nv12_to_uv_c(int w, int h,int upitch, int vpitch, uint8_t *dstu, uint8_t *dstv,int srcPitch, uint8_t *src)
{
        
        for(int y=0;y<h;y++)
        {
                uint8_t *ssrc=src;                
                uint8_t *u=dstu;
                uint8_t *v=dstv;
                src+=srcPitch;
                dstu+=upitch;
                dstv+=vpitch;

                for(int x=0;x<w;x++)
                {
                    *u++=ssrc[1];
                    *v++=ssrc[0];
                    ssrc+=2;
                }
        }
}

/**
 * \fn convertFromNV12
 * @param yData
 * @param uvData
 * @param strideY
 * @param strideUV
 * @return 
 */
bool    ADMImage::convertFromNV12(uint8_t *yData, uint8_t *uvData, int strideY, int strideUV)
{
        // Y
    int w=_width;
    int h=_height;
        int dstride=GetPitch(PLANAR_Y);
        int sstride=strideY;
        uint8_t *dst=GetWritePtr(PLANAR_Y);
        uint8_t *src=yData;
        for(int y=0;y<_height;y++)
        {
            memcpy(dst,src,w);
            src+=sstride;
            dst+=dstride;
        }
        
        //U & V
        sstride=strideUV;
        src=uvData;
        h/=2;
        w/=2;
        
        #if defined(ADM_CPU_X86) && 1
                if(CpuCaps::hasMMX())
                    nv12_to_uv_mmx(w,h,GetPitch(PLANAR_U),GetPitch(PLANAR_V),GetWritePtr(PLANAR_U),GetWritePtr(PLANAR_V),strideUV,uvData);
                else
        #endif   
                    nv12_to_uv_c(w,h,GetPitch(PLANAR_U),GetPitch(PLANAR_V),GetWritePtr(PLANAR_U),GetWritePtr(PLANAR_V),strideUV,uvData);

       
        
        return true;
}
/**
 * \fn convertFromNV12
 * @param yData
 * @param uvData
 * @param strideY
 * @param strideUV
 * @return 
 */
bool    ADMImage::convertToNV12(uint8_t *yData, uint8_t *uvData, int strideY, int strideUV)
{
        // Y
        int w=_width;
        int h=_height;
        int sstride=GetPitch(PLANAR_Y);
        int dstride=strideY;
        uint8_t *src=GetReadPtr(PLANAR_Y);
        uint8_t *dst=yData;
        for(int y=0;y<_height;y++)
        {
            memcpy(dst,src,w);
            src+=sstride;
            dst+=dstride;
        }
        interleaveUV(uvData,strideUV);
        return true;
}
/**
        \fn nv12_to_uv_c
*/
static void uv_to_nv12_c(int w, int h,int upitch, int vpitch, uint8_t *srcu, uint8_t *srcv,int dstride, uint8_t *dst)
{ 
        for(int y=0;y<h;y++)
        {                
                uint8_t *u=srcu;
                uint8_t *v=srcv;
                uint8_t *d=dst;
                srcu+=upitch;
                srcv+=vpitch;
                dst+=dstride;

                for(int x=0;x<w;x++)
                {
                    d[0]=*v++;
                    d[1]=*u++;
                    d+=2;
                }
        }
 } 
/**
 * 
 * @param target
 * @param stride
 * @return 
 */
bool    ADMImage::interleaveUV(uint8_t *target, int stride)
{    
        int w=_width/2;
        int h=_height/2;    
#if defined(ADM_CPU_X86) && 1
        if(CpuCaps::hasMMX())
            uv_to_nv12_mmx(w,h,GetPitch(PLANAR_U),GetPitch(PLANAR_V),GetWritePtr(PLANAR_U),GetWritePtr(PLANAR_V),stride,target);
        else
#endif   
            uv_to_nv12_c(w,h,GetPitch(PLANAR_U),GetPitch(PLANAR_V),GetReadPtr(PLANAR_U),GetReadPtr(PLANAR_V),stride,target);
        return true;
}
/**
 * \fn convertFromYUV444
 * @param from
 * @return 
 */
bool ADMImage::convertFromYUV444(uint8_t *from)
{
    int stride=this->GetPitch(PLANAR_Y);
    int width=this->GetWidth(PLANAR_Y);
    int height=this->GetHeight(PLANAR_Y);
    uint8_t *dst=this->GetWritePtr(PLANAR_Y);
    uint8_t *src=from;

    #if defined(ADM_CPU_X86) && 1
        if(CpuCaps::hasMMX())
            yuv444_MMX(src,dst,width,height,stride);            
        else
    #endif
            yuv444_C(src,dst,width,height,stride);
    

    //
    stride=this->GetPitch(PLANAR_U);
    width=this->GetWidth(PLANAR_U);
    height=this->GetHeight(PLANAR_U);
    dst=this->GetWritePtr(PLANAR_U);
    int stride2=this->GetPitch(PLANAR_V);
    uint8_t * dst2=this->GetWritePtr(PLANAR_V);
    src=from+0;
    #ifdef ADM_CPU_X86
        if(  CpuCaps::hasMMX())
        {
            YUV444_chroma_MMX(src,dst,dst2,width,height,stride,stride2); 
        }
        else
    #endif
        {
            YUV444_chroma_C(src,dst,width,height,stride);
            YUV444_chroma_C(src+1,dst2,width,height,stride2);
        }

    return true;
}
//EOF
