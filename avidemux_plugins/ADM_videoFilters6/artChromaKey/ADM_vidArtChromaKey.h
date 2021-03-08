/***************************************************************************
                          ChromaKey filter 
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
    \class ADMVideoArtChromaKey
*/
class  ADMVideoArtChromaKey:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    ADMImage *      _backgrnd;
    artChromaKey    _param;
    bool            _cen[3];
    float           _cu[3];
    float           _cv[3];
    float           _cdist[3];
    float           _cslope[3];
    uint32_t        _spill;
  public:
    ADMVideoArtChromaKey(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtChromaKey();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static void          ArtChromaKeyProcess_C(ADMImage *img, ADMImage *backgrnd, bool * cen, float * cu, float * cv, float * cdist, float * cslope, uint32_t _spill);

  private:
    static float   valueLimit(float val, float min, float max);
    static int32_t valueLimit(int32_t val, int32_t min, int32_t max);
    bool           reloadImage(void);

};
