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

/**
    \class flyASharp
*/
#include "delogo.h"
class ADM_rubberControl;
class flyMpDelogo;

/**
    \class flyMpDelogo
*/
class flyMpDelogo : public ADM_flyDialogYuv
{
public:
    bool    blockChanges(bool block)  ;
public:
   delogo      param;
   bool        preview;
   ADM_rubberControl *rubber;
public:
   uint8_t     processYuv(ADMImage* in, ADMImage *out);
   uint8_t     download(void);
   uint8_t     upload() {return upload(true,true);}
   uint8_t     upload(bool update, bool toRubber);
               flyMpDelogo (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider);
                
   virtual     ~flyMpDelogo() ;
   bool         setPreview(bool onoff)
                {
                    preview=onoff;
                    if(preview)
                      rubber->hide();
                    else
                      rubber->show();
                    return true;
                }
    bool    bandResized(int x,int y,int w, int h);
private:
    int _ox,_oy,_ow,_oh;
};
// EOF
