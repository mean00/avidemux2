/**/
/***************************************************************************
                          DIA_flyFlat360
                             -------------------

			   Ui for Flat360

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
#include "DIA_flyFlat360.h"

/************* COMMON PART *********************/

/**
 */
flyFlat360::flyFlat360 (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
{
    ADMVideoFlat360::Flat360CreateBuffers(width,height, &buffers);
}
/**
 * 
 */
flyFlat360::~flyFlat360()
{
    ADMVideoFlat360::Flat360DestroyBuffers(&buffers);
}

uint8_t  flyFlat360::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyFlat360::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    // Do it!
    ADMVideoFlat360::Flat360Process_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param, &buffers);
    return 1;
}

