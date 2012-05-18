/***************************************************************************
                          ADM_guiContrast.cpp  -  description
                             -------------------
    begin                : Mon Sep 23 2002
    copyright            : (C) 2002 by mean
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
#include "DIA_flyDialog.h"
#include "ADM_default.h"
#include "ADM_image.h"

#include "ADM_vidContrast.h"
#include "contrast.h"

#include "DIA_flyContrast.h"

/************* COMMON PART *********************/
uint8_t  flyContrast::update(void)
{
    return 1;
}
// Ugly !
static uint8_t tableluma[256], tablechroma[256];
/**
    \fn processYuv
*/
uint8_t    flyContrast::processYuv(ADMImage* in, ADMImage *out)
{
    buildContrastTable (param.coef, param.offset, tableluma, tablechroma);


    out->copyInfo(in);

    if(param.doLuma)
        doContrast(in,out,tableluma,PLANAR_Y);
    else
        out->copyPlane(in,out,PLANAR_Y);


    if(param.doChromaU)
        doContrast(in,out,tablechroma,PLANAR_U);
    else
        out->copyPlane(in,out,PLANAR_U);

    if(param.doChromaV)
        doContrast(in,out,tablechroma,PLANAR_V);
    else
        out->copyPlane(in,out,PLANAR_V);

    return 1;
}
/************* COMMON PART *********************/
//EOF

