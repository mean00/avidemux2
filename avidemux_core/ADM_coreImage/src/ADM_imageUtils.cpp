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
#include "ADM_coreUtils.h"
#include "ADM_cpuCap.h"
void testYUV444();

#ifdef ADM_CPU_X86
  #define ADM_EMMS()             ADM_emms()
#endif

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
static inline void yuv444_C(uint8_t *src,uint8_t *dst,int w,int h,int dstStride,int srcStride)
{
    src+=2;
    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
                dst[x]=src[4*x];
        dst+=dstStride;
        src+=srcStride;
    }
}
#ifdef ADM_CPU_X86
extern "C"
{
void adm_YUV444Luma_mmx(int w8,uint8_t *dst,uint8_t *src, const uint64_t *mangle);
}

static inline void yuv444_MMX(uint8_t *src,uint8_t *dst,int w,int h,int destStride, int sourceStride)
{
static const uint64_t attribute_used FUNNY_MANGLE(mask) = 0x00ff000000ff0000LL;


    int step=w>>3;
    int left=w&7;
    int targetOffset=(step<<3);
    int sourceOffset=(step<<5)+2;

    for(int y=0;y<h;y++)
    {

        adm_YUV444Luma_mmx(step,dst,src,&mask);
        for(int i=0;i<left;i++)
             dst[targetOffset+i]=src[sourceOffset+4*i];
        dst+=destStride;
        src+=sourceStride;
    }
     ADM_EMMS();

}
#endif
#ifdef ADM_CPU_X86
extern "C"
{
extern void adm_YUV444_chroma_mmx(uint8_t *src, uint8_t *dst, uint8_t *dst2, int w4);
}


static inline void YUV444_chroma_MMX(uint8_t *src,uint8_t *dst,uint8_t *dst2,int w,int h,int s,int s2)
{
    int step=w/4;
    int left=w-4*step;

    for(int y=0;y<h;y++)
    {
        adm_YUV444_chroma_mmx(src,dst,dst2,step);
        for(int i=0;i<left;i++)
        {
             dst[step*4+i]=src[step*32+8*i];
             dst2[step*4+i]=src[step*32+8*i+1];
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
extern "C"
{
  void adm_uv_to_nv12_mmx(uint8_t *u, uint8_t *v, uint8_t *dst, int len);
}
static void uv_to_nv12_mmx(int w, int h,int upitch, int vpitch, uint8_t *srcv, uint8_t *srcu,int strideUV, uint8_t *dst)
{
        int mod8=w>>3;
        int leftOver=w&7;
        for(int y=0;y<h;y++)
        {
                adm_uv_to_nv12_mmx(srcu,srcv,dst,mod8);

                if(leftOver)
                {
                  uint8_t *u=srcu+mod8*8;
                  uint8_t *v=srcv+mod8*8;
                  uint8_t *ddst=dst+mod8*16;

                    for(int x=0;x<leftOver;x++)
                    {
                        ddst[0]=*(u++);
                        ddst[1]=*(v++);
                        ddst+=2;
                    }
                }
                dst+=strideUV;
                srcu+=upitch;
                srcv+=vpitch;
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
extern "C"
{
void adm_nv12_to_u_v_one_line_mmx(int w8, uint8_t *dstu, uint8_t *dstv, uint8_t *src);

}
static void nv12_to_uv_mmx(int w, int h,int upitch, int vpitch, uint8_t *dstu, uint8_t *dstv,int srcPitch, uint8_t *src)
{
        int mod8=w>>3;
        int leftOver=w&7;
        for(int y=0;y<h;y++)
        {
                uint8_t *ssrc=src;
                uint8_t *u=dstu;
                uint8_t *v=dstv;
                src+=srcPitch;
                dstu+=upitch;
                dstv+=vpitch;

                adm_nv12_to_u_v_one_line_mmx(mod8,u,v,ssrc);
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

        BitBlit(dst,dstride,src,sstride,w,h);

        //U & V
        sstride=strideUV;
        src=uvData;
        h/=2;
        w/=2;

        #if defined(ADM_CPU_X86) && 1
                if(CpuCaps::hasMMX())
                    nv12_to_uv_mmx(w,h,GetPitch(PLANAR_V),GetPitch(PLANAR_U),GetWritePtr(PLANAR_V),GetWritePtr(PLANAR_U),strideUV,uvData);
                else
        #endif
                    nv12_to_uv_c(w,h,GetPitch(PLANAR_V),GetPitch(PLANAR_U),GetWritePtr(PLANAR_V),GetWritePtr(PLANAR_U),strideUV,uvData);



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
        int sstride=GetPitch(PLANAR_Y);
        int dstride=strideY;
        uint8_t *src=GetReadPtr(PLANAR_Y);
        uint8_t *dst=yData;
        for(int y=0;y<_height;y++)
        {
            memcpy(dst,src,_width);
            src+=sstride;
            dst+=dstride;
        }
        interleaveUVtoNV12(uvData,strideUV);
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
bool    ADMImage::interleaveUVtoNV12(uint8_t *target, int targetStride)
{
        int w=_width/2;
        int h=_height/2;
#if defined(ADM_CPU_X86) && 1
        if(CpuCaps::hasMMX())
            uv_to_nv12_mmx(w,h,GetPitch(PLANAR_V),GetPitch(PLANAR_U),GetWritePtr(PLANAR_V),GetWritePtr(PLANAR_U),targetStride,target);
        else
#endif
            uv_to_nv12_c(w,h,GetPitch(PLANAR_V),GetPitch(PLANAR_U),GetReadPtr(PLANAR_V),GetReadPtr(PLANAR_U),targetStride,target);
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
            yuv444_MMX(src,dst,width,height,stride,4*width);
        else
    #endif
            yuv444_C(src,dst,width,height,stride,4*width);

    stride=this->GetPitch(PLANAR_V);
    width=this->GetWidth(PLANAR_V);
    height=this->GetHeight(PLANAR_V);
    dst=this->GetWritePtr(PLANAR_V);
    int stride2=this->GetPitch(PLANAR_U);
    uint8_t * dst2=this->GetWritePtr(PLANAR_U);
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
#if defined(ADM_CPU_X86) && 1

#define START(x) ADM_info(#x)
#define CHECKOK(x) if(!(x)) {ADM_warning(#x " failed at line %d , file %s\n",__LINE__,__FILE__);exit(-1);}
#define PASS() ADM_info("   OK\n")
void testYUV444Chroma(void)
{
    uint8_t src[50];
    uint8_t dst[50],dstb[50];
    uint8_t dst2[50],dst2b[50];

    for(int i=0;i<50;i++) src[i]=(i*0x55) ^( i+1);
    memset(dst,0,50);
    memset(dst2,0,50);
    memset(dstb,0,50);
    memset(dst2b,0,50);


#define ROW_SIZE 23

    YUV444_chroma_C(src,dst,ROW_SIZE,1,ROW_SIZE);
    YUV444_chroma_C(src+1,dst2,ROW_SIZE,1,ROW_SIZE);


    YUV444_chroma_MMX(src,dstb,dst2b,ROW_SIZE,1,ROW_SIZE,ROW_SIZE);

    START(YUV444_chroma_C);
    CHECKOK(!memcmp(dst,dstb,ROW_SIZE));
    CHECKOK(!memcmp(dst2,dst2b,ROW_SIZE));
    PASS();

}

void testUV(void)
{

    uint8_t src[50];
    uint8_t dst[50],dstb[50];
    uint8_t dst2[50],dst2b[50];

    for(int i=0;i<50;i++) src[i]=(i*0x55) ^( i+1);
    memset(dst,0,50);
    memset(dst2,0,50);
    memset(dstb,0,50);
    memset(dst2b,0,50);



    nv12_to_uv_c(ROW_SIZE,1,ROW_SIZE,ROW_SIZE,dst,dstb,0,src);
    nv12_to_uv_mmx(ROW_SIZE,1,ROW_SIZE,ROW_SIZE,dst2,dst2b,0,src);

    printf("U:\n");
    mixDump(dst,ROW_SIZE);
    mixDump(dstb,ROW_SIZE);
    printf("V:\n");
    mixDump(dst2,ROW_SIZE);
    mixDump(dst2b,ROW_SIZE);


    START(testUV);
    CHECKOK(!memcmp(dst,dst2,ROW_SIZE));
    CHECKOK(!memcmp(dstb,dst2b,ROW_SIZE));
    PASS();

}
void testYUV444Luma()
{
#define SRC_SIZE 600
    uint8_t src[SRC_SIZE];
    uint8_t dst[SRC_SIZE];
    uint8_t dst2[SRC_SIZE];

    for(int i=0;i<SRC_SIZE;i++) src[i]=i;
    memset(dst,0,SRC_SIZE);
    memset(dst2,0,SRC_SIZE);


    yuv444_MMX(src,dst,ROW_SIZE,1,ROW_SIZE,4*ROW_SIZE);
    yuv444_C(src,dst2,ROW_SIZE,1,ROW_SIZE,4*ROW_SIZE);

    printf("SRC\n");
    mixDump(src,ROW_SIZE*4);


    printf("MMX\n");
    mixDump(dst,ROW_SIZE);
    printf("C\n");
    mixDump(dst2,ROW_SIZE);

    START(testYUV444);
    CHECKOK(!memcmp(dst,dst2,ROW_SIZE));
    PASS();


}

void testInterleaveUv()
{
  #define SRC_SIZE 600
      uint8_t srcu[SRC_SIZE];
      uint8_t srcv[SRC_SIZE];
      uint8_t dst[SRC_SIZE];
      uint8_t dst2[SRC_SIZE];

      for(int i=0;i<SRC_SIZE;i++)
      {
          srcu[i]=i;
          srcv[i]=0x80^i;
      }
      memset(dst,0,SRC_SIZE);
      memset(dst2,0,SRC_SIZE);


      uv_to_nv12_mmx(ROW_SIZE,1,ROW_SIZE>>1,ROW_SIZE>>1,srcu,srcv,ROW_SIZE,dst2);
      uv_to_nv12_c(ROW_SIZE,1,ROW_SIZE>>1,ROW_SIZE>>1,srcu,srcv,ROW_SIZE,dst);


      printf("SRCU\n");
      mixDump(srcu,ROW_SIZE);
      printf("SRCV\n");
      mixDump(srcv,ROW_SIZE);


      printf("MMX\n");
      mixDump(dst,ROW_SIZE*2);
      printf("C\n");
      mixDump(dst2,ROW_SIZE*2);

      START(testInterleaveUV);
      CHECKOK(!memcmp(dst,dst2,ROW_SIZE*2));
      PASS();

}

void allTestImageUtil()
{
    testYUV444Chroma();
    testYUV444Luma();
    testUV();
    testInterleaveUv();
}
#endif
//EOF
