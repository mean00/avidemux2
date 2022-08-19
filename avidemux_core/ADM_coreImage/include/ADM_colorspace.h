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
#include "ADM_toneMapper.h"

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
    ADM_CS_FAST_BILINEAR,
    ADM_CS_POINT
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


#include "ADM_threads.h"

/**
    \class ADMColorScaler
*/
class ADM_COREIMAGE6_EXPORT ADMRGB32Scaler
{
  protected:
    void                *context[3];
    ADMColorScaler_algo algo;
    uint32_t            srcWidth,srcHeight;
    uint32_t            dstWidth,dstHeight;
    pthread_t           worker_threads[3];
    uint8_t             *inputPlanes[3];
    uint8_t             *outputPlanes[3];
    typedef struct {
        void            *context;
        uint8_t         *sourceData;
        uint8_t         *destData;
        uint8_t         *iPlane;
        uint8_t         *oPlane;
        uint32_t        srcWidth,srcHeight;
        uint32_t        dstWidth,dstHeight;
    } worker_thread_arg;
    worker_thread_arg   worker_thread_args[3];
    
    static void *       planeWorker(void *argptr);
    void                cleanUp();
  public :
    
                    ADMRGB32Scaler(ADMColorScaler_algo algo, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    bool            reset(ADMColorScaler_algo, int sw, int sh, int dw,int dh,ADM_pixelFormat from,ADM_pixelFormat to);
    bool            convert(uint8_t *sourceData, uint8_t *destData);
                    ~ADMRGB32Scaler();
};

#endif
//EOF

