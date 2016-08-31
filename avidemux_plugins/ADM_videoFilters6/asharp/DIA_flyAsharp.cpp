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


#include "asharp.h"
#include "DIA_flyAsharp.h"


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


                ww=in->GetWidth(PLANAR_Y);
                hh=in->GetHeight(PLANAR_Y);
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

                out->duplicateFull(in);
                uint8_t *line=new uint8_t[ww];
                asharp_run_c(     
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
    
    // Copy back half source to display
    dst=out->GetWritePtr(PLANAR_Y);
    src=in->GetReadPtr(PLANAR_Y);
    sstride=in->GetPitch(PLANAR_Y);
    dstride=out->GetPitch(PLANAR_Y);
    for(uint32_t y=0;y<hh;y++)   // We do both u & v!
    {
        memcpy(dst,src,ww/2);
        dst+=dstride;
        src+=sstride;
    }
    // add separator
    dst=out->GetWritePtr(PLANAR_Y)+ww/2;
    for(int j=0;j<hh/2;j++)
    {
        dst[0]=0;
        dst[dstride]=0xff;
        dst+=dstride*2;
    }
    out->printString(1,1,QT_TRANSLATE_NOOP("asharp", "Original"));
    out->printString(ww/24+1,1,QT_TRANSLATE_NOOP("asharp", "Processed"));
    return 1;
}
//EOF
