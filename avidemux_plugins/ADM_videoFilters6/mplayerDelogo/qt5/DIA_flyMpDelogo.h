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
#include "delogo.h"

class flyMpDelogo;

/**
    \class flyMpDelogo
*/
class flyMpDelogo : public ADM_flyDialogYuv
{
private:
    int         _ox,_oy,_ow,_oh;
    delogo      param;
    bool        preview;
    ADM_rubberControl *rubber;

    uint8_t     upload(bool update, bool toRubber);
    bool        boundCheck(bool sizeHasPriority);
    void        adjustRubber(void);

public:
   uint8_t     processYuv(ADMImage* in, ADMImage *out);
   uint8_t     download(bool sizeHasPriority);
   uint8_t     download(void) {return download(true);}
   uint8_t     upload() {return upload(true,true);}
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
    bool    bandMoved(int x,int y,int w, int h);

    bool    blockChanges(bool block);
    void    initRubber(void);
    int     lockRubber(bool lock);

    delogo  *getParam(void) { return &param; }
    void    setParam(delogo *ps);
    void    setTabOrder(void);
};
// EOF
