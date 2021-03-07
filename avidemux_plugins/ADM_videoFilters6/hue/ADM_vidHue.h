/***************************************************************************
                          Hue/Saturation filter ported from mplayer 
 (c) Michael Niedermayer
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
#include "hue.h"

typedef struct {
    uint8_t lutU[256][256];
    uint8_t lutV[256][256];
    hue param;
} huesettings;

/**
    \class ADMVideoHue
*/
class ADMVideoHue : public ADM_coreVideoFilterCached
{
  protected:
        huesettings _settings;

  public:
                    ADMVideoHue(ADM_coreVideoFilter *in, CONFcouple *couples);
                    ~ADMVideoHue();

virtual const char *getConfiguration(void); /// Return  current configuration as a human readable string
virtual bool        getNextFrame(uint32_t *fn,ADMImage *image); /// Return the next image
virtual bool        getCoupledConf(CONFcouple **couples); /// Return the current filter configuration
virtual void        setCoupledConf(CONFcouple *couples);
virtual bool        configure(void); /// Start graphical user interface

        static void update(huesettings *s);
        static void reset(hue *h) { h->hue = 0.0; h->saturation = 1.0; }
        static void HueProcess_C(uint8_t *udst, uint8_t *vdst, uint8_t *usrc, uint8_t *vsrc,
                                 int dststride, int srcstride, int w, int h, huesettings *s);
};

