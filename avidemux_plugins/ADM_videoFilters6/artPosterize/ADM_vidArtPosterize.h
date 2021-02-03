/***************************************************************************
                          Posterize filter ported from frei0r
    Algorithm:
        Copyright 2006 Jerry Huxtable
        Copyright 2012 Janne Liljeblad
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
    \class ADMVideoArtPosterize
*/
class  ADMVideoArtPosterize:public ADM_coreVideoFilter
{

  protected:
    void                  update(void);
    artPosterize        _param;
    uint32_t              _levels;
    int                   _rgbBufStride;
    ADM_byteBuffer *      _rgbBufRaw;
    ADMImageRef *         _rgbBufImage;
    ADMColorScalerFull *  _convertYuvToRgb;
    ADMColorScalerFull *  _convertRgbToYuv;
  public:
    ADMVideoArtPosterize(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtPosterize();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface        

    static void ArtPosterizeCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv);
    static void ArtPosterizeDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);
    static void ArtPosterizeProcess_C(ADMImage *img, int w, int h, unsigned int levels, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);

};
