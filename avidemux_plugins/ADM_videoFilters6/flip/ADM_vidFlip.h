/***************************************************************************
                          Flip filter
    Algorithm:
        Copyright 2009 by mean
        Copyright 2011 by mean
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
    \class ADMVideoFlip
*/
class  ADMVideoFlip:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    uint8_t        *_scratch;
    flip            _param;
    uint32_t        _flipdir;
  public:
     ADMVideoFlip(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoFlip();

    virtual const char  *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static  void         FlipProcess_C(ADMImage *img, uint8_t * scratch, uint32_t flipdir);

};


