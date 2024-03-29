/**/
/***************************************************************************
                          DIA_Grain
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
#include "DIA_flyGrain.h"
#include "ADM_vidGrain.h"

/************* COMMON PART *********************/
uint8_t  flyGrain::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyGrain::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    // Do it!
    ADMVideoGrain::GrainProcess_C(out, param.noise);
    return 1;
}

