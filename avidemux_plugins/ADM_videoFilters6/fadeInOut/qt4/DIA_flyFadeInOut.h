/***************************************************************************
  \fn     DIA_flyFadeInOut
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for fadeInOut
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DIA_FLYFadeInOut_H
#define DIA_FLYFadeInOut_H
#include "fadeInOut.h"
#include "ADM_vidFadeInOut.h"
#include "DIA_flyDialogQt4.h"
/**
    \class flyFadeInOut
*/
class flyFadeInOut : public ADM_flyDialogYuv
{
  public:
    fadeInOut  param;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyFadeInOut (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {};
};
#endif
