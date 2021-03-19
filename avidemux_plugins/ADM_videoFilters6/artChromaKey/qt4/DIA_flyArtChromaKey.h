/***************************************************************************
  \fn     DIA_flyArtChromaKey
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for artChromaKey
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_ARTChromaKey_H
#define FLY_ARTChromaKey_H
#include "artChromaKey.h"
#include "ADM_image.h"
#include <QGraphicsScene>
/**
    \class flyArtChromaKey
*/
class flyArtChromaKey : public ADM_flyDialogYuv
{

  public:
    artChromaKey  param;
    QGraphicsScene *scene;
    bool          showTestImage;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
    flyArtChromaKey (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in, ADM_QCanvas *canvas, ADM_QSlider *slider,QGraphicsScene*sc) : 
        ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {
            scene=sc;
        };
  private:
    void drawScene();
};
#endif
