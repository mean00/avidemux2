/***************************************************************************
                          DIA_flyCrop.cpp  -  description
                             -------------------

        Common part of the crop dialog
    
    copyright            : (C) 2002/2007 by mean
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
#include "DIA_flyBlackenBorders.h"


/**
    \fn process
	\brief 
*/
uint8_t    flyBlacken::processRgb(uint8_t *imageIn, uint8_t *imageOut)
{
        uint32_t x,y;
        uint8_t  *in;
        uint32_t w=_w,h=_h;
  
        
        memcpy(imageOut,imageIn,_w*_h*4);
        in=imageOut;
        for(y=0;y<top;y++)
        {
                for(x=0;x<w;x++)
                {
                        *in++=0;
                        
                        
                        *in++=0xff;
                        
                        *in++=0;
                        *in++=0xff;
                }
        }
        // bottom
        in=imageOut+(w*4)*(h-bottom);
        for(y=0;y<bottom;y++)
        {
                for(x=0;x<w;x++)
                {
                        *in++=0;
                        
                        
                        *in++=0xff;
                        *in++=0;
                        *in++=0xff;
                }
        }
        // left
        in=imageOut;
        uint32_t stride=4*w-4;
        for(y=0;y<h;y++)
        {
                for(x=0;x<left;x++)
                {
                        *(in+4*x)=0;
                        
                        
                        *(in+4*x+1)=0xff;
                        *(in+4*x+2)=0;
                        *(in+4*x+3)=0xff;
                }
                for(x=0;x<right;x++)
                {
                        *(in-4*x+stride-4)=0;
                        
                        
                        *(in-4*x+stride-3)=0xff;
                        *(in-4*x+stride-2)=0;
                        *(in-4*x+stride-1)=0xff;
                        
                }
                in+=4*w;
  
        }
        return true;

}

//EOF

