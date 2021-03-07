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
#include "ADM_default.h"
#include "DIA_flyDialogQt4.h"
#include "ADM_vidArtColorEffect.h"
#include "DIA_flyArtColorEffect.h"

/************* COMMON PART *********************/

/**
    \fn ctor
*/
flyArtColorEffect::flyArtColorEffect (QDialog *parent, uint32_t width, uint32_t height,
        ADM_coreVideoFilter *in, ADM_QCanvas *canvas, ADM_QSlider *slider)
        : ADM_flyDialogYuv (parent, width, height, in, canvas, slider, RESIZE_AUTO)
{
    ADMVideoArtColorEffect::ArtColorEffectCreateBuffers (width, height, &rgbBufStride,
        &rgbBufRaw, &rgbBufImage, &convertYuvToRgb, &convertRgbToYuv);
}
/**
    \fn dtor
*/
flyArtColorEffect::~flyArtColorEffect()
{
    ADMVideoArtColorEffect::ArtColorEffectDestroyBuffers (rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
}
/**
    \fn update
*/
uint8_t  flyArtColorEffect::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyArtColorEffect::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    // Do it!
    ADMVideoArtColorEffect::ArtColorEffectProcess_C (out, in->GetWidth(PLANAR_Y), in->GetHeight(PLANAR_Y),
        param.effect, rgbBufStride, rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
    // Copy half source to display
    //in->copyLeftSideTo(out);
    //out->printString(1,1,"Processed"); // printString can't handle non-ascii input, do not translate this!
    //out->printString(in->GetWidth(PLANAR_Y)/24+1,1,"Processed"); // as above, don't try to translate

    return 1;
}
//EOF
