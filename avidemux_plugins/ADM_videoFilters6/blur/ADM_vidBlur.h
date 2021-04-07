/***************************************************************************
                          Blur filter ported from frei0r
    Algorithm:
        Copyright Mario Klingemann
        Copyright Maxim Shemanarev
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
    \class ADMVideoBlur
*/
class  ADMVideoBlur:public ADM_coreVideoFilter
{

  protected:
    void                  update(void);
    blur                  _param;
    int                   _rgbBufStride;
    ADM_byteBuffer *      _rgbBufRaw;
    ADMImageRef *         _rgbBufImage;
    ADMColorScalerFull *  _convertYuvToRgb;
    ADMColorScalerFull *  _convertRgbToYuv;
  public:
    ADMVideoBlur(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoBlur();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface        

    static void BlurCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv);
    static void BlurDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);
    static void BlurProcess_C(ADMImage *img, int w, int h, blur param, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);

  private:
    static void BoxBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius);
    static void StackBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius);

};
