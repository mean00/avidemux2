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
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyImageStab.h"

/************* COMMON PART *********************/

/**
 */
flyImageStab::flyImageStab (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
{
    newScene = false;
    sceneDiff = 0.;
    ADMVideoImageStab::ImageStabCreateBuffers(width,height,&buffers);
}
/**
 * 
 */
flyImageStab::~flyImageStab()
{
    ADMVideoImageStab::ImageStabDestroyBuffers(&buffers);
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
    out->duplicate(in);

    // Do it!
    ADMVideoImageStab::ImageStabProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param, &buffers, &newScene, &sceneDiff);

    refreshIndicator();

    return 1;
}

