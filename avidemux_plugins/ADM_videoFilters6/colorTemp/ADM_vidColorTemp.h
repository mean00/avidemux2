/***************************************************************************
                          colorTemp filter 
    Algorithm:
        Copyright (C) 1999 Winston Chang
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
    \class ADMVideoColorTemp
*/
class  ADMVideoColorTemp:public ADM_coreVideoFilter
{

  protected:
    void            update(void);
    colorTemp       _param;
    float           _temperature;
    float           _angle;
  public:
    ADMVideoColorTemp(ADM_coreVideoFilter *in,CONFcouple *couples);
    ~ADMVideoColorTemp();

    virtual const char   *getConfiguration(void);          /// Return  current configuration as a human readable string
    virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
    virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
    virtual void         setCoupledConf(CONFcouple *couples);
    virtual bool         configure(void) ;                 /// Start graphical user interface

    static void          ColorTempProcess_C(ADMImage *img, float temperature, float angle);
    static void          reset(colorTemp *cfg);

  private:
    float   valueLimit(float val, float min, float max);
    int32_t valueLimit(int32_t val, int32_t min, int32_t max);

};
