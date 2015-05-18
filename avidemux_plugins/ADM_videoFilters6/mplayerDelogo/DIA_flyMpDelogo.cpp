/**/
/***************************************************************************
                          DIA_flyMpDelogo
                             -------------------

                           Ui for MPlayer DeLogo filter

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


#include "delogo.h"
#include "DIA_flyMpDelogo.h"
#include "ADM_vidMPdelogo.h"


/************* COMMON PART *********************/
/**
    \fn process
*/
uint8_t    flyMpDelogo::processYuv(ADMImage* in, ADMImage *out)
{
    out->duplicate(in);
    if(preview)
        MPDelogo::doDelogo(out, param.xoff, param.yoff,
                             param.lw,  param.lh,param.band,param.show);        
    else
    {
        uint8_t *y=out->GetWritePtr(PLANAR_Y);
        int stride=out->GetPitch(PLANAR_Y);
        int mx=param.lw+param.xoff;
        int my=param.lh+param.yoff;
        if(mx>=out->GetWidth(PLANAR_Y)) mx=out->GetWidth(PLANAR_Y)-1;
        if(my>=out->GetHeight(PLANAR_Y)) my=out->GetHeight(PLANAR_Y)-1;
        
        
        // hz
        uint8_t *p=y+stride*param.yoff;
        uint8_t *p2=y+stride*(my);
        
        uint8_t toggle=0;
        for(int x=param.xoff;x<mx;x++)
        {
            *(p+x)=toggle;
            toggle=0xff^toggle;
            *(p2+x)=toggle;
        }
        // vz
         p=y+stride*(param.yoff)+param.xoff;
         p2=y+stride*(param.yoff)+mx;
         
         for(int yy=param.yoff;yy<my;yy++)
         {
            *(p)=toggle;
            toggle=0xff^toggle;
            *(p2)=toggle;
            p+=stride;   p2+=stride;
         }
    }
    return 1;
}
//EOF
