/***************************************************************************
                          DIA_flyCrop.cpp  -  description
                             -------------------

        Common part of the crop dialog
    
    copyright            : (C) 2002/2017 by mean
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
#include "DIA_flyCrop.h"
#include "Q_crop.h"
#include "ADM_toolkitQt.h"
/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
int DIA_getCropParams(	const char *name,crop *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;

    Ui_cropWindow dialog(qtLastRegisteredDialog(), param,in);
    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=1;
    }
    qtUnregisterDialog(&dialog);
    return ret;
}

/**
     \fn Metrics
	\brief Compute the average value, variance and maximum of pixels
*/

uint8_t Metrics( uint8_t *in, uint32_t stride, uint32_t length, uint32_t *avg, uint32_t *var, uint32_t * max)
{
    uint8_t * ptr;
    uint32_t x;
    uint32_t sum=0,eq=0,m=0;
    int v;
    ptr = in;
    for(x=0; x<length; x++)
    {
        sum += *ptr;
        if (m < *ptr)
            m = *ptr;
        ptr += stride;
    }
    sum /= length;
    *avg = sum;
    *max = m;
    ptr = in;
    for(x=0;x<length;x++)
    {
        v = *ptr - sum;
        eq += v*v;
        ptr += stride;
    }
    eq /= length;
    *var = eq;
    return 1;
}
