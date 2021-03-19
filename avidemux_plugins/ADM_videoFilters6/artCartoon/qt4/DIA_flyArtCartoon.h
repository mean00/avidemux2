/***************************************************************************
  \fn     DIA_flyArtCartoon
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for artCartoon
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_ARTCartoon_H
#define FLY_ARTCartoon_H
#include "artCartoon.h"
/**
    \class flyArtCartoon
*/
class flyArtCartoon : public ADM_flyDialogYuv
{
  private:
    int                    rgbBufStride;
    ADM_byteBuffer *       rgbBufRaw;
    ADMImageRef *          rgbBufImage;
    ADMColorScalerFull *   convertYuvToRgb;
    ADMColorScalerFull *   convertRgbToYuv;

    void       createBuffers(void);
    void       destroyBuffers(void);

  public:
    artCartoon param;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);

    flyArtCartoon(QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in, ADM_QCanvas *canvas, ADM_QSlider *slider)
        : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) { createBuffers(); }
    ~flyArtCartoon() { destroyBuffers(); }
};
#endif
