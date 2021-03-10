/***************************************************************************
                    \fn       ADM_vidFitToSize.h  

    copyright            : (C) 2009 by mean
                               2021 szlldm

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
    \class ADMVideoFitToSize
*/
class ADMVideoFitToSize : public  ADM_coreVideoFilter
{
  protected:
    ADMColorScalerFull * resizer;
    bool                 reset(uint32_t nw, uint32_t nh,uint32_t algo, float tolerance);
    bool                 clean( void );
    ADMImage *           original;
    ADMImage *           stretch;
    ADMImage *           echo;
    ADMColorScalerFull * resizerOrigToEcho;
    ADMColorScalerFull * resizerEchoToImage;
    int                  stretchW;
    int                  stretchH;
    int                  pads[4];
    fitToSize            configuration;

  public:
    ADMVideoFitToSize(ADM_coreVideoFilter *previous,CONFcouple *conf);
    ~ADMVideoFitToSize();

    virtual const char * getConfiguration(void);                   /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual FilterInfo * getInfo(void);                             /// Return picture parameters after this filter
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;             /// Start graphical user interface

    static void          getFitParameters(int inW, int inH, int outW, int ouH, float tolerance, int * strW, int * strH, int * padLeft, int * padRight, int * padTop, int * padBottom);

};


