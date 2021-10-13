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
#ifndef ADM_TONEMAPPER_H
#define ADM_TONEMAPPER_H

#include "ADM_coreImage6_export.h"
#include "ADM_rgb.h" // To have colors

class ADMImage;
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
    ADM_pixelFormat fromPixFrmt,toPixFrmt;
    int             sws_flag;
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
                    ADMToneMapper(int sws_flag, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    bool            toneMap(ADMImage *sourceImage, ADMImage *destImage, unsigned int toneMappingMethod, double targetLuminance, double saturationAdjust);
    bool            toneMap_fastYUV(ADMImage *sourceImage, ADMImage *destImage, double targetLuminance, double saturationAdjust);
    bool            toneMap_RGB(ADMImage *sourceImage, ADMImage *destImage, unsigned int method, double targetLuminance, double saturationAdjust);
                    ~ADMToneMapper();
};

#endif
//EOF

