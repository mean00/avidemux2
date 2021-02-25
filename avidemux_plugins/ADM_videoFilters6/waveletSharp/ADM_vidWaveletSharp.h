/***************************************************************************
                          waveletSharp filter 
    Algorithm:
        Copyright 2008 Marco Rossini
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
    \class ADMVideoWaveletSharp
*/
class  ADMVideoWaveletSharp:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    waveletSharp       _param;
    float           _strength;
    float           _radius;
    bool            _highq;

    static void          WaveletSharpProcess_HatTransformHorizontal(int32_t *temp, int32_t *base, int size, int sc);
    static void          WaveletSharpProcess_HatTransformVertical(int i, int32_t *temp, int32_t *base, int st, int size, int sc);
    static void          WaveletSharpProcess_Core(int32_t *buf[4], int levels, unsigned int width, unsigned int height, double amount, double radius);
  public:
    ADMVideoWaveletSharp(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoWaveletSharp();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static void          WaveletSharpProcess_C(ADMImage *img, float strength, float radius, bool highq);
    static void          reset(waveletSharp *cfg);

  private:
    float   valueLimit(float val, float min, float max);
    int32_t valueLimit(int32_t val, int32_t min, int32_t max);

};
