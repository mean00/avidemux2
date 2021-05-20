/**/
/***************************************************************************
                          DIA_Flip
                          --------

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
#include "DIA_flyFlip.h"

#include "ADM_vidFlip.h"

/************* COMMON PART *********************/
/**
 * 
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
flyFlip::flyFlip (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
{
    scratch=(uint8_t *)malloc(width);
}

/**
    \fn dtor
*/
flyFlip::~flyFlip()
{
    free(scratch);
    scratch=NULL;
}

/**
    \fn processYuv
*/
uint8_t   flyFlip::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    ADMVideoFlip::FlipProcess_C(out, scratch, param.flipdir);
    return 1;
}

