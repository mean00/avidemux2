/***************************************************************************
  \fn     DIA_flyFadeFromImage
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for fadeFromImage
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DIA_FLYFADEFROMIMAGE_H
#define DIA_FLYFADEFROMIMAGE_H
#include "fadeFromImage.h"
#include "ADM_vidFadeFromImage.h"
#include "DIA_flyDialogQt4.h"
/**
    \class flyFadeFromImage
*/
class flyFadeFromImage : public ADM_flyDialogYuv
{
  private:
    ADMVideoFadeFromImage::fadeFromImage_buffers_t  buffers;

  public:
    fadeFromImage  param;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyFadeFromImage (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
              ~flyFadeFromImage();
};
#endif
