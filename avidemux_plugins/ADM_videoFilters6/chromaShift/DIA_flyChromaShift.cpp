/***************************************************************************
                          ADM_guiChromaShift.cpp  -  description
                             -------------------
    begin                : Sun Aug 24 2003
    copyright            : (C) 2002-2003 by mean
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

#include "ADM_vidChromaShift.h"
#include "chromashift.h"

#include "DIA_flyChromaShift.h"

/*********  COMMON PART *********/
uint8_t  flyChromaShift::update(void)
{
    return 1;
}
/**
    \fn process
*/
uint8_t    flyChromaShift::processYuv(ADMImage* in, ADMImage *out)
{
        in->copyPlane(in,out,PLANAR_Y);        
        ADMVideoChromaShift::shiftPlane(PLANAR_U,in,out,param.u);
        ADMVideoChromaShift::shiftPlane(PLANAR_V,in,out,param.v);
        if(param.u)
                ADMVideoChromaShift::fixup(out,param.u*2);
        if(param.v)
                ADMVideoChromaShift::fixup(out,param.v*2);
        return 1;
}
//EOF

