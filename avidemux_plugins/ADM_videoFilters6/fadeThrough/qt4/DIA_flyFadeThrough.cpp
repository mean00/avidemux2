/**/
/***************************************************************************
                          DIA_flyFadeThrough
                             -------------------

			   Ui for FadeThrough

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
#include "DIA_flyFadeThrough.h"

/************* COMMON PART *********************/

/**
 */
flyFadeThrough::flyFadeThrough (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider, QGraphicsScene *sc) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
{
    scene = sc;
    ADMVideoFadeThrough::FadeThroughCreateBuffers(width,height,&buffers);
}
/**
 * 
 */
flyFadeThrough::~flyFadeThrough()
{
    ADMVideoFadeThrough::FadeThroughDestroyBuffers(&buffers);
}

uint8_t  flyFadeThrough::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyFadeThrough::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    // Do it!
    ADMVideoFadeThrough::FadeThroughProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param, &buffers);

    return 1;
}

