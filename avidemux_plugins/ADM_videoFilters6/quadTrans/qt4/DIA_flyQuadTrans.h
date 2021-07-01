/***************************************************************************
  \fn     DIA_flyQuadTrans
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for quadTrans
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
#include "quadTrans.h"
#include "ADM_vidQuadTrans.h"

/**
    \class flyQuadTrans
*/
class flyQuadTrans : public ADM_flyDialogYuv
{
  friend class Ui_quadTransWindow;
  public:
    quadTrans            param;
    ADMVideoQuadTrans::quadTrans_buffers_t  buffers;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyQuadTrans (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
              ~flyQuadTrans();
};
#endif
