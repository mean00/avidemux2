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
#ifndef DIA_FLYIMAGESTAB_H
#define DIA_FLYIMAGESTAB_H
#include "imageStab.h"
#include "ADM_vidImageStab.h"
#include "DIA_flyDialogQt4.h"
/**
    \class flyImageStab
*/
class flyImageStab : public ADM_flyDialogYuv
{
  private:
    ADMVideoImageStab::imageStab_buffers_t  buffers;
    bool       newScene;
    float      sceneDiff;

    void       refreshIndicator(void);

  public:
    imageStab  param;

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
