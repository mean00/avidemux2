/***************************************************************************
  \fn     DIA_flyWaveletSharp
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for waveletSharp
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_WaveletSharp_H
#define FLY_WaveletSharp_H
#include "waveletSharp.h"
/**
    \class flyWaveletSharp
*/
class flyWaveletSharp : public ADM_flyDialogYuv
{

  public:
    waveletSharp  param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
    flyWaveletSharp (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {};
};
#endif
