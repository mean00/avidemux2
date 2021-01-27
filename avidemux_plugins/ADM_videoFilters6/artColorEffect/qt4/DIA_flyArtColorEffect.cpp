/**/
/***************************************************************************
                          DIA_flyArtColorEffect
                             -------------------

			   Ui for ColorEffect

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
#include "DIA_flyArtColorEffect.h"

#include "ADM_assert.h"

extern void ArtColorEffectProcess_C(ADMImage *img, int w, int h, int effect, int rgbBufStride, ADM_byteBuffer * rgbBufRaw, ADMImageRef * rgbBufImage, ADMColorScalerFull * convertYuvToRgb, ADMColorScalerFull * convertRgbToYuv);

/************* COMMON PART *********************/
uint8_t  flyArtColorEffect::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyArtColorEffect::processYuv(ADMImage *in,ADMImage *out )
{
uint8_t *src,*dst;
uint32_t stride;
uint32_t effect;
    effect=param.effect;
    out->copyPlane(in,out,PLANAR_Y);
    out->copyPlane(in,out,PLANAR_U);
    out->copyPlane(in,out,PLANAR_V);

    // Do it!
    ArtColorEffectProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),effect,rgbBufStride, rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
    // Copy half source to display
    //in->copyLeftSideTo(out);
    out->printString(1,1,"Processed"); // printString can't handle non-ascii input, do not translate this!
    //out->printString(in->GetWidth(PLANAR_Y)/24+1,1,"Processed"); // as above, don't try to translate

    return 1;
}
//EOF
