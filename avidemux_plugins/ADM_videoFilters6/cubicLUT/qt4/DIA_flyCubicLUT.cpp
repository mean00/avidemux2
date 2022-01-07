/**/
/***************************************************************************
                          DIA_flyCubicLUT
                             -------------------

			   Ui for CubicLUT filter

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
#include "DIA_flyCubicLUT.h"
#include "ADM_vidCubicLUT.h"
#include "DIA_coreToolkit.h"

/************* COMMON PART *********************/
uint8_t  flyCubicLUT::update(void)
{
    return 1;
}
/**
    \fn setHald
*/
const char * flyCubicLUT::loadHald(const char *filename)
{
    if (!filename) return " ";
    if (!lut) return " ";
    const char * errorMsg = ADMVideoCubicLUT::FileToLUT(filename, true, lut);
    if (errorMsg != NULL)
        return errorMsg;
    param.hald=true;
    return NULL;
}
/**
    \fn setCube
*/
const char * flyCubicLUT::loadCube(const char *filename)
{
    if (!filename) return " ";
    if (!lut) return " ";
    const char * errorMsg = ADMVideoCubicLUT::FileToLUT(filename, false, lut);
    if (errorMsg != NULL)
        return errorMsg;
    param.hald=false;
    return NULL;
}
/**
    \fn processYuv
*/
uint8_t   flyCubicLUT::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    if (lutValid)
        ADMVideoCubicLUT::CubicLUTProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),lut);

    return 1;
}

