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

#include <cmath>
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
    unsigned int toneMappingMethod;
    if(!prefs->get(HDR_TONEMAPPING,&toneMappingMethod))
        toneMappingMethod = 0;
    if ((toneMapper != NULL) && (toneMappingMethod > 0))
    {
        float targetLuminance, saturation;
        if(!prefs->get(HDR_TARGET_LUMINANCE,&targetLuminance))
            targetLuminance = 100.0;
        if(!prefs->get(HDR_SATURATION,&saturation))
            saturation = 1.0;
        if (toneMapper->toneMap(sourceImage, destImage, toneMappingMethod, targetLuminance, saturation))
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
        toneMapper = new ADMToneMapper(algo, sw, sh, dw, dh, from, to);
    }
    
    srcWidth=sw;
    srcHeight=sh;

    dstWidth=dw;
    dstHeight=dh;

    fromPixFrmt=from;
    toPixFrmt=to;
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





#define CONTEXTYUV (SwsContext *)contextYUV
#define CONTEXTRGB1 (SwsContext *)contextRGB1
#define CONTEXTRGB2 (SwsContext *)contextRGB2

/**
    \fn  ADMToneMapper
    \brief Ctor
*/
ADMToneMapper::ADMToneMapper(ADMColorScaler_algo algo,
            int sw, int sh,
            int dw, int dh,
            ADM_pixelFormat from,ADM_pixelFormat to)
{
    contextYUV=NULL;
    contextRGB1=NULL;
    contextRGB2=NULL;
    hdrLumaLUT = NULL;
    hdrRGBLUT = NULL;
    for (int i=0; i<256; i++)
    {
        hdrChromaBLUT[i] = NULL;
        hdrChromaRLUT[i] = NULL;
        hdrLumaCrLUT[i] = NULL;
    }
    hdrTMsrcLum = hdrTMtrgtLum = hdrTMsat = -1.0;	// invalidate
    hdrTMmethod = 0;
    hdrYUV=NULL;
    for (int i=0; i<3; i++)
    {
        hdrRGB[i]=NULL;
        sdrRGB[i]=NULL;
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
    srcWidth=sw;
    srcHeight=sh;

    dstWidth=dw;
    dstHeight=dh;

    ADM_assert(to==ADM_PIXFRMT_YV12);
    fromPixFrmt=from;
    toPixFrmt=to;

    AVPixelFormat lavFrom=ADMPixFrmt2LAVPixFmt(fromPixFrmt );
    AVPixelFormat lavTo=ADMPixFrmt2LAVPixFmt(toPixFrmt );
    
    contextYUV=(void *)sws_getContext(
                      srcWidth,srcHeight,
                      lavFrom ,
                      dstWidth,dstHeight,
                      AV_PIX_FMT_YUV420P16LE,
                      flags, NULL, NULL,NULL);

    contextRGB1=(void *)sws_getContext(
                      srcWidth,srcHeight,
                      lavFrom ,
                      srcWidth,srcHeight,
                      AV_PIX_FMT_GBRP16LE,
                      SWS_POINT, NULL, NULL,NULL);		// use fast n.n. scaling, as resolution will not change in the first step
    contextRGB2=(void *)sws_getContext(
                      srcWidth,srcHeight,
                      AV_PIX_FMT_GBR24P ,
                      dstWidth,dstHeight,
                      lavTo,
                      flags, NULL, NULL,NULL);
}


/**
    \fn  ~ADMToneMapper
    \brief Destructor
*/
ADMToneMapper::~ADMToneMapper()
{
    if(contextYUV)
    {
        sws_freeContext(CONTEXTYUV);
        contextYUV=NULL;
    }
    if(contextRGB1)
    {
        sws_freeContext(CONTEXTRGB1);
        contextRGB1=NULL;
    }
    if(contextRGB2)
    {
        sws_freeContext(CONTEXTRGB2);
        contextRGB2=NULL;
    }

    delete [] hdrLumaLUT;
    delete [] hdrRGBLUT;
    for (int i=0; i<256; i++)
    {
        delete [] hdrChromaBLUT[i];
        delete [] hdrChromaRLUT[i];
        delete [] hdrLumaCrLUT[i];
    }
    if (hdrYUV)
    {
        delete [] hdrYUV;
        hdrYUV = NULL;
    }
    for (int i=0; i<3; i++)
    {
        delete [] hdrRGB[i];
        delete [] sdrRGB[i];
    }
}


/**
    \fn toneMap
*/
bool ADMToneMapper::toneMap(ADMImage *sourceImage, ADMImage *destImage, unsigned int toneMappingMethod, double targetLuminance, double saturationAdjust)
{
    if (hdrTMmethod != toneMappingMethod)
    {
        hdrTMmethod = toneMappingMethod;
        hdrTMsrcLum = hdrTMtrgtLum = hdrTMsat = -1.0;	// invalidate
    }

    switch (toneMappingMethod)
    {
        case 1:	// fastYUV
                return toneMap_fastYUV(sourceImage, destImage, targetLuminance, saturationAdjust);
            break;
        case 2:
        case 3:
        case 4:
                return toneMap_RGB(sourceImage, destImage, toneMappingMethod, targetLuminance, saturationAdjust);
        default:
            return false;
    }
}


/**
    \fn toneMap_fastYUV
*/
bool ADMToneMapper::toneMap_fastYUV(ADMImage *sourceImage, ADMImage *destImage, double targetLuminance, double saturationAdjust)
{
    // Check if tone mapping is needed & can do 
    if (!((sourceImage->_colorTrc == ADM_COL_TRC_SMPTE2084) || (sourceImage->_colorTrc == ADM_COL_TRC_ARIB_STD_B67) || (sourceImage->_colorSpace == ADM_COL_SPC_BT2020_NCL) || (sourceImage->_colorSpace == ADM_COL_SPC_BT2020_CL)))
        return false;
    if ((sourceImage->_colorTrc == ADM_COL_TRC_BT2020_10) || (sourceImage->_colorTrc == ADM_COL_TRC_BT2020_12))	// excluding trc, not hdr
        return false;
    if (!isnan(sourceImage->_hdrInfo.colorSaturationWeight))
        if (sourceImage->_hdrInfo.colorSaturationWeight > 0)
            saturationAdjust *= sourceImage->_hdrInfo.colorSaturationWeight;
    
    // Determine max luminance
    double maxLuminance = 10000.0;
    if (!isnan(sourceImage->_hdrInfo.maxLuminance))
        if (sourceImage->_hdrInfo.maxLuminance > 0)
            if (maxLuminance > sourceImage->_hdrInfo.maxLuminance)
                maxLuminance = sourceImage->_hdrInfo.maxLuminance;
    if (!isnan(sourceImage->_hdrInfo.targetMaxLuminance))
        if (sourceImage->_hdrInfo.targetMaxLuminance > 0)
            if (maxLuminance > sourceImage->_hdrInfo.targetMaxLuminance)
                maxLuminance = sourceImage->_hdrInfo.targetMaxLuminance;
    double boost = 1;
    if ((!isnan(sourceImage->_hdrInfo.maxCLL)) && (!isnan(sourceImage->_hdrInfo.maxFALL)))
        if ((sourceImage->_hdrInfo.maxCLL > 0) && (sourceImage->_hdrInfo.maxFALL > 0))
            boost = sourceImage->_hdrInfo.maxCLL / sourceImage->_hdrInfo.maxFALL;

    // Allocate if not done yet
    if (hdrLumaLUT == NULL)
    {
        hdrLumaLUT = new uint8_t[ADM_COLORSPACE_HDR_LUT_SIZE];
        if (hdrLumaLUT == NULL)
            return false;
    }
    for (int i=0; i<256; i++)
    {
        if (hdrChromaBLUT[i] == NULL)
        {
            hdrChromaBLUT[i] = new uint8_t[ADM_COLORSPACE_HDR_LUT_SIZE];
            if (hdrChromaBLUT[i] == NULL)
                return false;
        }
        if (hdrChromaRLUT[i] == NULL)
        {
            hdrChromaRLUT[i] = new uint8_t[ADM_COLORSPACE_HDR_LUT_SIZE];
            if (hdrChromaRLUT[i] == NULL)
                return false;
        }
        if (hdrLumaCrLUT[i] == NULL)
        {
            hdrLumaCrLUT[i] = new uint8_t[256];
            if (hdrLumaCrLUT[i] == NULL)
                return false;
        }
    }
    if (hdrYUV == NULL)
    {
        hdrYUV = new uint16_t[ADM_IMAGE_ALIGN(dstWidth)*dstHeight*2];
        if (hdrYUV == NULL)
            return false;
    }
    
    // Populate LUTs if parameters have changed
    bool extended_range = false;
    if ((maxLuminance != hdrTMsrcLum) || (targetLuminance != hdrTMtrgtLum) || (saturationAdjust != hdrTMsat))
    {
        hdrTMsrcLum = maxLuminance;
        hdrTMtrgtLum = targetLuminance;
        hdrTMsat = saturationAdjust;
        
        for (int l=0; l<ADM_COLORSPACE_HDR_LUT_SIZE; l+=1)
        {
            double LumHDR = maxLuminance;	// peak mastering display luminance [nit]
            double LumSDR = targetLuminance;		// peak target display luminance [nit]

            double Y = l;
            // normalize:
            Y /= ADM_COLORSPACE_HDR_LUT_SIZE;
            Y -= 16.0/256.0;	// deal with limited range
            Y *= 256.0/220.0;
            if (Y < 0)
                Y = 0.0;
            if (Y > 1)
                Y = 1.0;

            // WTF? step 0
            Y = std::pow(Y, 2.4);

            // Tone mapping step 1
            double rhoHDR = 1 + 32*std::pow(LumHDR/10000.0, 1/2.4);
            double Yp = std::log(1 + (rhoHDR-1)*Y) / std::log(rhoHDR);

            // Tone mapping step 2
            double Yc;
            if (Yp < 0)
                Yc = 0.0;
            else if ((0 <= Yc) && (Yc <= 0.7399))
                Yc = 1.0770*Yp;
            else if ((0.7399 < Yc) && (Yc <  0.9909))
                Yc = -1.1510*Yp*Yp + 2.7811*Yp - 0.6302;
            else if (( 0.9909 <= Yc) && (Yc <= 1.0))
                Yc = 0.5*Yp + 0.5;
            else // Yc > 1
                Yc = 1.0;

            // Tone mapping step 3
            double rhoSDR = 1 + 32*std::pow(LumSDR/10000.0, 1/2.4);
            double YSDR = (std::pow(rhoSDR, Yc) - 1) / (rhoSDR - 1);

            // WTF? step 4
            double maxboost = std::sqrt(2)*1.1;
            if (std::sqrt(boost) > maxboost)
                maxboost = std::sqrt(boost);
            YSDR *= maxboost;
            if (YSDR < 0)
                YSDR = 0;
            if (YSDR > 1)
                YSDR = 1.0;

            int YSDRint;
            if (extended_range)
                YSDRint = std::round(255.0*YSDR);
            else
                YSDRint = std::round(219.0*YSDR) + 16;
            hdrLumaLUT[l] = YSDRint;

            for (int l=0; l<ADM_COLORSPACE_HDR_LUT_SIZE; l+=1)
            {
                double C = l;
                C /= ADM_COLORSPACE_HDR_LUT_SIZE;
                C -= 16.0/256.0;	// deal with limited range
                C *= 256.0/224.0;
                if (C < 0)
                    C = 0.0;
                if (C > 1)
                    C = 1.0;
                C -= 0.5;

                double fYSDR = ((Y==0)||(YSDR==0)) ? 1.0 : (YSDR / (1.1*Y));
                C *= fYSDR;
                // WTF? step 5 prevent shadow glow
                C *= std::pow(YSDR+0.001,1/2.4)*saturationAdjust;
                if (C < -0.5)
                    C = -0.5;
                if (C > 0.5)
                    C = 0.5;
                C += 0.5;
                if (extended_range)
                {
                    hdrChromaBLUT[YSDRint][l] = std::round(255.0*C);
                    hdrChromaRLUT[YSDRint][l] = std::round(255.0*C);
                }
                else
                {
                    hdrChromaBLUT[YSDRint][l] = std::round(224.0*C) + 16;
                    hdrChromaRLUT[YSDRint][l] = std::round(224.0*C) + 16;
                }
            }
        }

        for (int cr=0; cr<256; cr++)
        {
            for (int y=0; y<256; y++)
            {
                double yn, crn;
                if (extended_range)
                {
                    crn = cr/255.0;
                    yn = y/255.0;
                }
                else
                {
                    crn = cr;
                    crn -= 16.0;
                    crn /= 224.0;
                    if (crn < 0) crn = 0;
                    if (crn > 1) crn = 1;
                    yn = y;
                    yn -= 16.0;
                    yn /= 219.0;
                    if (yn < 0) yn = 0;
                    if (yn > 1) yn = 1;
                }
                crn -= 0.5;
                if (crn > 0)
                    yn -= 0.1*crn;
                if (yn < 0) yn = 0;
                int YSDRint;
                if (extended_range)
                    YSDRint = std::round(255.0*yn);
                else
                    YSDRint = std::round(219.0*yn) + 16;
                hdrLumaCrLUT[cr][y] = YSDRint;
            }
        }
    }
    
    // Do tone mapping
    uint8_t * srcData[3];
    int srcStride[3];
    uint8_t * dstData[3];
    int dstStride[3];
    
    sourceImage->GetPitches(srcStride);
    destImage->GetPitches(dstStride);
    sourceImage->GetReadPlanes(srcData);
    destImage->GetWritePlanes(dstData);
    
    // ADM_PIXFRMT_YV12 swapped UV
    uint8_t *p=dstData[1];
    dstData[1]=dstData[2];
    dstData[2]=p;
    
    uint8_t *gbrData[3];
    int gbrStride[3];
    gbrData[0] = (uint8_t*)hdrYUV;
    gbrStride[0] = ADM_IMAGE_ALIGN(dstWidth)*2;
    gbrStride[1] = gbrStride[2]= ADM_IMAGE_ALIGN(dstWidth/2)*2;
    gbrData[1] = gbrData[0] + gbrStride[0]*(dstHeight);
    gbrData[2] = gbrData[1] + gbrStride[1]*(dstHeight/2);

    sws_scale(CONTEXTYUV,srcData,srcStride,0,srcHeight,gbrData,gbrStride);

    uint8_t * ptr = dstData[0];
    uint16_t * hptr = (uint16_t *)gbrData[0];
    for (int q=0;q<ADM_IMAGE_ALIGN(dstWidth)*dstHeight;q++)
    {
        ptr[q] = hdrLumaLUT[(ADM_COLORSPACE_HDR_LUT_SIZE-1)&(hptr[q]>>(16-ADM_COLORSPACE_HDR_LUT_WIDTH))];
    }
    uint8_t * ptrU = dstData[1];
    uint8_t * ptrV = dstData[2];
    uint16_t * hptrU = (uint16_t *)gbrData[1];
    uint16_t * hptrV = (uint16_t *)gbrData[2];
    int ystride = ADM_IMAGE_ALIGN(dstWidth);
    int uvstride = ADM_IMAGE_ALIGN(dstWidth/2);
    uint8_t * ptrNext = dstData[0]+ystride;
    for (int y=0; y<(dstHeight/2); y++)
    {
        for (int x=0; x<(dstWidth/2); x++)
        {
            int luma = 0;
            luma += ptr[x*2];
            luma += ptr[x*2 +1];
            luma += ptrNext[x*2];
            luma += ptrNext[x*2 +1];
            luma /= 4;
            luma &= 0xFF;
            ptrU[x] = hdrChromaBLUT[luma][(ADM_COLORSPACE_HDR_LUT_SIZE-1)&(hptrU[x]>>(16-ADM_COLORSPACE_HDR_LUT_WIDTH))];
            ptrV[x] = hdrChromaRLUT[luma][(ADM_COLORSPACE_HDR_LUT_SIZE-1)&(hptrV[x]>>(16-ADM_COLORSPACE_HDR_LUT_WIDTH))];

            int Cr = ptrV[x]&0xFF;
            ptr[x*2] = hdrLumaCrLUT[Cr][ptr[x*2]];
            ptr[x*2+1] = hdrLumaCrLUT[Cr][ptr[x*2+1]];
            ptrNext[x*2] = hdrLumaCrLUT[Cr][ptrNext[x*2]];
            ptrNext[x*2+1] = hdrLumaCrLUT[Cr][ptrNext[x*2+1]];
        }
        ptr += ystride*2;
        ptrNext += ystride*2;
        ptrU += uvstride;
        ptrV += uvstride;
        hptrU += uvstride;
        hptrV += uvstride;
    }

    // Reset output color properties
    destImage->_pixfrmt=ADM_PIXFRMT_YV12;
    destImage->_range=ADM_COL_RANGE_MPEG;
    destImage->_colorPrim=ADM_COL_PRI_BT709;
    destImage->_colorTrc=ADM_COL_TRC_BT709;
    destImage->_colorSpace=ADM_COL_SPC_BT709;
    
    return true;
}


/**
    \fn toneMap_fastYUV
*/
bool ADMToneMapper::toneMap_RGB(ADMImage *sourceImage, ADMImage *destImage, unsigned int method, double targetLuminance, double saturationAdjust)
{
    // Check if tone mapping is needed & can do 
    if (!((sourceImage->_colorTrc == ADM_COL_TRC_SMPTE2084) || (sourceImage->_colorTrc == ADM_COL_TRC_ARIB_STD_B67) || (sourceImage->_colorSpace == ADM_COL_SPC_BT2020_NCL) || (sourceImage->_colorSpace == ADM_COL_SPC_BT2020_CL)))
        return false;
    if ((sourceImage->_colorTrc == ADM_COL_TRC_BT2020_10) || (sourceImage->_colorTrc == ADM_COL_TRC_BT2020_12))	// excluding trc, not hdr
        return false;
    if (!isnan(sourceImage->_hdrInfo.colorSaturationWeight))
        if (sourceImage->_hdrInfo.colorSaturationWeight > 0)
            saturationAdjust *= sourceImage->_hdrInfo.colorSaturationWeight;
    
    // Determine max luminance
    double maxLuminance = 10000.0;
    if (!isnan(sourceImage->_hdrInfo.maxLuminance))
        if (sourceImage->_hdrInfo.maxLuminance > 0)
            if (maxLuminance > sourceImage->_hdrInfo.maxLuminance)
                maxLuminance = sourceImage->_hdrInfo.maxLuminance;
    if (!isnan(sourceImage->_hdrInfo.targetMaxLuminance))
        if (sourceImage->_hdrInfo.targetMaxLuminance > 0)
            if (maxLuminance > sourceImage->_hdrInfo.targetMaxLuminance)
                maxLuminance = sourceImage->_hdrInfo.targetMaxLuminance;
    double peakLuminance = maxLuminance;
    if (!isnan(sourceImage->_hdrInfo.maxCLL))
        if (sourceImage->_hdrInfo.maxCLL > 0)
            if (peakLuminance > sourceImage->_hdrInfo.maxCLL)
                peakLuminance = sourceImage->_hdrInfo.maxCLL;
    double boost = 1;
    if ((!isnan(sourceImage->_hdrInfo.maxCLL)) && (!isnan(sourceImage->_hdrInfo.maxFALL)))
        if ((sourceImage->_hdrInfo.maxCLL > 0) && (sourceImage->_hdrInfo.maxFALL > 0))
            boost = sourceImage->_hdrInfo.maxCLL / sourceImage->_hdrInfo.maxFALL;

    // Allocate if not done yet
    if (hdrRGBLUT == NULL)
    {
        hdrRGBLUT = new uint8_t[ADM_COLORSPACE_HDR_LUT_SIZE];
        if (hdrRGBLUT == NULL)
            return false;
    }
    for (int i=0; i<3; i++)
    {
        if (hdrRGB[i] == NULL)
        {
            hdrRGB[i] = new uint16_t[ADM_IMAGE_ALIGN(dstWidth)*dstHeight];
            if (hdrRGB[i] == NULL)
                return false;
        }
        if (sdrRGB[i] == NULL)
        {
            sdrRGB[i] = new uint8_t[ADM_IMAGE_ALIGN(dstWidth)*dstHeight];
            if (sdrRGB[i] == NULL)
                return false;
        }
    }


    // Populate LUTs if parameters have changed
    bool extended_range = false;
    if ((maxLuminance != hdrTMsrcLum) || (targetLuminance != hdrTMtrgtLum) || (saturationAdjust != hdrTMsat))
    {
        hdrTMsrcLum = maxLuminance;
        hdrTMtrgtLum = targetLuminance;
        hdrTMsat = saturationAdjust;
        
        for (int l=0; l<ADM_COLORSPACE_HDR_LUT_SIZE; l+=1)
        {
            double LumHDR = maxLuminance;	// peak mastering display luminance [nit]
            double LumSDR = targetLuminance;		// peak target display luminance [nit]

            double Y = l;
            // normalize:
            Y /= ADM_COLORSPACE_HDR_LUT_SIZE;
            
            double Ylin;
            // linearize
            if (sourceImage->_colorTrc == ADM_COL_TRC_ARIB_STD_B67)	//HLG
            {
                if (Y <= 0.5)
                    Ylin = Y*Y/3.0;
                else
                    Ylin = (std::exp((Y-0.55991073)/0.17883277) + 0.28466892)/12;
            } else
            if ((sourceImage->_colorTrc == ADM_COL_TRC_SMPTE2084) || (sourceImage->_colorSpace == ADM_COL_SPC_BT2020_NCL) || (sourceImage->_colorSpace == ADM_COL_SPC_BT2020_CL))	//PQ
            {
                Ylin = 0;
                if ((std::pow(Y, 1/78.84375) - 0.8359375) > 0)
                    Ylin = std::pow((std::pow(Y, 1/78.84375) - 0.8359375) / (18.8515625 - 18.6875*std::pow(Y, 1/78.84375)) , 1/0.1593017578125);
            } else {
                Ylin = std::pow(Y, 2.6);
            }
            
            Ylin *= LumHDR/LumSDR;
            double npl = peakLuminance/LumSDR;
            
            double Ytm = Ylin;
            // tonemap
            switch (method)
            {
                default:
                case 2:	// clip
                        Ytm *= std::sqrt(boost);
                        if (Ytm > 1.0)
                            Ytm = 1.0;
                    break;
                case 3:	// reinhard
                        Ytm *= std::sqrt(boost);
                        Ytm = Ytm/(1.0+Ytm);
                        Ytm *= (npl+1)/npl;
                    break;
                case 4:	// hable
                        Ytm = (Ytm * (Ytm * 0.15 + 0.50 * 0.10) + 0.20 * 0.02) / (Ytm * (Ytm * 0.15 + 0.50) + 0.20 * 0.30) - 0.02 / 0.30;
                        Ytm /= (npl * (npl * 0.15 + 0.50 * 0.10) + 0.20 * 0.02) / (npl * (npl * 0.15 + 0.50) + 0.20 * 0.30) - 0.02 / 0.30;
                        Ytm *= boost;
                    break;
            }
            
            // gamma
            double YSDR;
            YSDR = ((Ytm > 0.0031308) ? (( 1.055 * std::pow(Ytm, (1.0 / 2.4)) ) - 0.055) : (Ytm * 12.92));
            if (YSDR < 0)
                YSDR = 0;
            if (YSDR > 1)
                YSDR = 1;
            hdrRGBLUT[l] = std::round(255.0*YSDR);
        }
        
        for (int l=0; l<256; l+=1)
        {
            double C = l;
            C -= 128;
            C *= saturationAdjust;
            C += 128;
            if (C < 0)
                C = 0;
            if (C > 255)
                C = 255;
            sdrRGBSat[l] = std::round(C);
        }
    }

    // Do tone mapping
    uint8_t * srcData[3];
    int srcStride[3];
    uint8_t * dstData[3];
    int dstStride[3];
    
    sourceImage->GetPitches(srcStride);
    destImage->GetPitches(dstStride);
    sourceImage->GetReadPlanes(srcData);
    destImage->GetWritePlanes(dstData);
    
    // ADM_PIXFRMT_YV12 swapped UV
    uint8_t *p=dstData[1];
    dstData[1]=dstData[2];
    dstData[2]=p;
    
    uint8_t *gbrData[3];
    int gbrStride[3];

    for (int p=0; p<3; p++)
    {
        gbrData[p] = (uint8_t*)hdrRGB[p];
        gbrStride[p] = ADM_IMAGE_ALIGN(srcWidth)*2;
    }
    sws_scale(CONTEXTRGB1,srcData,srcStride,0,srcHeight,gbrData,gbrStride);
    
    for (int p=0; p<3; p++)
    {
        uint8_t * sdr = sdrRGB[p];
        uint16_t * hdr = hdrRGB[p];
        for (int xy=0; xy<ADM_IMAGE_ALIGN(srcWidth)*srcHeight; xy++)
        {
            sdr[xy] = hdrRGBLUT[(ADM_COLORSPACE_HDR_LUT_SIZE-1)&(hdr[xy]>>(16-ADM_COLORSPACE_HDR_LUT_WIDTH))];
        }
    }

    for (int p=0; p<3; p++)
    {
        gbrData[p] = (uint8_t*)sdrRGB[p];
        gbrStride[p] = ADM_IMAGE_ALIGN(srcWidth);
    }
    const int *coeffsrc = sws_getCoefficients(SWS_CS_BT2020);
    const int *coeffdst = sws_getCoefficients(SWS_CS_ITU709);
    sws_setColorspaceDetails(CONTEXTRGB2, coeffsrc, 0, coeffdst, 0, 0, (1 << 16), (1 << 16));
    sws_scale(CONTEXTRGB2,gbrData,gbrStride,0,srcHeight,dstData,dstStride);

    // dst is YUV420
    for (int p=1; p<3; p++)
    {
        uint8_t * ptr = dstData[p];
        for (int q=0; q<(dstStride[p] * dstHeight/2); q++)
        {
            *ptr = sdrRGBSat[*ptr];
            ptr++;
        }
    }

    // Reset output color properties
    destImage->_pixfrmt=ADM_PIXFRMT_YV12;
    destImage->_range=ADM_COL_RANGE_MPEG;
    destImage->_colorPrim=ADM_COL_PRI_BT709;
    destImage->_colorTrc=ADM_COL_TRC_BT709;
    destImage->_colorSpace=ADM_COL_SPC_BT709;
    
    return true;

}
//EOF
