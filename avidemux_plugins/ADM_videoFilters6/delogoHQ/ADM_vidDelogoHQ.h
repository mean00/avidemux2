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
    int                   _plYuvStride;
    uint16_t *            _plYuvBuf;
    uint16_t *            _toLinLut;
    uint8_t *             _toLumaLut;
  public:
    ADMVideoDelogoHQ(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoDelogoHQ();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface        

    static void DelogoHQCreateBuffers(int w, int h, int * plYuvStride, uint16_t ** plYuvBuf, uint16_t ** toLinLut, uint8_t ** toLumaLut);
    static void DelogoHQDestroyBuffers(uint16_t * plYuvBuf, uint16_t * toLinLut, uint8_t * toLumaLut);
    static void BoxBlurLine_C(uint16_t * line, int len, int pixPitch, uint64_t * stack, unsigned int radius);
    static void DelogoHQProcess_C(ADMImage *img, int w, int h, int * mask, int * maskHint, uint32_t blur, uint32_t gradient, int plYuvStride, uint16_t * plYuvBuf, uint16_t * toLinLut, uint8_t * toLumaLut);
    static void DelogoHQPrepareMask_C(int * mask, int * maskHint, int w, int h, ADMImage * maskImage);

  private:
    bool reloadImage(void);
};
