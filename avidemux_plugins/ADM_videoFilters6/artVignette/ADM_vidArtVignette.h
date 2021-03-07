/***************************************************************************
                          Vignette filter 
    Algorithm:
        Copyright (C) 2011 Simon Andreas Eugster (simon.eu@gmail.com)
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

/**
    \class ADMVideoArtVignette
*/
class ADMVideoArtVignette:public ADM_coreVideoFilter
{
  protected:
            void        update(void);
            artVignette _param;
            float       _aspect;
            float       _center;
            float       _soft;
            float *     _filterMask;
  public:
                        ADMVideoArtVignette(ADM_coreVideoFilter *in,CONFcouple *couples);
                        ~ADMVideoArtVignette();

    virtual const char  *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool        getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool        getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void        setCoupledConf(CONFcouple *couples);
    virtual bool        configure(void) ;                 /// Start graphical user interface

    static  void        ArtVignetteCreateMask(float *mask, int w, int h, float aspect, float center, float soft);
    static  void        ArtVignetteProcess_C(ADMImage *img, float *mask);
    static  void        reset(artVignette *cfg);
};

