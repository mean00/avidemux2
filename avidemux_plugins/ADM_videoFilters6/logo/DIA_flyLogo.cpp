/**/
/***************************************************************************
\file DIA_flyLogo
\author mean (c) 2016, fixounet@free.Fr
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


#include "logo.h"
#include "DIA_flyLogo.h"
#include "ADM_vidLogo.h"
#include "qt4/Q_logo.h"

/**
    \fn process
*/
uint8_t    flyLogo::processYuv(ADMImage* in, ADMImage *out)
{
    out->duplicate(in);
    if(!parent->image)
        return true;
    
    int targetHeight=out->GetHeight(PLANAR_Y);
    int targetWidth =out->GetWidth(PLANAR_Y);
    
    
    if(param.y>targetHeight) 
        return true;
    if(param.x>targetWidth) 
        return true;

    
    // Draw hashed line
    int box_w=parent->imageWidth+this->param.x;
    int box_h=parent->imageHeight+this->param.y;;
    if(box_w>=targetWidth)
    {
        box_w=targetWidth;
    }
    if(box_h>=targetHeight)
    {
        box_h=targetHeight;
    }
    
    uint8_t *top=out->GetWritePtr(PLANAR_Y);
    int     pitch=out->GetPitch(PLANAR_Y);
    uint8_t *p=top+param.y*pitch;
    uint8_t mask=0xFF;
    for(int x=param.x;x<box_w;x++)     {p[x]=mask;mask^=0xff;}   
    p=top+(box_h-1)*pitch;    
    for(int x=param.x;x<box_w;x++)     {p[x]=mask;mask^=0xff;}

    return true;
}
//EOF
