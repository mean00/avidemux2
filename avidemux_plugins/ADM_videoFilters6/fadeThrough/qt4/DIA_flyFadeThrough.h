/***************************************************************************
  \fn     DIA_flyFadeThrough
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for fadeThrough
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DIA_FLYFadeThrough_H
#define DIA_FLYFadeThrough_H
#include "fadeThrough.h"
#include "ADM_vidFadeThrough.h"
#include "DIA_flyDialogQt4.h"
#include <QGraphicsScene>

/**
    \class flyFadeThrough
*/
class flyFadeThrough : public ADM_flyDialogYuv
{
  private:
    ADMVideoFadeThrough::fadeThrough_buffers_t  buffers;

  public:
    fadeThrough  param;
    QGraphicsScene *scene;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
    bool       getTabEnabled(int tabIndex);
    int        getTabTransient(int tabIndex);
    double     getTabTransientDuration(int tabIndex);
    void       redrawScene();
               flyFadeThrough (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider, QGraphicsScene *sc);
              ~flyFadeThrough();
};
#endif
