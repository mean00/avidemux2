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
#include "ADM_rgb.h" // To have colors

typedef enum 
{
    ADM_CS_BILINEAR,
    ADM_CS_BICUBIC,
    ADM_CS_LANZCOS
}ADMColorSpace_algo;
/**
    \class ADMColorSpace
*/
class ADMColorSpaceFull
{
  protected:
    void            *context;
    uint32_t        srcWidth,srcHeight;
    uint32_t        dstWidth,dstHeight;
    ADM_colorspace  fromColor,toColor;
    ADMColorSpace_algo algo;
    uint8_t         getStrideAndPointers(bool dst,uint8_t  *from,ADM_colorspace fromColor,
                                            uint8_t **srcData,int *srcStride);
  public :
    
                    ADMColorSpaceFull(ADMColorSpace_algo algo, uint32_t sw, uint32_t sh, uint32_t dw,uint32_t dh,ADM_colorspace from,ADM_colorspace to);
    bool            reset(ADMColorSpace_algo, uint32_t sw, uint32_t sh, uint32_t dw,uint32_t dh,ADM_colorspace from,ADM_colorspace to);
    

    bool            convert(uint8_t  *from, uint8_t *to);
    bool            convertPlanes(uint32_t  sourceStride[3],uint32_t destStride[3],     
                                  uint8_t   *sourceData[3], uint8_t *destData[3]);
                    ~ADMColorSpaceFull();
};
/**
    \class ADMColorSpaceSimple
    \brief Same as Full but target & source width/height are the same
*/
class ADMColorSpaceSimple :public ADMColorSpaceFull
{
public:
    bool            changeWidthHeight(uint32_t newWidth, uint32_t newHeight);
                    ADMColorSpaceSimple( uint32_t width, uint32_t height, ADM_colorspace from,ADM_colorspace to,ADMColorSpace_algo algo=ADM_CS_BICUBIC):
                        ADMColorSpaceFull(algo, width, height, width,height, from, to)
                     {

                     }
};

// Some misc functions
bool ADM_ConvertRgb24ToYV12(bool inverted,uint32_t w, uint32_t h, uint8_t *source, uint8_t *destination);
#endif
//EOF

