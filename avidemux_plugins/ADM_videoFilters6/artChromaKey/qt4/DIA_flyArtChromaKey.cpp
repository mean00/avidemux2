/**/
/***************************************************************************
                          DIA_ArtChromaKey
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
#include "DIA_flyArtChromaKey.h"
#include "Q_artChromaKey.h"
#include "ADM_vidArtChromaKey.h"

/************* COMMON PART *********************/
uint8_t  flyArtChromaKey::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyArtChromaKey::processYuv(ADMImage *in,ADMImage *out )
{
    bool cen[3];
    float cu[3];
    float cv[3];
    float cdist[3];
    float cslope[3];
    cen[0] = param.c1en;
    cu[0] = param.c1u;
    cv[0] = param.c1v;
    cdist[0] = param.c1dist;
    cslope[0] = param.c1slope;
    cen[1] = param.c2en;
    cu[1] = param.c2u;
    cv[1] = param.c2v;
    cdist[1] = param.c2dist;
    cslope[1] = param.c2slope;
    cen[2] = param.c3en;
    cu[2] = param.c3u;
    cv[2] = param.c3v;
    cdist[2] = param.c3dist;
    cslope[2] = param.c3slope;
    out->duplicate(in);

    Ui_artChromaKeyWindow *parent=(Ui_artChromaKeyWindow *)_parent;
    // Do it!
    if (parent)
    {
        if (showTestImage)
            ADMVideoArtChromaKey::ArtChromaKeyProcess_C(out, parent->testimage, cen, cu, cv, cdist, cslope, param.spill);
        else
            ADMVideoArtChromaKey::ArtChromaKeyProcess_C(out, parent->image, cen, cu, cv, cdist, cslope, param.spill);
    }
    parent = NULL;

    return 1;
}

