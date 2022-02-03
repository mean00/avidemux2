/***************************************************************************
  \fn     DIA_flyCubicLUT
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for cubicLUT
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_CubicLUT_H
#define FLY_CubicLUT_H
#include "cubicLUT.h"
/**
    \class flyCubicLUT
*/
class flyCubicLUT : public ADM_flyDialogYuv
{
  public:
    cubicLUT         param;
  private:
    uint8_t *        lut;
  public:
    bool             lutValid;
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void) {return 1;};
    uint8_t    upload(void) {return 1;};
    uint8_t    update(void);
    void       setTabOrder(void);
    flyCubicLUT (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {
                          lut=(uint8_t*)malloc(256L*256L*256L*3L);
                          lutValid=false;
                };
    virtual ~flyCubicLUT() {
                          if (lut) free(lut);
                };
    const char *   loadHald(const char *filename);
    const char *   loadCube(const char *filename);

};
#endif
