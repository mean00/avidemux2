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
#include "Q_logo.h"

/**
    \fn process
*/
uint8_t    flyLogo::processYuv(ADMImage* in, ADMImage *out)
{
    out->duplicate(in);
    Ui_logoWindow *parent=(Ui_logoWindow *)this->_cookie;
    if(!parent->image)
        return true;
    
    int targetHeight=out->GetHeight(PLANAR_Y);
    int targetWidth =out->GetWidth(PLANAR_Y);
    
    
    if(param.y>targetHeight) 
        return true;
    if(param.x>targetWidth) 
        return true;

    ADMImage *myImage=parent->image;
    if(myImage->GetReadPtr(PLANAR_ALPHA))
        myImage->copyWithAlphaChannel(out,param.x,param.y);
    else
        myImage->copyToAlpha(out,param.x,param.y,param.alpha);
    return true;
}
//EOF
