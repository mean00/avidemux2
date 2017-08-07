/***************************************************************************
                          ADM_vidContrast.cpp  -  description
                             -------------------
    begin                : Sun Sep 22 2002
    copyright            : (C) 2002 by mean
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
#include "contrast.h"
class QGraphicsScene;
/**
    \class flyContrast
*/
class flyContrast : public ADM_flyDialogYuv
{
  public:
   contrast     param;
   QGraphicsScene *scene;
   bool          previewActivated;
   
  public:
    uint8_t    processYuv(ADMImage* in, ADMImage *out);
    void       setState(bool a){previewActivated=a;}
   uint8_t    download(void);
   uint8_t    upload(void);
   uint8_t    update(void);
                flyContrast (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_QSlider *slider,QGraphicsScene*sc)
                : ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) 
                {
                  scene=sc;
                  previewActivated=true;
                };
};

