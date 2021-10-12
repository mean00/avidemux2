/***************************************************************************
                       
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
    copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_COLORSPACE_H
#define ADM_COLORSPACE_H

#include "ADM_coreImage6_export.h"
#include "ADM_rgb.h" // To have colors

class ADMImage;
class ADMToneMapper;
typedef enum 
{
    ADM_CS_BILINEAR,
    ADM_CS_BICUBIC,
    ADM_CS_LANCZOS,
    ADM_CS_BICUBLIN,
    ADM_CS_GAUSS,
    ADM_CS_SINC,
    ADM_CS_SPLINE,
    ADM_CS_FAST_BILINEAR,
    ADM_CS_POINT
}ADMColorScaler_algo;

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

/**
    \class ADMColorScaler
*/
class ADM_COREIMAGE6_EXPORT ADMColorScalerFull
{
  protected:
    void            *context;
    uint32_t        srcWidth,srcHeight;
    uint32_t        dstWidth,dstHeight;
    ADM_pixelFormat  fromPixFrmt,toPixFrmt;
    ADMColorScaler_algo algo;
    uint8_t         getStrideAndPointers(bool dst,uint8_t  *from,ADM_pixelFormat fromPixFrmt,
                                            uint8_t **srcData,int *srcStride);
    bool            possibleHdrContent;
    ADMToneMapper * toneMapper;
  public :
    
                    ADMColorScalerFull(ADMColorScaler_algo algo, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    bool            reset(ADMColorScaler_algo, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    

    bool            convert(uint8_t  *from, uint8_t *to);
    bool            convertImage(ADMImage *img, uint8_t *to);
    bool            convertImage(ADMImage *sourceImage, ADMImage *destImage);
    bool            convertPlanes(int  sourceStride[3],int destStride[3],     
                                  uint8_t   *sourceData[3], uint8_t *destData[3]);
                    ~ADMColorScalerFull();
};
/**
    \class ADMColorScalerSimple
    \brief Same as Full but target & source width/height are the same
*/
class ADMColorScalerSimple :public ADMColorScalerFull
{
public:
    bool            changeWidthHeight(int newWidth, int newHeight);
                    ADMColorScalerSimple( int width, int height, ADM_pixelFormat from,ADM_pixelFormat to,ADMColorScaler_algo algo=ADM_CS_BICUBIC):
                        ADMColorScalerFull(algo, width, height, width,height, from, to)
                     {

                     }
                    
};


/**
    \class ADMToneMapper
*/
class ADM_COREIMAGE6_EXPORT ADMToneMapper
{
  protected:
    void            *contextYUV;
    void            *contextRGB1, *contextRGB2;
    uint32_t        srcWidth,srcHeight;
    uint32_t        dstWidth,dstHeight;
    ADM_pixelFormat  fromPixFrmt,toPixFrmt;
    ADMColorScaler_algo algo;
    #define ADM_COLORSPACE_HDR_LUT_WIDTH (12)	// bits
    #define ADM_COLORSPACE_HDR_LUT_SIZE	(1<<ADM_COLORSPACE_HDR_LUT_WIDTH)
    uint8_t         *hdrLumaLUT;
    uint8_t         *hdrChromaBLUT[256];
    uint8_t         *hdrChromaRLUT[256];
    uint8_t         *hdrLumaCrLUT[256];
    uint8_t         *hdrRGBLUT;
    double          hdrTMsrcLum, hdrTMtrgtLum, hdrTMsat;
    unsigned int    hdrTMmethod;
    uint16_t        *hdrYUV;
    uint16_t        *hdrRGB[3];
    uint8_t         *sdrRGB[3];
    uint8_t         sdrRGBSat[256];
  public :
                    ADMToneMapper(ADMColorScaler_algo algo, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    bool            toneMap(ADMImage *sourceImage, ADMImage *destImage, unsigned int toneMappingMethod, double targetLuminance, double saturationAdjust);
    bool            toneMap_fastYUV(ADMImage *sourceImage, ADMImage *destImage, double targetLuminance, double saturationAdjust);
    bool            toneMap_RGB(ADMImage *sourceImage, ADMImage *destImage, unsigned int method, double targetLuminance, double saturationAdjust);
                    ~ADMToneMapper();
};

#endif
//EOF

