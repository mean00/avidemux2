/**/
/***************************************************************************
                          DIA_flyArtCartoon
                             -------------------

			   Ui for Cartoon

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
#include "DIA_flyArtCartoon.h"
#include "ADM_vidArtCartoon.h"

/************* COMMON PART *********************/
uint8_t  flyArtCartoon::update(void)
{
    return 1;
}
/**
    \fn createBuffers
*/
void flyArtCartoon::createBuffers(void)
{
    ADMVideoArtCartoon::ArtCartoonCreateBuffers(_w, _h, &rgbBufStride, &rgbBufRaw, &rgbBufImage, &convertYuvToRgb, &convertRgbToYuv);
}
/**
    \fn destroyBuffers
*/
void flyArtCartoon::destroyBuffers(void)
{
    ADMVideoArtCartoon::ArtCartoonDestroyBuffers(rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
}
/**
    \fn processYuv
*/
uint8_t   flyArtCartoon::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    // Do it!
    ADMVideoArtCartoon::ArtCartoonProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param.threshold,param.scatter,param.color, rgbBufStride, rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
    return 1;
}

