/**/
/***************************************************************************
                          DIA_flyFadeInOut
                             -------------------

			   Ui for FadeInOut

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
#include "ADM_image.h"
#include "DIA_flyFadeInOut.h"

/************* COMMON PART *********************/
uint8_t  flyFadeInOut::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyFadeInOut::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    // Do it!
    ADMVideoFadeInOut::FadeInOutProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),param);

    return 1;
}

