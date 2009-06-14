/***************************************************************************
                          ADM_guiContrast.cpp  -  description
                             -------------------
    begin                : Mon Sep 23 2002
    copyright            : (C) 2002 by mean
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

#include "DIA_flyDialog.h"
#include "ADM_default.h"

#include "ADM_image.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidContrast.h"
#include "DIA_flyContrast.h"
/************* COMMON PART *********************/
uint8_t  flyContrast::update(void)
{
    download();
    process();
	copyYuvFinalToRgb();
    display();
    return 1;
}
// Ugly !
static uint8_t tableluma[256], tablechroma[256];

uint8_t flyContrast::process(void)

{
  buildContrastTable (param.coef, param.offset, tableluma, tablechroma);

  memcpy (_yuvBufferOut->data, _yuvBuffer->data, (_h * _w * 3)>>1);
  if (param.doLuma)
    {
      doContrast (YPLANE(_yuvBuffer), YPLANE(_yuvBufferOut), tableluma, _w, _h);
    }
  if (param.doChromaU)
    {
      doContrast (UPLANE(_yuvBuffer), UPLANE(_yuvBufferOut), tablechroma, _w >> 1, _h >> 1);
    }

  if (param.doChromaV)
    {
     doContrast (VPLANE(_yuvBuffer), VPLANE(_yuvBufferOut), tablechroma, _w >> 1, _h >> 1);
    }
    return 1;
}
/************* COMMON PART *********************/
