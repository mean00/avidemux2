/***************************************************************************
  \fn     DIA_flyAiEnhance
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for aiEnhance
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
#define FLY_QUADTRANS_H
#include "aiEnhance.h"
#include "ADM_vidAiEnhance.h"

/**
    \class flyAiEnhance
*/
class flyAiEnhance : public ADM_flyDialogYuv
{
  protected:
    ADMVideoAiEnhance::aiEnhance_buffers_t  buffers;

  public:
    aiEnhance            param;
    bool                 showOriginal;
    int                  previewScale;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyAiEnhance (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider);
              ~flyAiEnhance();
};
#endif
