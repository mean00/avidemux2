/***************************************************************************
    copyright            : (C) 2007 by mean
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
#include "ADM_colorspace.h"
#include "ADM_image.h"
#include "ADM_rgb.h" 
#include "prefs.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}


#ifdef ADM_CPU_X86
		#define ADD(x,y) if( CpuCaps::has##x()) flags|=SWS_CPU_CAPS_##y;
#define FLAGS()		ADD(MMX,MMX);				ADD(3DNOW,3DNOW);		ADD(MMXEXT,MMX2);
#else
#ifdef ADM_CPU_ALTIVEC
#define FLAGS() flags|=SWS_CPU_CAPS_ALTIVEC;
#else
#define FLAGS()
#endif
#endif

#define CONTEXT (SwsContext *)context

/**
    \fn swapRGB
*/
static void swapRGB32(uint32_t w, uint32_t h, uint32_t p, uint8_t *to)
{
    uint32_t i,l;
    uint8_t *start=(uint8_t *)to;
    for(i=0; i < h; i++)
    {
        uint8_t *d=start;
        start+=p;
        l=w;
        while(l--)
        {
            uint8_t s=d[0];
            d[0]=d[2];
            d[2]=s;
            d+=4;
        }
    }
}

/**
      \fn getStrideAndPointers
      \param dst=1 -> destination, =0 source
      \brief Fill in strides etc.. needed by libswscale
*/
uint8_t ADMColorScalerFull::getStrideAndPointers(bool dst,
        uint8_t  *from,ADM_pixelFormat fromPixFrmt,
        uint8_t **srcData,int *srcStride)
{
    uint32_t width,height;
    if(!dst)
    {
        width=srcWidth;
        height=srcHeight;
    }else
    {
        width=dstWidth;
        height=dstHeight;
    }
  switch(fromPixFrmt)
  {
    case ADM_PIXFRMT_RGB555: 
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*2);
            srcStride[1]=0;
            srcStride[2]=0;
            break;
    case ADM_PIXFRMT_RGB24:
    case ADM_PIXFRMT_BGR24:
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*3);
            srcStride[1]=0;
            srcStride[2]=0;
            break;
    case ADM_PIXFRMT_GBR24P:
            srcData[0]=from;
            width=ADM_IMAGE_ALIGN(width);
            height=ADM_IMAGE_ALIGN(height);
            from+=width*height;
            srcData[1]=from;
            from+=width*height;
            srcData[2]=from;
            srcStride[0]=width;
            srcStride[1]=width;
            srcStride[2]=width;
            break;
    case  ADM_PIXFRMT_YV12:
            srcData[0]=from;
            width=ADM_IMAGE_ALIGN(width);
            height=ADM_IMAGE_ALIGN(height);
            from+=width*height;
            srcData[1]=from;
            from+=(width>>1)*(height>>1);
            srcData[2]=from;
            srcStride[0]=width;
            srcStride[1]=width>>1;
            srcStride[2]=width>>1;
            break;
    case ADM_PIXFRMT_YUV420_10BITS:
    case ADM_PIXFRMT_YUV420_12BITS:
            srcData[0]=from;
            width=ADM_IMAGE_ALIGN(width*2);
            height=ADM_IMAGE_ALIGN(height);
            from+=width*height;
            srcData[1]=from;
            from+=(width>>1)*(height>>1);
            srcData[2]=from;
            srcStride[0]=width;
            srcStride[1]=width>>1;
            srcStride[2]=width>>1;
            break;
    case ADM_PIXFRMT_NV12:
            srcData[0]=from;
            width=ADM_IMAGE_ALIGN(width);
            height=ADM_IMAGE_ALIGN(height);
            from+=width*height;
            srcData[1]=from;
            srcData[2]=NULL;
            srcStride[0]=width;
            srcStride[1]=width;
            srcStride[2]=0;
            break;
    case  ADM_PIXFRMT_YUV422:
    case  ADM_PIXFRMT_UYVY422:        
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*2);
            srcStride[1]=0;
            srcStride[2]=0;
            break;            
    case  ADM_PIXFRMT_YUV422P:
            srcData[0]=from;
            width=ADM_IMAGE_ALIGN(width);
            height=ADM_IMAGE_ALIGN(height);
            from+=width*height;
            srcData[1]=from;
            from+=(width>>1)*height;
            srcData[2]=from;
            srcStride[0]=width;
            srcStride[1]=width>>1;
            srcStride[2]=width>>1;
            break;
    case ADM_PIXFRMT_RGB32A:
    case ADM_PIXFRMT_BGR32A:
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*4);
            srcStride[1]=0;
            srcStride[2]=0;
            break;
    case ADM_PIXFRMT_YUV422_10BITS:
            srcData[0]=from;
            width=ADM_IMAGE_ALIGN(width*2);
            height=ADM_IMAGE_ALIGN(height);
            from+=width*height;
            srcData[1]=from;
            from+=(width>>1)*height;
            srcData[2]=from;
            srcStride[0]=width;
            srcStride[1]=width>>1;
            srcStride[2]=width>>1;
            break;
    case ADM_PIXFRMT_YUV444_12BITS:
            srcData[0]=from;
            width=ADM_IMAGE_ALIGN(width*2);
            height=ADM_IMAGE_ALIGN(height);
            from+=width*height;
            srcData[1]=from;
            from+=width*height;
            srcData[2]=from;
            srcStride[0]=width;
            srcStride[1]=width;
            srcStride[2]=width;
            break;
    default:
        ADM_assert(0);
  }
  return 1;
}
/**
    \fn  convert
    \brief Do the color conversion
  @param from Source image
  @param to Target image
*/
#define swap16(x) x=((x>>8)&0xff)+(x<<8)
bool ADMColorScalerFull::convert(uint8_t  *from, uint8_t *to)
{
    uint8_t *srcData[3];
    uint8_t *dstData[3];
    int srcStride[3];
    int dstStride[3];

    getStrideAndPointers(false,from,fromPixFrmt,srcData,srcStride);
    getStrideAndPointers(true,to,toPixFrmt,dstData,dstStride);

    if(fromPixFrmt == ADM_PIXFRMT_YV12)
    {
        uint8_t *p=srcData[1];
        srcData[1]=srcData[2];
        srcData[2]=p;
    }
    if(toPixFrmt == ADM_PIXFRMT_YV12)
    {
        uint8_t *p=dstData[1];
        dstData[1]=dstData[2];
        dstData[2]=p;
    }

#ifdef BGR32_IS_SWAPPED
    if(fromPixFrmt != toPixFrmt && fromPixFrmt == ADM_PIXFRMT_BGR32A)
        swapRGB32(srcWidth,srcHeight,srcStride[0],srcData[0]);
#endif
    sws_scale(CONTEXT,srcData,srcStride,0,srcHeight,dstData,dstStride);
#ifdef BGR32_IS_SWAPPED
    if(fromPixFrmt != toPixFrmt && toPixFrmt==ADM_PIXFRMT_BGR32A)
        swapRGB32(dstWidth,dstHeight,dstStride[0],dstData[0]);
#endif
    return true;
}
/**
    \fn convertPlanes
    \brief Same as convert but the 3 planes are given separately and the caller takes care of swapping planes
*/
bool ADMColorScalerFull::convertPlanes(int sourceStride[3], int destStride[3], uint8_t *sourceData[3], uint8_t *destData[3])
{
    int xs[4]={(int)sourceStride[0],(int)sourceStride[1],(int)sourceStride[2],0};
    int xd[4]={(int)destStride[0],(int)destStride[1],(int)destStride[2],0};
    uint8_t *src[4]={NULL,NULL,NULL,NULL};
    uint8_t *dst[4]={NULL,NULL,NULL,NULL};
    for(int i=0;i<3;i++)
    {
        src[i]=sourceData[i];
        dst[i]=destData[i];
    }
#ifdef BGR32_IS_SWAPPED
    if(fromPixFrmt != toPixFrmt && fromPixFrmt == ADM_PIXFRMT_BGR32A)
        swapRGB32(srcWidth,srcHeight,xs[0],src[0]);
#endif
    sws_scale(CONTEXT,src,xs,0,srcHeight,dst,xd);
#ifdef BGR32_IS_SWAPPED
    if(fromPixFrmt != toPixFrmt && toPixFrmt == ADM_PIXFRMT_BGR32A)
        swapRGB32(dstWidth,dstHeight,xd[0],dst[0]);
#endif
    return true;
}
/**
    \fn convertImage
    \brief Both source and target are given as ADMImage
*/
bool            ADMColorScalerFull::convertImage(ADMImage *sourceImage, ADMImage *destImage)
{
    if (toneMapper != NULL)
    {
        if (toneMapper->toneMap(sourceImage, destImage))
            return true;
    }

    int xs[4];
    int xd[4];
    uint8_t *src[4];
    uint8_t *dst[4];
    sourceImage->GetPitches(xs);
    destImage->GetPitches(xd);
    xs[3]=sourceImage->GetPitch(PLANAR_ALPHA);
    xd[3]=destImage->GetPitch(PLANAR_ALPHA);
    
    destImage->GetWritePlanes(dst);
    sourceImage->GetReadPlanes(src);
    
    src[3]=sourceImage->GetReadPtr(PLANAR_ALPHA);
    dst[3]=destImage->GetWritePtr(PLANAR_ALPHA);

    if(fromPixFrmt==ADM_PIXFRMT_YV12)
    {
        uint8_t *p=src[1];
        src[1]=src[2];
        src[2]=p;
    }

    if(toPixFrmt==ADM_PIXFRMT_YV12)
    {
        uint8_t *p=dst[1];
        dst[1]=dst[2];
        dst[2]=p;
    }
    if(fromPixFrmt != toPixFrmt)
    {
        int *itbl = NULL;
        int *ta = NULL;
        int sr,dr,bri,con,sat,ret;
        ret = sws_getColorspaceDetails(CONTEXT,&itbl,&sr,&ta,&dr,&bri,&con,&sat);
        if(ret < 0)
        {
            ADM_warning("Cannot get colorspace details to set color range.\n");
        }else
        {
            sr=(sourceImage->_range == ADM_COL_RANGE_JPEG)? 1 : 0;
            dr=(destImage->_range   == ADM_COL_RANGE_JPEG)? 1 : 0;
            ret = sws_setColorspaceDetails(CONTEXT,itbl,sr,ta,dr,bri,con,sat);
            if(ret < 0)
            {
                const char *strSrc = sr? "JPEG" : "MPEG";
                const char *strDst = dr? "JPEG" : "MPEG";
                ADM_warning("Cannot set colorspace details, %s --> %s\n",strSrc,strDst);
            }
        }
#ifdef BGR32_IS_SWAPPED
        if(fromPixFrmt == ADM_PIXFRMT_BGR32A)
            swapRGB32(srcWidth,srcHeight,xs[0],src[0]);
#endif
    }else
    {
        destImage->_range = sourceImage->_range;
    }

    sws_scale(CONTEXT,src,xs,0,srcHeight,dst,xd);

#ifdef BGR32_IS_SWAPPED
    if(fromPixFrmt != toPixFrmt && toPixFrmt == ADM_PIXFRMT_BGR32A)
        swapRGB32(dstWidth,dstHeight,xd[0],dst[0]);
#endif
    return true;
}

/**
    \fn  ADMColorScaler
    \brief Constructor
  @param w width
  @param h height
  @param from colorspace to convert from
  @param to colorspace to concert to
*/

ADMColorScalerFull::ADMColorScalerFull(ADMColorScaler_algo algo,
            int sw, int sh,
            int dw, int dh,
            ADM_pixelFormat from,ADM_pixelFormat to)
{
    context=NULL;
    possibleHdrContent=false;
    toneMapper=NULL;
    reset(algo,sw,sh,dw,dh,from,to);
}
/**
    \fn  ~ADMColorScaler
    \brief Destructor
*/
ADMColorScalerFull::~ADMColorScalerFull()
{
    if(context)
    {
        sws_freeContext(CONTEXT);
        context=NULL;
    }
    if (toneMapper)
    {
        delete toneMapper;
        toneMapper=NULL;
    }
}
/**
    \fn reset
*/
bool  ADMColorScalerFull::reset(ADMColorScaler_algo algo, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to)
{
    if(context) sws_freeContext(CONTEXT);
    context=NULL;
    if (toneMapper)
    {
        delete toneMapper;
        toneMapper=NULL;
    }
    this->algo=algo;
    int flags;
    switch(algo)
    {
#define SETAL(x) case ADM_CS_##x: flags=SWS_##x;break;

    SETAL(BILINEAR);
    SETAL(FAST_BILINEAR);
    SETAL(BICUBIC);
    SETAL(LANCZOS);
    SETAL(BICUBLIN);
    SETAL(GAUSS);
    SETAL(SINC);
    SETAL(SPLINE);
    SETAL(POINT);    // nearest neighbor
    default: ADM_assert(0);
    }
#if 0 // this is gone, we need to patch av_get_cpu_flags directly now
    {
        FLAGS();
    }
#endif
    possibleHdrContent = (from >= ADM_PIXFRMT_YUV444_10BITS) && (from <= ADM_PIXFRMT_YUV444_12BITS) && (to == ADM_PIXFRMT_YV12);
    if (possibleHdrContent)
    {
        toneMapper = new ADMToneMapper(flags, sw, sh, dw, dh, from, to);
    }
    
    srcWidth=sw;
    srcHeight=sh;

    dstWidth=dw;
    dstHeight=dh;

    fromPixFrmt=from;
    toPixFrmt=to;

    if (fromPixFrmt == ADM_PIXFRMT_BGR24 && toPixFrmt == ADM_PIXFRMT_YV12)
        flags |= SWS_ACCURATE_RND; // work around an issue of output color range being always limited

    AVPixelFormat lavFrom=ADMPixFrmt2LAVPixFmt(fromPixFrmt );
    AVPixelFormat lavTo=ADMPixFrmt2LAVPixFmt(toPixFrmt );
    
    context=(void *)sws_getContext(
                      srcWidth,srcHeight,
                      lavFrom ,
                      dstWidth,dstHeight,
                      lavTo,
                      flags, NULL, NULL,NULL);
    return true;
}
//------------------------------
bool            ADMColorScalerSimple::changeWidthHeight(int newWidth, int newHeight)
{
    if(newWidth==srcWidth && newHeight==srcHeight) return true; // no change
    
     return reset(algo, newWidth,newHeight, newWidth,newHeight,fromPixFrmt,toPixFrmt);

}

/**
    \fn convertColorSpace
*/
bool ADMColorScalerFull::convertImage(ADMImage *img, uint8_t *to)
{
    uint8_t *srcPlanes[3];
    uint8_t *dstPlanes[3];
    int srcPitch[3];
    int dstPitch[3];
    img->GetPitches(srcPitch);
    img->GetReadPlanes(srcPlanes);
    getStrideAndPointers(true,to,toPixFrmt, dstPlanes, dstPitch);

    if(fromPixFrmt==ADM_PIXFRMT_YV12)
    {
        uint8_t *p=srcPlanes[1];
        srcPlanes[1]=srcPlanes[2];
        srcPlanes[2]=p;
    }
    if(toPixFrmt==ADM_PIXFRMT_YV12)
    {
        uint8_t *p=dstPlanes[1];
        dstPlanes[1]=dstPlanes[2];
        dstPlanes[2]=p;
    }
    if(img->_range==ADM_COL_RANGE_JPEG)
    {
        int *itbl = NULL;
        int *ta = NULL;
        int sr,dr,bri,con,sat,ret;
        ret = sws_getColorspaceDetails(CONTEXT,&itbl,&sr,&ta,&dr,&bri,&con,&sat);
        if(ret < 0)
            ADM_warning("Cannot get colorspace details to set color range.\n");
        else
        {
            ret = sws_setColorspaceDetails(CONTEXT,itbl,1,ta,0,bri,con,sat);
            if(ret < 0)
                ADM_warning("Cannot set colorspace details, JPEG --> MPEG\n");
        }
    }
    return convertPlanes(srcPitch,dstPitch,srcPlanes,dstPlanes);
}



// ADMRGB32Scaler
// faster than libswscale rgb2rgb resizing, using libswscale XD
// for use in ADM_flyDialogRgb
// only support RGB32 format ("ADM_PIXFRMT_RGB32A")

/**
    \fn  ADMRGB32Scaler
    \brief Constructor
  @param w width
  @param h height
  @param from colorspace to convert from
  @param to colorspace to concert to
*/

ADMRGB32Scaler::ADMRGB32Scaler(ADMColorScaler_algo algo,
            int sw, int sh,
            int dw, int dh,
            ADM_pixelFormat from,ADM_pixelFormat to)
{
    for (int i=0; i<3; i++)
    {
        context[i]=NULL;
        inputPlanes[i]=NULL;
        outputPlanes[i]=NULL;
    }
    reset(algo,sw,sh,dw,dh,from,to);
}
/**
    \fn  ~ADMRGB32Scaler
    \brief Destructor
*/
ADMRGB32Scaler::~ADMRGB32Scaler()
{
    cleanUp();
}

/**
    \fn cleanUp
*/
void ADMRGB32Scaler::cleanUp()
{
    for (int i=0; i<3; i++)
    {
        if(context[i])
        {
            sws_freeContext((SwsContext *)context[i]);
            context[i]=NULL;
        }
        if (inputPlanes[i])
        {
            delete [] inputPlanes[i];
            inputPlanes[i] = NULL;
        }
        if (outputPlanes[i])
        {
            delete [] outputPlanes[i];
            outputPlanes[i] = NULL;
        }
    }    
}

/**
    \fn reset
*/
bool  ADMRGB32Scaler::reset(ADMColorScaler_algo algo, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to)
{
    cleanUp();
    this->algo=algo;
    int flags;
    switch(algo)
    {
#define SETAL(x) case ADM_CS_##x: flags=SWS_##x;break;

    SETAL(BILINEAR);
    SETAL(FAST_BILINEAR);
    SETAL(BICUBIC);
    SETAL(LANCZOS);
    SETAL(BICUBLIN);
    SETAL(GAUSS);
    SETAL(SINC);
    SETAL(SPLINE);
    SETAL(POINT);    // nearest neighbor
    default: ADM_assert(0);
    }
#if 0 // this is gone, we need to patch av_get_cpu_flags directly now
    {
        FLAGS();
    }
#endif

    srcWidth=sw;
    srcHeight=sh;

    dstWidth=dw;
    dstHeight=dh;

    ADM_assert(to == ADM_PIXFRMT_RGB32A);
    ADM_assert(from == ADM_PIXFRMT_RGB32A);

    for (int i=0; i<3; i++)
    {
        context[i]=(void *)sws_getContext(
                            srcWidth,srcHeight,
                            AV_PIX_FMT_GRAY8 ,
                            dstWidth,dstHeight,
                            AV_PIX_FMT_GRAY8,
                            flags, NULL, NULL,NULL);
        inputPlanes[i]  = new uint8_t [ADM_IMAGE_ALIGN(srcWidth)*srcHeight];
        outputPlanes[i] = new uint8_t [ADM_IMAGE_ALIGN(dstWidth)*dstHeight];
    }    
    return true;
}

void * ADMRGB32Scaler::planeWorker(void *argptr)
{
    worker_thread_arg * arg = (worker_thread_arg*)argptr;
    
    // copy packed data to plane
    for (int y=0; y<arg->srcHeight; y++)
    {
        uint8_t * p = arg->sourceData + y*(ADM_IMAGE_ALIGN(arg->srcWidth*4));
        uint8_t * q = arg->iPlane + y*(ADM_IMAGE_ALIGN(arg->srcWidth));
        for (int x=0; x<arg->srcWidth; x++)
        {
            *q = *p;
            q++;
            p+=4;
        }
    }
    
    // resize plane
    int xs[4]={static_cast<int>(ADM_IMAGE_ALIGN(arg->srcWidth)),0,0,0};
    int xd[4]={static_cast<int>(ADM_IMAGE_ALIGN(arg->dstWidth)),0,0,0};
    uint8_t *src[4]={NULL,NULL,NULL,NULL};
    uint8_t *dst[4]={NULL,NULL,NULL,NULL};
    src[0]=arg->iPlane;
    dst[0]=arg->oPlane;
    sws_scale((SwsContext *)arg->context,src,xs,0,arg->srcHeight,dst,xd);

    // copy plane to packed format
    for (int y=0; y<arg->dstHeight; y++)
    {
        uint8_t * p = arg->destData + y*(ADM_IMAGE_ALIGN(arg->dstWidth*4));
        uint8_t * q = arg->oPlane + y*(ADM_IMAGE_ALIGN(arg->dstWidth));
        for (int x=0; x<arg->dstWidth; x++)
        {
            *p = *q;
            q++;
            p+=4;
        }
    }
    
    pthread_exit(NULL);
    return NULL;
}
/**
    \fn convert
*/
bool  ADMRGB32Scaler::convert(uint8_t *sourceData, uint8_t *destData)
{
    if ((srcWidth == dstWidth) && (srcHeight == dstHeight))
    {
        memcpy(destData, sourceData, ADM_IMAGE_ALIGN(srcWidth*4)*srcHeight);
        return true;
    }
    
    for (int i=0; i<3; i++)
    {
        worker_thread_args[i].context = context[i];
        worker_thread_args[i].sourceData = sourceData+i;
        worker_thread_args[i].destData = destData+i;
        worker_thread_args[i].dstHeight = dstHeight;
        worker_thread_args[i].dstWidth = dstWidth;
        worker_thread_args[i].iPlane = inputPlanes[i];
        worker_thread_args[i].oPlane = outputPlanes[i];
        worker_thread_args[i].srcHeight = srcHeight;
        worker_thread_args[i].srcWidth = srcWidth;
    }
    for (int i=0; i<3; i++)
    {
        pthread_create( &worker_threads[i], NULL, planeWorker, (void*) &worker_thread_args[i]);
    }
    // work in thread workers...
    
    // set alpha channel; required even if QImage::Format_RGB32 used
    for (int y=0; y<dstHeight; y++)
    {
        uint8_t * p = destData + 3 + y*(ADM_IMAGE_ALIGN(dstWidth*4));
        for (int x=0; x<dstWidth; x++)
        {
            *p = 0xFF;
            p+=4;
        }
    }
    
    for (int i=0; i<3; i++)
    {
        pthread_join( worker_threads[i], NULL);
    }    

    return true;
}

//EOF
