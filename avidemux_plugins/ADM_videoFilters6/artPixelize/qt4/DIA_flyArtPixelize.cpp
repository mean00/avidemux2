/**/
/***************************************************************************
                          DIA_ArtPixelize
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
#include "DIA_flyArtPixelize.h"

extern void ArtPixelizeProcess_C(ADMImage *img, unsigned int pw, unsigned int ph);

/************* COMMON PART *********************/
uint8_t  flyArtPixelize::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyArtPixelize::processYuv(ADMImage *in,ADMImage *out )
{
    uint8_t *src,*dst;
    uint32_t stride;
    out->duplicate(in);

    // Do it!
    ArtPixelizeProcess_C(out, param.pw, param.ph);
    // Copy half source to display
    //in->copyLeftSideTo(out);
    //out->printString(1,1,"Processed"); // printString can't handle non-ascii input, do not translate this!
    //out->printString(in->GetWidth(PLANAR_Y)/24+1,1,"Processed"); // as above, don't try to translate

    return 1;
}
//EOF
