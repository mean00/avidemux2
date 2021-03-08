/***************************************************************************
                          Mirror filter 
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
    \class ADMVideoArtMirror
*/
class  ADMVideoArtMirror:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    artMirror       _param;
    uint32_t        _method;
    float           _displacement;
  public:
    ADMVideoArtMirror(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoArtMirror();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static void          ArtMirrorProcess_C(ADMImage *img, uint32_t method, float displacement);

  private:
    static float   valueLimit(float val, float min, float max);
    static int32_t valueLimit(int32_t val, int32_t min, int32_t max);

};
