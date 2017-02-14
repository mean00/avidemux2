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
class QRubberBand;
class flyMpDelogo;
/**
        \class Resizable_rubber_band
        \brief http://stackoverflow.com/questions/19066804/implementing-resize-handles-on-qrubberband-is-qsizegrip-relevant
*/
class Resizable_rubber_band : public QWidget 
{
public:
  flyMpDelogo     *flyParent;
  Resizable_rubber_band(flyMpDelogo *fly,QWidget* parent = 0);

public:
  QRubberBand* rubberband;
  void resizeEvent(QResizeEvent *);
  void blockSignals(bool sig)
  {
      rubberband->blockSignals(sig);
  }
#if 0
  void update()
  {
    rubberband->update();
  }
#endif
};

/**
    \class flyMpDelogo
*/
class flyMpDelogo : public ADM_flyDialogYuv
{
protected:
    bool    blockChanges(bool block)  ;
public:
   delogo      param;
   bool        preview;
   Resizable_rubber_band *rubber;
public:
   uint8_t     processYuv(ADMImage* in, ADMImage *out);
   uint8_t     download(void);
   uint8_t     upload() {return upload(true);}
   uint8_t     upload(bool update);
               flyMpDelogo (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, QSlider *slider);
                
   virtual     ~flyMpDelogo() ;
   bool         setXy(int x,int y);
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
    void    autoZoom(bool state);
};
// EOF
