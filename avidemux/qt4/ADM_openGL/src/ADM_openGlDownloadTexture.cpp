/***************************************************************************
  \file T_openGL.h
  \brief OpenGL related filters
  \author (C) 2011 Mean Fixounet@free.fr 
***************************************************************************/
#include "ADM_openGl.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"


#define TEX_Y_OFFSET 2
#define TEX_U_OFFSET 1
#define TEX_V_OFFSET 0  
#define TEX_A_OFFSET 3   

#if defined( ADM_CPU_X86) 
extern "C"
{
    void adm_glYUV444_Init_mmx();
    void adm_glYUV444_luma_mmx(const uint8_t *src, uint8_t *dst, int count);
    void adm_glYUV444_luma2_mmx(const uint8_t *src, uint8_t *dst, int count);
}
void admTestDownloadTexture();
#endif

typedef void typeGlYv444(const uint8_t *src,uint8_t *dst,const int width);
typedef void typeGlYUV444(const uint8_t *src,uint8_t *dstY,uint8_t *dstU, uint8_t *dstV,const int width);

/**
 */

/**
 * \fn glYUV444_ChromaC
 * \brief very stupid downsampler for U & V plane, one line is discarded
 * @param src
 * @param toU
 * @param toV
 * @param width
 */
static inline void glYUV444_ChromaC(const uint8_t *src, uint8_t *toU, uint8_t *toV, const int width)
{
       // ?
       const uchar *p=src;
       for(int x=0;x<width;x++) // Stupid subsample: 1 out of 2
        {
            if(!*(uint32_t *)p || !*(uint32_t *)(p+4))
            {
                toU[x]=128;
                toV[x]=128;
            }else
            {
                toU[x]  =  ((int)p[TEX_U_OFFSET]+(int)p[TEX_U_OFFSET+4])>>1;
                toV[x]  =  ((int)p[TEX_V_OFFSET]+(int)p[TEX_V_OFFSET+4])>>1;
            }
            p+=8;
        }
}
/**
 * 
 */
#ifdef ADM_CPU_X86
ADM_NO_OPTIMIZE static inline void glYUV444_MMX(const uint8_t *src, uint8_t *dst, const int width)
{
    int count=width/8;
    adm_glYUV444_luma_mmx(src,dst,count);                 
    if(width&7)
    {
        for(int i=count*8;i<width;i++)
            dst[i]  = src[i*4+TEX_Y_OFFSET];
    }
}
/**
 * 
 * @param src
 * @param dstY
 * @param dstU  ** NOT USED AT THE MOMENT **
 * @param dstV
 * @param width
 */
ADM_NO_OPTIMIZE static inline void glYUV444_MMX_Chroma(const uint8_t *src2, uint8_t *dstY2, uint8_t *dstU2, uint8_t *dstV2,const int width)
{
    const uint8_t *src=src2;
    uint8_t *dstY=dstY2, *dstU=dstU2, *dstV=dstV2;
    int count=width/8;
    adm_glYUV444_luma2_mmx(src2,dstY2,count);
    if(width&7)
    {
        count=width/8;
        for(int i=count*8;i<width;i++)
            dstY2[i]  = src2[i*4+TEX_Y_OFFSET];
    }
}
/**
 */
static inline void glYUV444_YUVMMX(const uint8_t *src, uint8_t *toY,uint8_t *toU, uint8_t *toV, const int width)
{
    glYUV444_MMX(src,toY,width);
    glYUV444_ChromaC(src,toU,toV,width>>1);
    
    
}
#endif
static inline void glYUV444_C(const uint8_t *src, uint8_t *dst, const int width)
{
       for(int x=0;x<width;x++)
        {
            dst[x]  = src[x*4+TEX_Y_OFFSET];
        }
}

static inline void glYUV444_C_withChroma(const uint8_t *src, uint8_t *dstY,uint8_t *dstU, uint8_t *dstV, const int width)
{
       glYUV444_C(src,dstY,width);
       glYUV444_ChromaC(src,dstU,dstV,width>>1);
}
/**
    \fn downloadTexture
    \brief Download YUVA texture into a YV12 image
*/
bool ADM_coreQtGl::downloadTexturesQt(ADMImage *image, QOpenGLFramebufferObject *fbo)
{

    QImage qimg(fbo->toImage()); // this is slow ! ~ 15 ms for a 720 picture (Y only).
    // Assume RGB32, read R or A
    int strideY=image->GetPitch(PLANAR_Y);
    uint8_t *toY=image->GetWritePtr(PLANAR_Y);
    uint8_t *toU=image->GetWritePtr(PLANAR_U);
    uint8_t *toV=image->GetWritePtr(PLANAR_V);
    int      strideU=image->GetPitch(PLANAR_U);
    int      strideV=image->GetPitch(PLANAR_V);

    int width=image->GetWidth(PLANAR_Y);
    int height=image->GetHeight(PLANAR_Y);
    typeGlYv444  *luma=glYUV444_C;
    typeGlYUV444 *lumaAndChroma=glYUV444_C_withChroma;
#ifdef ADM_CPU_X86
      if(1 && CpuCaps::hasMMX())
      {
            adm_glYUV444_Init_mmx();
            luma=glYUV444_MMX;
            lumaAndChroma=glYUV444_YUVMMX;
      }
#endif
    const uchar **yy=(const uchar **)admAlloca(height*sizeof(uint8_t *)); // FIXME : Use alloca here
    for(int i=0;i<height;i++)
    {
        yy[i]=qimg.constScanLine(height-i-1);
        if(!yy[i])
        {
            ADM_error("Can t get pointer to openGl texture\n");
            yy=NULL;
            return false;
        }
    }
    // Do Y
    for(int y=0;y<height;y++)
    {
       const uchar *src=yy[y];
       lumaAndChroma(src,toY,toU,toV,width);
       toY+=strideY;
       toU+=strideU;
       toV+=strideV;
       // 2nd line
       y++;
       src=yy[y];
       luma(src,toY,width);
       toY+=strideY;        
    }
    
#ifdef ADM_CPU_X86
    ADM_emms();    
#endif
    yy=NULL;
    return true;
}
/**
    \fn downloadTexture
    \brief Download YUVA texture into a YV12 image
 TODO FIXME : Make same optimisation as Qt version
*/
bool ADM_coreQtGl::downloadTexturesDma(ADMImage *image, QOpenGLFramebufferObject *fbo, GLuint bufferARB)
{
    bool r=true;
    int width=image->GetWidth(PLANAR_Y);
    int height=image->GetHeight(PLANAR_Y);
    ADM_glExt::bindBuffer(GL_PIXEL_PACK_BUFFER_ARB,0);
    // that one might fail : checkGlError("BindARB-00");

    ADM_glExt::bindBuffer(GL_PIXEL_PACK_BUFFER_ARB,bufferARB);
    checkGlError("BindARB");
    ADM_glExt::bufferData(GL_PIXEL_PACK_BUFFER_ARB,width*height*sizeof(uint32_t),
                                NULL,GL_STREAM_READ_ARB);
    checkGlError("BufferDataRB");

    glReadBuffer(GL_COLOR_ATTACHMENT0_EXT); 
    checkGlError("ReadBuffer (fbo)");
    ADM_glExt::bindBuffer(GL_PIXEL_PACK_BUFFER_ARB,bufferARB);
    checkGlError("Bind Buffer (arb)");

    glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0);
    checkGlError("glReadPixel");

    // DMA call done, we can do something else here
    ADM_usleep(1*1000);


    GLubyte* ptr = (GLubyte*)ADM_glExt::mapBuffer(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);
    checkGlError("MapBuffer");
    if(!ptr)
    {
        ADM_error("Cannot map output buffer!\n");
        r=false;
    }
    else
    {
        // Assume RGB32, read R or A
        int strideY=image->GetPitch(PLANAR_Y);
        uint8_t *toY=image->GetWritePtr(PLANAR_Y);
        uint8_t *toU=image->GetWritePtr(PLANAR_U);
        uint8_t *toV=image->GetWritePtr(PLANAR_V);
        int      strideU=image->GetPitch(PLANAR_U);
        int      strideV=image->GetPitch(PLANAR_V);

        int width=image->GetWidth(PLANAR_Y);
        int height=image->GetHeight(PLANAR_Y);
        typeGlYv444 *luma=glYUV444_C;
        typeGlYUV444 *lumaAndChroma=glYUV444_C_withChroma;
#ifdef ADM_CPU_X86
          if(1 && CpuCaps::hasMMX())
          {
                adm_glYUV444_Init_mmx();
                luma=glYUV444_MMX;
                lumaAndChroma=glYUV444_YUVMMX;
          }
    #endif
        // Do Y
        const uchar *src=ptr;
        for(int y=0;y<height;y+=2)
        {
           luma(src,toY,width);
           toY+=strideY;
           src+=4*width;
           lumaAndChroma(src,toY,toU,toV,width);
           toY+=strideY;
           src+=4*width;
           toU+=strideU;
           toV+=strideV;
        }
#ifdef ADM_CPU_X86
        ADM_emms();        
#endif
        ADM_glExt::unmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
    }
    ADM_glExt::bindBuffer(GL_PIXEL_PACK_BUFFER_ARB,0);
    return r;
}

/**
    \fn downloadTexture
*/
bool ADM_coreQtGl::downloadTexture(ADMImage *image, ADM_PLANE plane, QOpenGLFramebufferObject *fbo)
{
#ifdef BENCH_READTEXTURE
    {
    ADMBenchmark bench;
    for(int i=0;i<100;i++)
    {
        bench.start();
        QImage qimg(fbo->toImage());
        bench.end();
     }
    ADM_warning("convert to Qimage\n");
    bench.printResult();
    }
#endif

    QImage qimg(fbo->toImage()); // this is slow ! ~ 15 ms for a 720 picture (Y only).



    // Assume RGB32, read R or A
#ifdef BENCH_READTEXTURE
    ADMBenchmark bench;
    for(int i=0;i<100;i++)
    {
        bench.start();
#endif
    int stride=image->GetPitch(plane);
    uint8_t *to=image->GetWritePtr(plane);
    int width=image->GetWidth(plane);
    int height=image->GetHeight(plane);
    for(int y=0;y<height;y++)
    {
        const uchar *src=qimg.constScanLine(height-y-1);
        if(!src)
        {
            ADM_error("Can t get pointer to openGl texture\n");
            return false;
        }
        for(int x=0;x<width;x++)
            to[x]=src[x*4];
        to+=stride;
    }
#ifdef BENCH_READTEXTURE
        bench.end();
    }
    bench.printResult();

#endif
    return true;
}
/**
 * \fn downloadTextures
 */
bool ADM_coreQtGl::downloadTextures(ADMImage *image, QOpenGLFramebufferObject *fbo, GLuint bufferArb)
{
#if 1 // With QT5, download QT is faster ..    
    if(ADM_glHasARB())
        return downloadTexturesDma(image,fbo,bufferArb);
#endif
    return downloadTexturesQt(image,fbo);
}


#if defined( ADM_CPU_X86) 
void admTestDownloadTexture()
{
    adm_glYUV444_Init_mmx();
    uint8_t src[512*4+8],dst[512*4+8],dst2[512*4+8];
    bool fail=false;
    
    for(int i=0;i<512*4+8;i++)
    {
        src[i]=i;
        dst[i]=0;
        dst2[i]=0;
    }
#define CHECK(x) \
    glYUV444_C(src,dst,x); \
    glYUV444_MMX(src,dst2,x); \
    if(memcmp(dst,dst2,x)) \
    { \
        printf("Fail with width=%d at line %d\n",x,__LINE__); \
        fail=true; \
    } else\
    printf(" OK with width =%d\n",x);
    
    CHECK(512);
    CHECK(510);
    CHECK(508);
    CHECK(504);
    if(fail)
    {
        printf("** FAIL **\n");
        exit(-1);
    }
    else
    {
        printf("PASS \n");
    }
}
#endif

// EOF
