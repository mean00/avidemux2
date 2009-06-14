/***************************************************************************
                          ADM_guiChromaShift.cpp  -  description
                             -------------------
    begin                : Sun Aug 24 2003
    copyright            : (C) 2002-2003 by mean
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
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "DIA_flyDialog.h"
#include "ADM_default.h"

#include "ADM_image.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidChromaShift.h"
#include "ADM_vidChromaShift_param.h"

#include "DIA_flyChromaShift.h"

/*********  COMMON PART *********/
uint8_t    flyChromaShift::process(void)
{
        // First copy Y
        memcpy(_yuvBufferOut->data,_yuvBuffer->data,(_w*_h));
        // then shift u

         ADMVideoChromaShift::shift(_yuvBufferOut->data+_w*_h,
                                    _yuvBuffer->data+_w*_h, _w>>1,_h>>1,param.u);
        ADMVideoChromaShift::shift(_yuvBufferOut->data+((5*_w*_h)>>2),
                                    _yuvBuffer->data+((5*_w*_h)>>2), _w>>1,_h>>1,param.v);
        if(param.u)
                ADMVideoChromaShift::fixup(_yuvBufferOut->data,_w,_h,param.u*2);
        if(param.v)
                ADMVideoChromaShift::fixup(_yuvBufferOut->data,_w,_h,param.v*2);

}
uint8_t    flyChromaShift::update(void)
{
   download();
    process();
	copyYuvFinalToRgb();
    display();
}
//EOF

