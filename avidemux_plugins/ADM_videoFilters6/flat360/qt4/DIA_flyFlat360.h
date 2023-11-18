/***************************************************************************
  \fn     DIA_flyFlat360
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for flat360
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_FLAT360_H
#define FLY_FLAT360_H
#include "flat360.h"
#include "ADM_vidFlat360.h"

/**
    \class flyFlat360
*/
class flyFlat360 : public ADM_flyDialogYuv
{
  private:
    ADMVideoFlat360::flat360_buffers_t  buffers;
  public:
    flat360            param;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyFlat360 (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider);
              ~flyFlat360();
};
#endif
