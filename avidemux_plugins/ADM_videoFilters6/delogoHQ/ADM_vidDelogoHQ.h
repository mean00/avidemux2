/***************************************************************************
                          DelogoHQ filter
    Algorithm:
        Copyright 2021 szlldm
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
    \class ADMVideoDelogoHQ
*/
class  ADMVideoDelogoHQ:public ADM_coreVideoFilter
{

  protected:
    void                  update(void);
    delogoHQ              _param;
    int *                 _mask;
    int                   _maskHint[4];
    uint32_t              _blur;
    uint32_t              _gradient;
    int                   _rgbBufStride;
    ADM_byteBuffer *      _rgbBufRaw;
    ADMImageRef *         _rgbBufImage;
    ADMColorScalerFull *  _convertYuvToRgb;
    ADMColorScalerFull *  _convertRgbToYuv;
  public:
    ADMVideoDelogoHQ(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoDelogoHQ();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface        

    static void DelogoHQCreateBuffers(int w, int h, int * rgbBufStride, ADM_byteBuffer ** rgbBufRaw, ADMImageRef ** rgbBufImage, ADMColorScalerFull ** convertYuvToRgb, ADMColorScalerFull ** convertRgbToYuv);
    static void DelogoHQDestroyBuffers(ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);
    static void BoxBlurLine_C(uint8_t * line, int len, int pixPitch, uint32_t * stack, unsigned int radius);
    static void DelogoHQProcess_C(ADMImage *img, int w, int h, int * mask, int * maskHint, uint32_t blur, uint32_t gradient, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);
    static void DelogoHQPrepareMask_C(int * mask, int * maskHint, int w, int h, ADMImage * maskImage);

  private:
    bool reloadImage(void);
};
