/**/
/***************************************************************************
                          DIA_ColorTemp
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
#include "DIA_flyColorTemp.h"

#include "ADM_vidColorTemp.h"

/************* COMMON PART *********************/
uint8_t  flyColorTemp::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyColorTemp::processYuv(ADMImage *in,ADMImage *out )
{
    uint8_t *src,*dst;
    uint32_t stride;
    out->duplicate(in);

    // Do it!
    ADMVideoColorTemp::ColorTempProcess_C(out, param.temperature, param.angle);

    return 1;
}

