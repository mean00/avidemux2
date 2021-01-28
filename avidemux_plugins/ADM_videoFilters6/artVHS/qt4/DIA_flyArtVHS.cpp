/**/
/***************************************************************************
                          DIA_ArtVHS
                             -------------------

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
#include "DIA_flyArtVHS.h"

#include "ADM_assert.h"
// FIXME
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
extern void ArtVHSProcess_C(ADMImage *img, float lumaBW, float chromaBW, float unSync, bool lumaNoDelay, bool chromaNoDelay);

/************* COMMON PART *********************/
uint8_t  flyArtVHS::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyArtVHS::processYuv(ADMImage *in,ADMImage *out )
{
    uint8_t *src,*dst;
    uint32_t stride;
    out->copyPlane(in,out,PLANAR_Y);
    out->copyPlane(in,out,PLANAR_U);
    out->copyPlane(in,out,PLANAR_V);

    // Do it!
    ArtVHSProcess_C(out, param.lumaBW, param.chromaBW, param.unSync, param.lumaNoDelay, param.chromaNoDelay);
    return 1;
}
//EOF
