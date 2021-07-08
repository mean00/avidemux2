/***************************************************************************
  \fn     DIA_flyImageStab
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for imageStab
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_QUADTRANS_H
#define QUADTRANS_H
#include "imageStab.h"
#include "ADM_vidImageStab.h"
#include <QLineEdit>
#include <QProgressBar>
/**
    \class flyImageStab
*/
class flyImageStab : public ADM_flyDialogYuv
{
  friend class Ui_imageStabWindow;
  public:
    imageStab            param;
    ADMVideoImageStab::imageStab_buffers_t  buffers;
    QLineEdit          * indctr;
    QProgressBar       * indctrPB;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyImageStab (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
              ~flyImageStab();
};
#endif
