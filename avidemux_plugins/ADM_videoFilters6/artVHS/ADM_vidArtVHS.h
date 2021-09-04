/***************************************************************************
                          VHS filter 
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

/**
    \class ADMVideoArtVHS
*/
class  ADMVideoArtVHS:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    artVHS          _param;
    float           _lumaBW;
    float           _chromaBW;
    bool            _lumaNoDelay;
    bool            _chromaNoDelay;
    float           _unSync;
    float           _unSyncFilter;
    float           _noise;
    int *           _noiseBuffer4k;
  public:
    ADMVideoArtVHS(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtVHS();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static  void         ArtVHSProcess_C(ADMImage *img, float lumaBW, float chromaBW, float unSync, float unSyncFilter, bool lumaNoDelay, bool chromaNoDelay, float noise, int * noiseBuffer4k);
    static  void         reset(artVHS *cfg);

  private:
    float valueLimit(float val, float min, float max);
};

