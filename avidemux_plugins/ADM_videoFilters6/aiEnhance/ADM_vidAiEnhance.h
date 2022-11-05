/***************************************************************************
                          AiEnhance filter
        Copyright 2022 szlldm
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
#include "ADM_threads.h"
#include "ADM_byteBuffer.h"
#include "ADM_image.h"

#include "NeuronSW.h"

/**
    \class ADMVideoAiEnhance
*/
class  ADMVideoAiEnhance:public ADM_coreVideoFilter
{
  public:
    typedef struct {
        unsigned int          w,h;
        int                   algo;
        NeuronSW *            ai;
        ADMImage *            targetImg;
        ADMImage *            previewImg;
        ADMColorScalerFull *  upScaler;
        ADMColorScalerFull *  previewScaler;
    } aiEnhance_buffers_t;

  protected:
    void                  update(void);
    aiEnhance             _param;
    aiEnhance_buffers_t   _buffers;
    ADMImage *            inputImg;
    
  public:
    ADMVideoAiEnhance(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoAiEnhance();

    virtual const char    *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool          getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool          getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void          setCoupledConf(CONFcouple *couples);
    virtual bool          configure(void) ;                 /// Start graphical user interface

    static void AiEnhanceInitializeBuffers(int w, int h, aiEnhance_buffers_t * buffers);
    static void AiEnhanceDestroyBuffers(aiEnhance_buffers_t * buffers);
    static void AiEnhanceProcess_C(ADMImage *srcImg, ADMImage *dstImg, bool previewMode, bool skipProcess, aiEnhance param, aiEnhance_buffers_t * buffers);
    static int  getScaling(int algo);
};
