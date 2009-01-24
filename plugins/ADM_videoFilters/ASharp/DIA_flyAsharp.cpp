/**/
/***************************************************************************
                          DIA_Asharp
                             -------------------

                           Ui for Asharp

    begin                : 08 Apr 2005
    copyright            : (C) 2004/5 by mean
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
#include "ADM_vidASharp_param.h"
#include "DIA_flyAsharp.h"


/************* COMMON PART *********************/
uint8_t  flyASharp::update(void)
{
    download();
    process();
	copyYuvFinalToRgb();
    display();
    return 1;
}
uint8_t flyASharp::process(void)
{
uint8_t *src,*dst;
uint32_t stride;
int32_t T,D,B,B2;
uint32_t ww,hh;


                ww=_w;
                hh=_h;
                // parameters floating point to fixed point convertion
                T = (int)(param.t*(4<<7));
                D = (int)(param.d*(4<<7));
                B = (int)(256-param.b*64);
                B2= (int)(256-param.b*48);

                // clipping (recommended for SIMD code)

                if (T<-(4<<7)) T = -(4<<7); // yes, negatives values are accepted
                if (D<0) D = 0;
                if (B<0) B = 0;
                if (B2<0) B2 = 0;

                if (T>(32*(4<<7))) T = (32*(4<<7));
                if (D>(16*(4<<7))) D = (16*(4<<7));
                if (B>256) B = 256;
                if (B2>256) B2 = 256;


                memcpy(YPLANE(_yuvBufferOut),YPLANE(_yuvBuffer),ww*hh);
                memcpy(UPLANE(_yuvBufferOut),UPLANE(_yuvBuffer),ww*hh/4);
                memcpy(VPLANE(_yuvBufferOut),VPLANE(_yuvBuffer),ww*hh/4);
                asharp_run_c(     _yuvBufferOut->GetWritePtr(PLANAR_Y),
                        _yuvBufferOut->GetPitch(PLANAR_Y), 
                        hh,
                        ww,
                        T,
                        D,
                        B,
                        B2,
                        param.bf);
    
    // Copy back half source to display
    dst=_yuvBufferOut->data;
    src=_yuvBuffer->data;
    stride=ww;
    for(uint32_t y=0;y<hh;y++)   // We do both u & v!
    {
        memcpy(dst,src,stride>>1);
        dst+=stride;
        src+=stride;
    }
    return 1;
}
//EOF
