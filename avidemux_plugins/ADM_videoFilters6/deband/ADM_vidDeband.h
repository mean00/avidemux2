/***************************************************************************
                          Deband filter 
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
    \class ADMVideoDeband
*/
class  ADMVideoDeband:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    deband         _param;
    uint32_t        _range,_lthresh,_ctresh;
    ADMImage        *work;
  public:
    ADMVideoDeband(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoDeband();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static  void         DebandProcess_C(ADMImage *img, ADMImage *tmp, uint32_t range, uint32_t lthresh, uint32_t cthresh);
    static  void         reset(deband *cfg);

  private:
    uint32_t valueLimit(uint32_t val, uint32_t min, uint32_t max);
};


