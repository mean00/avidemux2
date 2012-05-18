
/***************************************************************************
                        flyDialog for Eq2-Gtk
                        (C) Mean 2007
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef FLY_EQ2_H
#define FLY_EQ2_H
#include "ADM_vidEq2.h"
class flyEq2 : public FLY_DIALOG_TYPE
{
  
  public:
   eq2         param;
  public:
   uint8_t    processYuv(ADMImage* in, ADMImage *out);
   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
   			  flyEq2 (uint32_t width,uint32_t height,ADM_coreVideoFilter *in, void *canvas, void *slider) : 
   			 FLY_DIALOG_TYPE(width, height,in,canvas, slider,1,RESIZE_AUTO)
                    {
                      
                    };
};

#endif
