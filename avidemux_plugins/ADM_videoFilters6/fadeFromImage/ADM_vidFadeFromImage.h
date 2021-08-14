/***************************************************************************
                          FadeFromImage filter
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

#include "ADM_default.h"
#include "ADM_image.h"
#include "ADM_coreVideoFilter.h"

/**
    \class ADMVideoFadeFromImage
*/
class  ADMVideoFadeFromImage:public ADM_coreVideoFilter
{
  public:
    typedef struct {
        bool                  validImg;
        ADMImage *            imgCopy;
    } fadeFromImage_buffers_t;

  protected:
    void                      update(void);
    fadeFromImage             _param;
    fadeFromImage_buffers_t   _buffers;
    

  public:
    ADMVideoFadeFromImage(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoFadeFromImage();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface

    static void FadeFromImageCreateBuffers(int w, int h, fadeFromImage_buffers_t * buffers);
    static void FadeFromImageDestroyBuffers(fadeFromImage_buffers_t * buffers);
    static void FadeFromImageProcess_C(ADMImage *img, int w, int h, fadeFromImage param, fadeFromImage_buffers_t * buffers);
};
