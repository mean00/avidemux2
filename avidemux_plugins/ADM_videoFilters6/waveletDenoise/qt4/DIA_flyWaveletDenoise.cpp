/**/
/***************************************************************************
                          DIA_WaveletDenoise
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
#include "DIA_flyWaveletDenoise.h"

#include "ADM_vidWaveletDenoise.h"

/************* COMMON PART *********************/
uint8_t  flyWaveletDenoise::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyWaveletDenoise::processYuv(ADMImage *in,ADMImage *out )
{
    out->duplicate(in);

    // Do it!
    ADMVideoWaveletDenoise::WaveletDenoiseProcess_C(out, param.threshold, param.softness, param.highq, param.chroma);

    return 1;
}

