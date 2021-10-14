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

#define BGR32_IS_SWAPPED 1

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




typedef enum
{
    ADM_COL_RANGE_MPEG,
    ADM_COL_RANGE_JPEG
}ADM_colorRange;

typedef enum
{
    ADM_COL_PRI_UNSPECIFIED = 0,
    ADM_COL_PRI_BT709,        ///< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
    ADM_COL_PRI_BT470M,       ///< also FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    ADM_COL_PRI_BT470BG,      ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
    ADM_COL_PRI_SMPTE170M,    ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    ADM_COL_PRI_SMPTE240M,    ///< functionally identical to above
    ADM_COL_PRI_FILM,         ///< colour filters using Illuminant C
    ADM_COL_PRI_BT2020,       ///< ITU-R BT2020
    ADM_COL_PRI_SMPTE428,     ///< SMPTE ST 428-1 (CIE 1931 XYZ)
    ADM_COL_PRI_SMPTE431,     ///< SMPTE ST 431-2 (2011) / DCI P3
    ADM_COL_PRI_SMPTE432,     ///< SMPTE ST 432-1 (2010) / P3 D65 / Display P3
    ADM_COL_PRI_EBU3213,      ///< EBU Tech. 3213-E / JEDEC P22 phosphors
    //ADM_COL_PRI_JEDEC_P22 = ADM_COL_PRI_EBU3213
}ADM_colorPrimaries;

typedef enum
{
    ADM_COL_TRC_UNSPECIFIED  = 0,
    ADM_COL_TRC_BT709,            ///< also ITU-R BT1361
    ADM_COL_TRC_GAMMA22,          ///< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
    ADM_COL_TRC_GAMMA28,          ///< also ITU-R BT470BG
    ADM_COL_TRC_SMPTE170M,        ///< also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC
    ADM_COL_TRC_SMPTE240M,
    ADM_COL_TRC_LINEAR,           ///< "Linear transfer characteristics"
    ADM_COL_TRC_LOG,              ///< "Logarithmic transfer characteristic (100:1 range)"
    ADM_COL_TRC_LOG_SQRT,         ///< "Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)"
    ADM_COL_TRC_IEC61966_2_4,     ///< IEC 61966-2-4
    ADM_COL_TRC_BT1361_ECG,       ///< ITU-R BT1361 Extended Colour Gamut
    ADM_COL_TRC_IEC61966_2_1,     ///< IEC 61966-2-1 (sRGB or sYCC)
    ADM_COL_TRC_BT2020_10,        ///< ITU-R BT2020 for 10-bit system
    ADM_COL_TRC_BT2020_12,        ///< ITU-R BT2020 for 12-bit system
    ADM_COL_TRC_SMPTE2084,        ///< SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems
    //ADM_COL_TRC_SMPTEST2084 = ADM_COL_TRC_SMPTE2084,
    ADM_COL_TRC_SMPTE428,         ///< SMPTE ST 428-1
    //ADM_COL_TRC_SMPTEST428_1 = ADM_COL_TRC_SMPTE428,
    ADM_COL_TRC_ARIB_STD_B67      ///< ARIB STD-B67, known as "Hybrid log-gamma"
}ADM_colorTrC;

typedef enum
{
    ADM_COL_SPC_UNSPECIFIED = 0,
    ADM_COL_SPC_sRGB,                 ///< order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB)
    ADM_COL_SPC_BT709,                ///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
    ADM_COL_SPC_FCC,                  ///< FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    ADM_COL_SPC_BT470BG,              ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
    ADM_COL_SPC_SMPTE170M,            ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    ADM_COL_SPC_SMPTE240M,            ///< functionally identical to above
    ADM_COL_SPC_YCGCO,                ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
    //ADM_COL_SPC_YCOCG = ADM_COL_SPC_YCGCO,
    ADM_COL_SPC_BT2020_NCL,           ///< ITU-R BT2020 non-constant luminance system
    ADM_COL_SPC_BT2020_CL,            ///< ITU-R BT2020 constant luminance system
    ADM_COL_SPC_SMPTE2085,            ///< SMPTE 2085, Y'D'zD'x
    ADM_COL_SPC_CHROMA_DERIVED_NCL,   ///< Chromaticity-derived non-constant luminance system
    ADM_COL_SPC_CHROMA_DERIVED_CL,    ///< Chromaticity-derived constant luminance system
    ADM_COL_SPC_ICTCP                 ///< ITU-R BT.2100-0, ICtCp    
}ADM_colorSpace;



#endif
