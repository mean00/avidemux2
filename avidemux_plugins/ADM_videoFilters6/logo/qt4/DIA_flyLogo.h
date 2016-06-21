/***************************************************************************
    copyright            : (C) 2011 by mean
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

#pragma once
#include "logo.h"
/**
    \class flyLogo
*/

class flyLogo : public FLY_DIALOG_TYPE
{
  
  public:
   logo        param;
   bool        preview;
  public:
   uint8_t     processYuv(ADMImage* in, ADMImage *out);
   uint8_t     download(void);
   uint8_t     upload(void);
               flyLogo (uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    void *canvas, void *slider) : 
                FLY_DIALOG_TYPE(width, height,in,canvas, slider,true,RESIZE_AUTO) 
                {};
   virtual     ~flyLogo() {};
   bool         setXy(int x,int y);
   bool         setPreview(bool onoff)
                {
                    preview=onoff;
                    return true;
                }
   
};
// EOF

