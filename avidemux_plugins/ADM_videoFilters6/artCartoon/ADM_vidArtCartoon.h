/***************************************************************************
                          Cartoon filter ported from frei0r
    Algorithm:
        Copyright 2003 Dries Pruimboom <dries@irssystems.nl>
        Copyright      Denis Rojo <jaromil@dyne.org>
    Ported to Avidemux:
        Copyright 2021 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

#include "ADM_byteBuffer.h"
#include "ADM_image.h"

/**
    \class ADMVideoArtCartoon
*/
class  ADMVideoArtCartoon:public ADM_coreVideoFilter
{

  protected:
    void                  update(void);
    artCartoon            _param;
    float                 _threshold;
    uint32_t              _scatter;
    uint32_t              _color;
    int                   _rgbBufStride;
    ADM_byteBuffer *      _rgbBufRaw;
    ADMImageRef *         _rgbBufImage;
    ADMColorScalerFull *  _convertYuvToRgb;
    ADMColorScalerFull *  _convertRgbToYuv;
  public:
    ADMVideoArtCartoon(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtCartoon();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface        

    static void ArtCartoonCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv);
    static void ArtCartoonDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);
    static void ArtCartoonProcess_C(ADMImage *img, int w, int h, float threshold, uint32_t scatter, uint32_t color, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);

  private:
    static int GMError(int err, uint8_t * p1, uint8_t * p2);
};
