/***************************************************************************
                         ADM_Rgb : wrapper for yv12->RGB display
                            using mplayer postproc


    begin                : Thu Apr 16 2003
    copyright            : (C) 2002 by mean
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
 #ifndef ADM_RGB_H
 #define ADM_RGB_H

extern "C" {
#include "libavutil/pixfmt.h"
}
#include "ADM_default.h"

#define ADM_PIXFRMT_IS_YUV 0x1000
 typedef enum 
 {
    ADM_PIXFRMT_INVALID = -1,
    ADM_PIXFRMT_RGB24 = 0,
    ADM_PIXFRMT_BGR24,
    ADM_PIXFRMT_GBR24P,
    ADM_PIXFRMT_BGR32A,
    ADM_PIXFRMT_RGB32A,
    ADM_PIXFRMT_RGB16,
    ADM_PIXFRMT_RGB555,
    ADM_PIXFRMT_BGR555,
    ADM_PIXFRMT_YV12 = ADM_PIXFRMT_IS_YUV,	// YUV420
    ADM_PIXFRMT_NV12,
    ADM_PIXFRMT_YUV422,
    ADM_PIXFRMT_UYVY422,
    ADM_PIXFRMT_YUV422P,
    ADM_PIXFRMT_YUV411,
    ADM_PIXFRMT_YUV444,
    ADM_PIXFRMT_VDPAU,
    ADM_PIXFRMT_XVBA,
    ADM_PIXFRMT_LIBVA,
    ADM_PIXFRMT_Y8,
    ADM_PIXFRMT_YUV444_10BITS,
    ADM_PIXFRMT_NV12_10BITS,
    ADM_PIXFRMT_YUV420_10BITS,
    ADM_PIXFRMT_YUV420_12BITS,
    ADM_PIXFRMT_YUV422_10BITS,
    ADM_PIXFRMT_YUV444_12BITS
 } ADM_pixelFormat;
#define ADM_PIXFRMT_BACKWARD 0x8000
#define ADM_PIXFRMT_MASK     0x7FFF


/**
    \fn ADMPixFrmt2LAVPixFmt
    \brief Convert ADM colorspace type swscale/lavcodec colorspace name

*/
static AVPixelFormat ADMPixFrmt2LAVPixFmt(ADM_pixelFormat fromPixFrmt_)
{
  ADM_pixelFormat fromPixFrmt=fromPixFrmt_;
  int intPixFrmt=(int)fromPixFrmt;
  intPixFrmt&=ADM_PIXFRMT_MASK;
  fromPixFrmt=(ADM_pixelFormat)intPixFrmt;
  switch(fromPixFrmt)
  {
    case ADM_PIXFRMT_YUV444: return AV_PIX_FMT_YUV444P;
    case ADM_PIXFRMT_YUV411: return AV_PIX_FMT_YUV411P;
    case ADM_PIXFRMT_YUV422: return AV_PIX_FMT_YUYV422;
    case ADM_PIXFRMT_UYVY422: return AV_PIX_FMT_UYVY422;
    case ADM_PIXFRMT_YV12: return AV_PIX_FMT_YUV420P;
    case ADM_PIXFRMT_NV12: return AV_PIX_FMT_NV12;
    case ADM_PIXFRMT_YUV422P: return AV_PIX_FMT_YUV422P;
    case ADM_PIXFRMT_RGB555: return AV_PIX_FMT_RGB555LE;
    case ADM_PIXFRMT_BGR555: return AV_PIX_FMT_BGR555LE;
    case ADM_PIXFRMT_RGB32A: return AV_PIX_FMT_RGBA;
#ifdef BGR32_IS_SWAPPED
    case ADM_PIXFRMT_BGR32A: return AV_PIX_FMT_RGBA; // Faster that way...
#else
    case ADM_PIXFRMT_BGR32A: return AV_PIX_FMT_BGRA;
#endif
    case ADM_PIXFRMT_RGB24: return AV_PIX_FMT_RGB24;
    case ADM_PIXFRMT_BGR24: return AV_PIX_FMT_BGR24;
    case ADM_PIXFRMT_GBR24P: return AV_PIX_FMT_GBRP;
    case ADM_PIXFRMT_YUV420_10BITS: return AV_PIX_FMT_YUV420P10LE;
    case ADM_PIXFRMT_YUV420_12BITS: return AV_PIX_FMT_YUV420P12LE;
    case ADM_PIXFRMT_NV12_10BITS:  return AV_PIX_FMT_P010LE;
    case ADM_PIXFRMT_YUV444_10BITS: return AV_PIX_FMT_YUV444P10LE;
    case ADM_PIXFRMT_YUV422_10BITS: return AV_PIX_FMT_YUV422P10LE;
    case ADM_PIXFRMT_YUV444_12BITS: return AV_PIX_FMT_YUV444P12LE;
    case ADM_PIXFRMT_Y8: return AV_PIX_FMT_GRAY8;
    default : ADM_assert(0); 
  }
  return AV_PIX_FMT_YUV420P;
}

#endif
