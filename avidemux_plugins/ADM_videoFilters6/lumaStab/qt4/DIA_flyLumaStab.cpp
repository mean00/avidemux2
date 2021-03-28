/**/
/***************************************************************************
                          DIA_LumaStab
                             -------------------

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
#include "DIA_flyLumaStab.h"
#include "ADM_vidLumaStab.h"
#include <QPalette>
#include <cmath>

/************* COMMON PART *********************/
uint8_t  flyLumaStab::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyLumaStab::processYuv(ADMImage *in,ADMImage *out )
{
    bool newScene;
    float sceneDiff;
    QPalette indctrPalette(indctr->palette());
    QColor color;

    out->duplicate(in);
    // Do it!
    ADMVideoLumaStab::LumaStabProcess_C(out, param.filterLength, param.cbratio, param.sceneThreshold, param.chroma, yHyst, &yHystlen, prevChromaHist, &newScene, &sceneDiff);

    color.setRgb(0,(newScene ? 255:64),0,255);
    indctrPalette.setColor(QPalette::Window,color);
    indctrPalette.setColor(QPalette::Base,color);
    indctrPalette.setColor(QPalette::AlternateBase,color);
    indctr->setPalette(indctrPalette);
    indctrPB->setValue(round(sceneDiff*100.0));

    return 1;
}

