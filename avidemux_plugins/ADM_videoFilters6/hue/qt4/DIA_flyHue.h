/***************************************************************************
  \fn     DIA_flyHue
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for hue & sat
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_HUE_H
#define FLY_HUE_H
#include "ADM_vidHue.h"
/**
    \class flyHue
*/

class flyHue : public ADM_flyDialogYuv
{
  private:
    huesettings flyset;
  public:

    uint8_t     processYuv(ADMImage* in, ADMImage *out);
    uint8_t     download(void);
    uint8_t     upload(void);
    uint8_t     update(void);
    uint8_t     reset(void);
                flyHue(QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                     ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {}
    void        setTabOrder(void);
    void        setParam(hue *par) { flyset.param.hue = par->hue; flyset.param.saturation = par->saturation; }
    void        getParam(hue *par) { par->hue = flyset.param.hue; par->saturation = flyset.param.saturation; }
};
#endif
