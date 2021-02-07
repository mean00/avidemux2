/***************************************************************************
                          ChromaHold filter 
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
    \class ADMVideoArtChromaHold
*/
class  ADMVideoArtChromaHold:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    artChromaHold   _param;
    bool            _cen[3];
    float           _cu[3];
    float           _cv[3];
    float           _cdist[3];
    float           _cslope[3];

  public:
    ADMVideoArtChromaHold(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtChromaHold();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static void          ArtChromaHoldProcess_C(ADMImage *img, bool * cen, float * cu, float * cv, float * cdist, float * cslope);

  private:
    static float   valueLimit(float val, float min, float max);
    static int32_t valueLimit(int32_t val, int32_t min, int32_t max);

};
