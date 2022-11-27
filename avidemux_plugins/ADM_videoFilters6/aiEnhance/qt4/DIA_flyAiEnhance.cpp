/**/
/***************************************************************************
                          DIA_flyAiEnhance
                             -------------------

			   Ui for AiEnhance

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
#include "DIA_flyAiEnhance.h"

/************* COMMON PART *********************/

/**
 */
flyAiEnhance::flyAiEnhance (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider) : ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
  {
  }
/**
 * 
 */
flyAiEnhance::~flyAiEnhance()
{

}

uint8_t  flyAiEnhance::update(void)
{
    return 1;
}
/**
    \fn processYuv
*/
uint8_t   flyAiEnhance::processYuv(ADMImage *in,ADMImage *out )
{
    out->copyInfo(in);

    // Do it!
    ADMVideoAiEnhance::AiEnhanceProcess_C(in, out, true, previewScale, showOriginal, param, &buffers);
    return 1;
}

