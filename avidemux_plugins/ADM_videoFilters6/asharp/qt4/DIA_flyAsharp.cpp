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
#include "ADM_default.h"
#include "DIA_flyAsharp.h"
#include "ADM_vidAsharp.h"

/************* COMMON PART *********************/
/**
    \fn update
*/
uint8_t  flyASharp::update(void)
{
//    download();
//    process();
//	copyYuvFinalToRgb();
//    display();
    return 1;
}
/**
    \fn process
*/
uint8_t    flyASharp::processYuv(ADMImage* in, ADMImage *out)
{

uint8_t *src,*dst;
uint32_t sstride,dstride;
int32_t T,D,B,B2;
uint32_t ww,hh;

#define ALMOST_ZERO 0.002
    // fake a non-zero value for param.d
    float faked = param.d;
    if(faked < ALMOST_ZERO) faked = ALMOST_ZERO;

                ww=in->GetWidth(PLANAR_Y);
                hh=in->GetHeight(PLANAR_Y);
                // parameters floating point to fixed point convertion
                T = (int)(param.t*(4<<7));
                D = param.d_enabled ? (int)(faked*(4<<7)) : 0;
                B = param.b_enabled ? (int)(256-param.b*64) : 256;
                B2= param.b_enabled ? (int)(256-param.b*48) : 256;

                // clipping (recommended for SIMD code)

                if (T<-(4<<7)) T = -(4<<7); // yes, negatives values are accepted
                if (D<0) D = 0;
                if (B<0) B = 0;
                if (B2<0) B2 = 0;

                if (T>(32*(4<<7))) T = (32*(4<<7));
                if (D>(16*(4<<7))) D = (16*(4<<7));
                if (B>256) B = 256;
                if (B2>256) B2 = 256;

                out->duplicateFull(in);
                uint8_t *line=new uint8_t[ww];
                ASharp::asharp_run_c(
                        out->GetWritePtr(PLANAR_Y),
                        out->GetPitch(PLANAR_Y), 
                        hh,
                        ww,
                        T,
                        D,
                        B,
                        B2,
                        param.bf,line);
                delete [] line;

    return 1;
}
//EOF
