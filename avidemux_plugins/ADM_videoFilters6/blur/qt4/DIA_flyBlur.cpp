/**/
/***************************************************************************
                          DIA_flyBlur
                             -------------------

			   Ui for Blur

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
#include "DIA_flyBlur.h"
#include "ADM_vidBlur.h"

/************* COMMON PART *********************/
uint8_t  flyBlur::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyBlur::processYuv(ADMImage *in,ADMImage *out )
{
    uint8_t *src,*dst;
    uint32_t stride;
    out->copyPlane(in,out,PLANAR_Y);
    out->copyPlane(in,out,PLANAR_U);
    out->copyPlane(in,out,PLANAR_V);

    // Do it!
    ADMVideoBlur::BlurProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param.algorithm, param.radius, rgbBufStride, rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
    return 1;
}

