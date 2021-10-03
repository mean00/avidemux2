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
#include "prefs.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
}

#include "ADM_rgb.h" 
#include "ADM_colorspace.h"

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
#define HDRYUVCONTEXT (SwsContext *)hdrYUVContext
#define BGR32_IS_SWAPPED 1

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
    \fn ADMColor2LAVColor
    \brief Convert ADM colorspace type swscale/lavcodec colorspace name

*/
static AVPixelFormat ADMColor2LAVColor(ADM_colorspace fromColor_)
{
  ADM_colorspace fromColor=fromColor_;
  int intColor=(int)fromColor;
  intColor&=ADM_COLOR_MASK;
  fromColor=(ADM_colorspace)intColor;
  switch(fromColor)
  {
    case ADM_COLOR_YUV444: return AV_PIX_FMT_YUV444P;
    case ADM_COLOR_YUV411: return AV_PIX_FMT_YUV411P;
    case ADM_COLOR_YUV422: return AV_PIX_FMT_YUYV422;
    case ADM_COLOR_UYVY422: return AV_PIX_FMT_UYVY422;
    case ADM_COLOR_YV12: return AV_PIX_FMT_YUV420P;
    case ADM_COLOR_NV12: return AV_PIX_FMT_NV12;
    case ADM_COLOR_YUV422P: return AV_PIX_FMT_YUV422P;
    case ADM_COLOR_RGB555: return AV_PIX_FMT_RGB555LE;
    case ADM_COLOR_BGR555: return AV_PIX_FMT_BGR555LE;
    case ADM_COLOR_RGB32A: return AV_PIX_FMT_RGBA;
#ifdef BGR32_IS_SWAPPED
    case ADM_COLOR_BGR32A: return AV_PIX_FMT_RGBA; // Faster that way...
#else
    case ADM_COLOR_BGR32A: return AV_PIX_FMT_BGRA;
#endif
    case ADM_COLOR_RGB24: return AV_PIX_FMT_RGB24;
    case ADM_COLOR_BGR24: return AV_PIX_FMT_BGR24;
    case ADM_COLOR_GBR24P: return AV_PIX_FMT_GBRP;
    case ADM_COLOR_YUV420_10BITS: return AV_PIX_FMT_YUV420P10LE;
    case ADM_COLOR_YUV420_12BITS: return AV_PIX_FMT_YUV420P12LE;
    case ADM_COLOR_NV12_10BITS:  return AV_PIX_FMT_P010LE;
    case ADM_COLOR_YUV444_10BITS: return AV_PIX_FMT_YUV444P10LE;
    case ADM_COLOR_YUV422_10BITS: return AV_PIX_FMT_YUV422P10LE;
    case ADM_COLOR_YUV444_12BITS: return AV_PIX_FMT_YUV444P12LE;
    case ADM_COLOR_Y8: return AV_PIX_FMT_GRAY8;
    default : ADM_assert(0); 
  }
  return AV_PIX_FMT_YUV420P;
}
/**
      \fn getStrideAndPointers
      \param dst=1 -> destination, =0 source
      \brief Fill in strides etc.. needed by libswscale
*/
uint8_t ADMColorScalerFull::getStrideAndPointers(bool dst,
        uint8_t  *from,ADM_colorspace fromColor,
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
  switch(fromColor)
  {
    case ADM_COLOR_RGB555: 
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*2);
            srcStride[1]=0;
            srcStride[2]=0;
            break;
    case ADM_COLOR_RGB24:
    case ADM_COLOR_BGR24:
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*3);
            srcStride[1]=0;
            srcStride[2]=0;
            break;
    case ADM_COLOR_GBR24P:
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
    case  ADM_COLOR_YV12:
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
    case ADM_COLOR_YUV420_10BITS:
    case ADM_COLOR_YUV420_12BITS:
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
    case ADM_COLOR_NV12:
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
    case  ADM_COLOR_YUV422:
    case  ADM_COLOR_UYVY422:        
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*2);
            srcStride[1]=0;
            srcStride[2]=0;
            break;            
    case  ADM_COLOR_YUV422P:
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
    case ADM_COLOR_RGB32A:
    case ADM_COLOR_BGR32A:
            srcData[0]=from;
            srcData[1]=NULL;
            srcData[2]=NULL;
            srcStride[0]=ADM_IMAGE_ALIGN(width*4);
            srcStride[1]=0;
            srcStride[2]=0;
            break;
    case ADM_COLOR_YUV422_10BITS:
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
    case ADM_COLOR_YUV444_12BITS:
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

    getStrideAndPointers(false,from,fromColor,srcData,srcStride);
    getStrideAndPointers(true,to,toColor,dstData,dstStride);

    if(fromColor == ADM_COLOR_YV12)
    {
        uint8_t *p=srcData[1];
        srcData[1]=srcData[2];
        srcData[2]=p;
    }
    if(toColor == ADM_COLOR_YV12)
    {
        uint8_t *p=dstData[1];
        dstData[1]=dstData[2];
        dstData[2]=p;
    }

#ifdef BGR32_IS_SWAPPED
    if(fromColor != toColor && fromColor == ADM_COLOR_BGR32A)
        swapRGB32(srcWidth,srcHeight,srcStride[0],srcData[0]);
#endif
    sws_scale(CONTEXT,srcData,srcStride,0,srcHeight,dstData,dstStride);
#ifdef BGR32_IS_SWAPPED
    if(fromColor != toColor && toColor==ADM_COLOR_BGR32A)
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
    if(fromColor != toColor && fromColor == ADM_COLOR_BGR32A)
        swapRGB32(srcWidth,srcHeight,xs[0],src[0]);
#endif
    sws_scale(CONTEXT,src,xs,0,srcHeight,dst,xd);
#ifdef BGR32_IS_SWAPPED
    if(fromColor != toColor && toColor == ADM_COLOR_BGR32A)
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

    if(fromColor==ADM_COLOR_YV12)
    {
        uint8_t *p=src[1];
        src[1]=src[2];
        src[2]=p;
    }

    if(toColor==ADM_COLOR_YV12)
    {
        uint8_t *p=dst[1];
        dst[1]=dst[2];
        dst[2]=p;
    }
    if(fromColor != toColor)
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
        if(fromColor == ADM_COLOR_BGR32A)
            swapRGB32(srcWidth,srcHeight,xs[0],src[0]);
#endif
    }else
    {
        destImage->_range = sourceImage->_range;
    }

    bool hdr_enabled = false;
    unsigned int method;
    if(prefs->get(HDR_TONEMAPPING,&method))
        hdr_enabled = (method > 0);
    if (possibleHdrContent && hdr_enabled && ((sourceImage->_hdrInfo.color_trc > ADM_HDR_TRC_DEFAULT) || (sourceImage->_hdrInfo.max_luminance > 0.0)))
    {
        updateHDR_LUT(destImage->_range, sourceImage->_hdrInfo.max_luminance);
        scaleHDR(src,xs, dst,xd);
    }
    else
    {
        sws_scale(CONTEXT,src,xs,0,srcHeight,dst,xd);
    }

#ifdef BGR32_IS_SWAPPED
    if(fromColor != toColor && toColor == ADM_COLOR_BGR32A)
        swapRGB32(dstWidth,dstHeight,xd[0],dst[0]);
#endif
    return true;
}


/**
    \fn  updateHDR_LUT
*/
void ADMColorScalerFull::updateHDR_LUT(bool extended_range,  double max_luminance)
{
    unsigned int method;
    float trgtLumF, postGainF, satF;
    if(!prefs->get(HDR_TONEMAPPING,&method))
        method = 1;
    if(!prefs->get(HDR_TARGET_LUMINANCE,&trgtLumF))
        trgtLumF = 100.0;
    if(!prefs->get(HDR_SATURATION,&satF))
        satF = 1.0;
    double trgtLum = trgtLumF;
    double sat = satF;

    if (max_luminance == 0.0)	// unspecified -> 10knits
        max_luminance = 10000.0;


    if ((method != hdrTMmethod) || (max_luminance != hdrTMsrcLum) || (trgtLum != hdrTMtrgtLum) || (sat != hdrTMsat))
    {
        int prevYSDRint = -1;
        for (int l=0; l<ADM_COLORSPACE_HDR_LUT_SIZE; l+=1)
        {
            double LumHDR = max_luminance;	// peak mastering display luminance [nit]
            double LumSDR = trgtLum;		// peak target display luminance [nit]

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
            YSDR *= std::sqrt(2)*1.1;
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

            //if (prevYSDRint != YSDRint)
            {
                prevYSDRint = YSDRint;
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
                    C *= (std::pow(YSDR,1/2.4)+1/3.)*sat;
                    if (C < -0.5)
                        C = -0.5;
                    if (C > 0.5)
                        C = 0.5;
                    C += 0.5;
                    if (extended_range)
                        hdrChromaLUT[YSDRint][l] = std::round(255.0*C);
                    else
                        hdrChromaLUT[YSDRint][l] = std::round(224.0*C) + 16;
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

        hdrTMmethod = method;
        hdrTMsrcLum = max_luminance;
        hdrTMtrgtLum = trgtLum;
        hdrTMsat = sat;
    }
}


/**
    \fn  scaleHDR
*/
void ADMColorScalerFull::scaleHDR(const uint8_t *const srcData[], const int srcStride[], uint8_t *const dstData[], const int dstStride[])
{
    uint8_t *gbrData[3];
    int gbrStride[3];
    gbrData[0] = (uint8_t*)hdrYUV;
    gbrStride[0] = ADM_IMAGE_ALIGN(dstWidth)*2;
    gbrStride[1] = gbrStride[2]= ADM_IMAGE_ALIGN(dstWidth/2)*2;
    gbrData[1] = gbrData[0] + gbrStride[0]*(dstHeight);
    gbrData[2] = gbrData[1] + gbrStride[1]*(dstHeight/2);

    sws_scale(HDRYUVCONTEXT,srcData,srcStride,0,srcHeight,gbrData,gbrStride);
    
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
            ptrU[x] = hdrChromaLUT[luma][(ADM_COLORSPACE_HDR_LUT_SIZE-1)&(hptrU[x]>>(16-ADM_COLORSPACE_HDR_LUT_WIDTH))];
            ptrV[x] = hdrChromaLUT[luma][(ADM_COLORSPACE_HDR_LUT_SIZE-1)&(hptrV[x]>>(16-ADM_COLORSPACE_HDR_LUT_WIDTH))];
            
            int Cr = ptrV[x];
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
            ADM_colorspace from,ADM_colorspace to)
{
    context=NULL;
    possibleHdrContent=false;
    hdrLumaLUT = NULL;
    for (int i=0; i<256; i++)
    {
        hdrChromaLUT[i] = NULL;
        hdrLumaCrLUT[i] = NULL;
    }
    hdrTMsrcLum = hdrTMtrgtLum = hdrTMsat = -1.0;	// invalidate
    hdrYUVContext=NULL;
    hdrYUV=NULL;
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
    delete [] hdrLumaLUT;
    for (int i=0; i<256; i++)
    {
        delete [] hdrChromaLUT[i];
        delete [] hdrLumaCrLUT[i];
    }
    if (hdrYUVContext)
    {
        sws_freeContext(HDRYUVCONTEXT);
        hdrYUVContext=NULL;
    }
    if (hdrYUV)
    {
        delete [] hdrYUV;
        hdrYUV = NULL;
    }
}
/**
    \fn reset
*/
bool  ADMColorScalerFull::reset(ADMColorScaler_algo algo, int sw, int sh, int dw,int dh,ADM_colorspace from,ADM_colorspace to)
{
    if(context) sws_freeContext(CONTEXT);
    context=NULL;
    if (hdrYUVContext)
    {
        sws_freeContext(HDRYUVCONTEXT);
        hdrYUVContext=NULL;
    }
    if (hdrYUV)
    {
        delete [] hdrYUV;
        hdrYUV = NULL;
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
    
    possibleHdrContent = (from >= ADM_COLOR_YUV444_10BITS) && (from <= ADM_COLOR_YUV444_12BITS) && (to == ADM_COLOR_IS_YUV);
    if (possibleHdrContent)
    {
        hdrYUV = new uint16_t[ADM_IMAGE_ALIGN(dw)*dh*2];
        if (hdrLumaLUT == NULL)
            hdrLumaLUT = new uint8_t[ADM_COLORSPACE_HDR_LUT_SIZE];
        for (int i=0; i<256; i++)
        {
            if (hdrChromaLUT[i] == NULL)
                hdrChromaLUT[i] = new uint8_t[ADM_COLORSPACE_HDR_LUT_SIZE];
            if (hdrLumaCrLUT[i] == NULL)
                hdrLumaCrLUT[i] = new uint8_t[256];
        }
    }
    
    srcWidth=sw;
    srcHeight=sh;

    dstWidth=dw;
    dstHeight=dh;

    fromColor=from;
    toColor=to;
    AVPixelFormat lavFrom=ADMColor2LAVColor(fromColor );
    AVPixelFormat lavTo=ADMColor2LAVColor(toColor );
    
    context=(void *)sws_getContext(
                      srcWidth,srcHeight,
                      lavFrom ,
                      dstWidth,dstHeight,
                      lavTo,
                      flags, NULL, NULL,NULL);

    if (possibleHdrContent)
    {
        hdrYUVContext=(void *)sws_getContext(
                          srcWidth,srcHeight,
                          lavFrom ,
                          dstWidth,dstHeight,
                          AV_PIX_FMT_YUV420P16LE,
                          flags, NULL, NULL,NULL);
    }
    
    return true;
}
//------------------------------
bool            ADMColorScalerSimple::changeWidthHeight(int newWidth, int newHeight)
{
    if(newWidth==srcWidth && newHeight==srcHeight) return true; // no change
    
     return reset(algo, newWidth,newHeight, newWidth,newHeight,fromColor,toColor);

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
    getStrideAndPointers(true,to,toColor, dstPlanes, dstPitch);

    if(fromColor==ADM_COLOR_YV12)
    {
        uint8_t *p=srcPlanes[1];
        srcPlanes[1]=srcPlanes[2];
        srcPlanes[2]=p;
    }
    if(toColor==ADM_COLOR_YV12)
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
//EOF
