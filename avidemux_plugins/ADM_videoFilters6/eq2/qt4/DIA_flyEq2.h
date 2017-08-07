
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
class flyEq2 : public ADM_flyDialogYuv
{
  
  public:
   eq2         param;
  public:
   uint8_t    processYuv(ADMImage* in, ADMImage *out);
   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
              flyEq2 (QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in, ADM_QCanvas *canvas, ADM_QSlider *slider) :
                  ADM_flyDialogYuv(parent, width, height, in, canvas, slider, RESIZE_AUTO)
                    {
                      
                    };
};

#endif
