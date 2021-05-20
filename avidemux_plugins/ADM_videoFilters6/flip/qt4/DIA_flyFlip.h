/***************************************************************************
  \fn     DIA_flyFlip
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for flip
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_FLIP_H
#define FLY_FLIP_H
#include "flip.h"
/**
    \class flyFlip
*/
class flyFlip : public ADM_flyDialogYuv
{
  private:
    uint8_t   *scratch;
  public:
    flip       param;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void) {return 1;};
    void       setTabOrder(void);
               flyFlip (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
    virtual    ~flyFlip() ;
};
#endif
