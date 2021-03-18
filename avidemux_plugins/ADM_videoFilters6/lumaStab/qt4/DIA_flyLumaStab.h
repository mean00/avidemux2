/***************************************************************************
  \fn     DIA_flyLumaStab
  \author (C) 2004/2012 by mean/fixounet@free.fr
  \brief  Ui for lumaStab
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_LumaStab_H
#define FLY_LumaStab_H
#include "lumaStab.h"
#include <QLineEdit>
#include <QProgressBar>
/**
    \class flyLumaStab
*/
class flyLumaStab : public ADM_flyDialogYuv
{
  protected:
    float *        yHyst;
    int            yHystlen;
    float          prevChromaHist[128];
  public:
    lumaStab  param;
    QLineEdit * indctr;
    QProgressBar * indctrPB;
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);
    void       setTabOrder(void);
    flyLumaStab (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO) {
                                        yHystlen = 0;
                                        yHyst = (float*)malloc(256*sizeof(float));
                                        memset(prevChromaHist,0,128*sizeof(float));
               };
    virtual ~flyLumaStab() {
                                        free(yHyst);
               };
};
#endif
