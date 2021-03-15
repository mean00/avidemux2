/**/
/***************************************************************************
                          DIA_ColorBalance
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
#include "DIA_flyColorBalance.h"
#include "ADM_vidColorBalance.h"


/**
    \fn update
*/
uint8_t  flyColorBalance::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyColorBalance::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    if (showRanges)
    {
        out->printString(1,1,"Ranges");
        ADMVideoColorBalance::ColorBalanceRanges_C(out);
    } else
    if (showOriginal)
    {
        out->printString(1,1,"Original");
    } else {
        // Do it!
        ADMVideoColorBalance::ColorBalanceProcess_C(out, param);
        out->printString(1,1,"Processed"); 
    }
    return 1;
}

