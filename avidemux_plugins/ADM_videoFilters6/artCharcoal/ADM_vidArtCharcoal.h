/***************************************************************************
                          Charcoal filter 
    Algorithm:
        Copyright (C) 2003-2020 Meltytech, LLC
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
    \class ADMVideoArtCharcoal
*/
class  ADMVideoArtCharcoal:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    artCharcoal     _param;
    int32_t         _scatterX;
    int32_t         _scatterY;
    float           _intensity;
    float           _color;
    bool            _invert;
    ADMImage        *work;
  public:
    ADMVideoArtCharcoal(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtCharcoal();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static  void         ArtCharcoalProcess_C(ADMImage *img, ADMImage *tmp, int32_t scatterX, int32_t scatterY, float intensity, float color, bool invert);
    static  void         reset(artCharcoal *cfg);

  private:
    static  float         ArtCharcoalProcess_Sqrti( int n );
    float   valueLimit(float val, float min, float max);
    int32_t valueLimit(int32_t val, int32_t min, int32_t max);
};


