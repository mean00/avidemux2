/***************************************************************************
                          Grid filter 
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
    \class ADMVideoArtGrid
*/
class  ADMVideoArtGrid:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    artGrid         _param;
    uint32_t        _size;
    bool            _roll;
    ADMImage        *work;
  public:
    ADMVideoArtGrid(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtGrid();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface
    virtual bool         goToTime(uint64_t usec);

    static  void         ArtGridProcess_C(ADMImage *img, ADMImage *tmp, uint32_t size, bool work);
    static  void         reset(artGrid *cfg);

  private:
    float   valueLimit(float val, float min, float max);
    int32_t valueLimit(int32_t val, int32_t min, int32_t max);
};


