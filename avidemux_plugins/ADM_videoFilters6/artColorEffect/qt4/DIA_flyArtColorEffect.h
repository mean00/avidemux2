/***************************************************************************
  \fn     DIA_flyArtColorEffect
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for artColorEffect
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_ARTCOLOREFFECT_H
#define FLY_ARTCOLOREFFECT_H
#include "artColorEffect.h"

/**
    \class flyArtColorEffect
*/
class flyArtColorEffect : public ADM_flyDialogYuv
{
  private:
    int                    rgbBufStride;
    ADM_byteBuffer *       rgbBufRaw;
    ADMImageRef *          rgbBufImage;
    ADMColorScalerFull *   convertYuvToRgb;
    ADMColorScalerFull *   convertRgbToYuv;

  public:
    artColorEffect         param;

    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
               flyArtColorEffect (QDialog *parent, uint32_t width, uint32_t height,
                   ADM_coreVideoFilter *in, ADM_QCanvas *canvas, ADM_QSlider *slider);
               ~flyArtColorEffect();
};
#endif
