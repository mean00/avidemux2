/**/
/***************************************************************************
                          DIA_flyImageStab
                             -------------------

			   Ui for ImageStab

    begin                : 08 Apr 2005
    copyright            : (C) 2004/7 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyImageStab.h"
#include <QPalette>
#include <cmath>
/************* COMMON PART *********************/

/**
 */
flyImageStab::flyImageStab (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
  {
  }
/**
 * 
 */
flyImageStab::~flyImageStab()
{

}

uint8_t  flyImageStab::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyImageStab::processYuv(ADMImage *in,ADMImage *out )
{
    bool newScene;
    float sceneDiff;
    QPalette indctrPalette(indctr->palette());
    QColor color;
    
    out->duplicate(in);

    // Do it!
    ADMVideoImageStab::ImageStabProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param, &buffers, &newScene, &sceneDiff);

    color.setRgb(0,(newScene ? 255:64),0,255);
    indctrPalette.setColor(QPalette::Window,color);
    indctrPalette.setColor(QPalette::Base,color);
    indctrPalette.setColor(QPalette::AlternateBase,color);
    indctr->setPalette(indctrPalette);
    indctrPB->setValue(round(sceneDiff*100.0));
    
    return 1;
}

