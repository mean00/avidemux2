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
#ifndef ADM_COLORSPACE_H
#define ADM_COLORSPACE_H

#include "ADM_coreImage6_export.h"
#include "ADM_rgb.h" // To have colors

class ADMImage;
typedef enum 
{
    ADM_CS_BILINEAR,
    ADM_CS_BICUBIC,
    ADM_CS_LANCZOS,
    ADM_CS_BICUBLIN,
    ADM_CS_GAUSS,
    ADM_CS_SINC,
    ADM_CS_SPLINE,
    ADM_CS_FAST_BILINEAR
}ADMColorScaler_algo;
/**
    \class ADMColorScaler
*/
class ADM_COREIMAGE6_EXPORT ADMColorScalerFull
{
  protected:
    void            *context;
    uint32_t        srcWidth,srcHeight;
    uint32_t        dstWidth,dstHeight;
    ADM_colorspace  fromColor,toColor;
    ADMColorScaler_algo algo;
    uint8_t         getStrideAndPointers(bool dst,uint8_t  *from,ADM_colorspace fromColor,
                                            uint8_t **srcData,int *srcStride);
  public :
    
                    ADMColorScalerFull(ADMColorScaler_algo algo, int sw, int sh, int dw,int dh,ADM_colorspace from,ADM_colorspace to);
    bool            reset(ADMColorScaler_algo, int sw, int sh, int dw,int dh,ADM_colorspace from,ADM_colorspace to);
    

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
                    ADMColorScalerSimple( int width, int height, ADM_colorspace from,ADM_colorspace to,ADMColorScaler_algo algo=ADM_CS_BICUBIC):
                        ADMColorScalerFull(algo, width, height, width,height, from, to)
                     {

                     }
                    
};

// Some misc functions
ADM_COREIMAGE6_EXPORT bool ADM_ConvertRgb24ToYV12(bool inverted,int w, int h, uint8_t *source, uint8_t *destination);
#endif
//EOF

