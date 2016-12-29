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
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"

#include "ADM_vidContrast.h"
#include "contrast.h"

#include "DIA_flyContrast.h"
#include "QGraphicsScene"

/************* COMMON PART *********************/
uint8_t  flyContrast::update(void)
{
    return 1;
}
// Ugly !
static uint8_t tableluma[256], tablechroma[256];
/**
    \fn processYuv
*/
uint8_t    flyContrast::processYuv(ADMImage* in, ADMImage *out)
{
    buildContrastTable (param.coef, param.offset, tableluma, tablechroma);


    out->copyInfo(in);
    if(!previewActivated)
    {
        out->copyPlane(in,out,PLANAR_Y);
        out->copyPlane(in,out,PLANAR_U);
        out->copyPlane(in,out,PLANAR_V);
    }
    else
    {
        if(param.doLuma)
            doContrast(in,out,tableluma,PLANAR_Y);
        else
            out->copyPlane(in,out,PLANAR_Y);


        if(param.doChromaU)
            doContrast(in,out,tablechroma,PLANAR_U);
        else
            out->copyPlane(in,out,PLANAR_U);

        if(param.doChromaV)
            doContrast(in,out,tablechroma,PLANAR_V);
        else
            out->copyPlane(in,out,PLANAR_V);
    }
    if(!scene) return true;
    
    // Draw luma histogram
    uint8_t *luma=out->GetReadPtr(PLANAR_Y);
    int     stride=out->GetPitch(PLANAR_Y);
    int     decimate=4;
    double  sumsum[256];    
    for(int i=0;i<256;i++) sumsum[i]=0;
    
    double  totalSum=(double)(out->_width*out->_height)/decimate; // # of sampling points
    for(int y=0;y<in->_height;y+=decimate)
    {
        uint8_t *p=luma;
        for(int x=0;x<in->_width;x++)
        {
            sumsum[*p]++;
            p++;
        }
        luma+=stride*decimate;
    }
    // normalize
    for(int i=0;i<256;i++)
    {
        // zoom factor =10
        sumsum[i]=(10*sumsum[i]*(127))/totalSum;
        if(sumsum[i]>127) sumsum[i]=127;
    }
    int toggle=0;
    
    scene->clear();
    for(int i=0;i<256;i++)
    {
        QLineF qline(i,127,i,127-sumsum[i]);
        scene->addLine(qline);
    }
    // Draw 16 and 235 line
        QLineF qline(16,100,16,126);
        scene->addLine(qline);
        QLineF qline2(235,100,235,126);
        scene->addLine(qline2);
    
    return 1;
}
/************* COMMON PART *********************/
//EOF

