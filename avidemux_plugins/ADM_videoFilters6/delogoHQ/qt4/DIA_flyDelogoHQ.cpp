/**/
/***************************************************************************
                          DIA_flyDelogoHQ
                             -------------------

			   Ui for DelogoHQ filter

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
#include "DIA_flyDelogoHQ.h"
#include "ADM_vidDelogoHQ.h"
#include "DIA_coreToolkit.h"

/************* COMMON PART *********************/
uint8_t  flyDelogoHQ::update(void)
{
    return 1;
}
/**
    \fn setMask
*/
bool flyDelogoHQ::setMask(ADMImage * newMask)
{
    if (!newMask) return false;
    if ((newMask->GetWidth(PLANAR_Y) != _w) || (newMask->GetHeight(PLANAR_Y) != _h))
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("delogoHQ","The selected image has different width/height."), NULL);
        return false;
    }

    if (!mask) mask = (int * )malloc(_w*_h*sizeof(int));
    if (!mask) return false;
    ADMVideoDelogoHQ::DelogoHQPrepareMask_C(mask, maskHint, _w, _h, newMask);
    return true;
}
/**
    \fn processYuv
*/
uint8_t   flyDelogoHQ::processYuv(ADMImage *in,ADMImage *out )
{
    uint8_t *src,*dst;
    uint32_t stride;
    out->duplicate(in);

    if (saveFilename)
    {
        if (!(in->saveAsPng(saveFilename)))
            GUI_Error_HIG(QT_TRANSLATE_NOOP("delogoHQ","Save failed!"), NULL);
        free(saveFilename);
        saveFilename = NULL;
    }


    if (showOriginal || (!mask))
    {
        //out->printString(1,1,"Original");
    } else {
        // Do it!
        ADMVideoDelogoHQ::DelogoHQProcess_C(out,in->GetWidth(PLANAR_Y),in->GetHeight(PLANAR_Y),mask,maskHint,param.blur,param.gradient, rgbBufStride, rgbBufRaw, rgbBufImage, convertYuvToRgb, convertRgbToYuv);
        //out->printString(1,1,"Processed"); 
    }

    return 1;
}

