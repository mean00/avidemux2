/***************************************************************************
                          lumaStab filter 
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
    \class ADMVideoLumaStab
*/
class  ADMVideoLumaStab:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    lumaStab       _param;
    uint32_t       _filterLength;
    float          _cbratio;
    float          _sceneThreshold;
    bool           _chroma;
    float *        _yHyst;
    int            _yHystlen;
    float          _prevChromaHist[128];
  public:
    ADMVideoLumaStab(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoLumaStab();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static void          LumaStabProcess_C(ADMImage *img, uint32_t filterLength, float cbratio, float sceneThreshold, bool chroma, float * yHyst, int * yHystlen, float * prevChromaHist, bool * newScene, float * sceneDiff);
    static void          reset(lumaStab *cfg);

  private:
    float   valueLimit(float val, float min, float max);
    int32_t valueLimit(int32_t val, int32_t min, int32_t max);

};
