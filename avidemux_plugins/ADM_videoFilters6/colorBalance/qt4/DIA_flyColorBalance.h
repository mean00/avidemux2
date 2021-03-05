/***************************************************************************
  \fn     DIA_flyColorBalance
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for colorBalance
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_ColorBalance_H
#define FLY_ColorBalance_H
#include "colorBalance.h"
/**
    \class flyColorBalance
*/
class flyColorBalance : public ADM_flyDialogYuv
{

  public:
    colorBalance  param;
    bool          showOriginal;
    bool          showRanges;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyColorBalance (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider)  : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {};
    virtual    ~flyColorBalance() {};
};
#endif
