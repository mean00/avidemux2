/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///


#include <QtGui/QGraphicsView>
#include <QtGui/QSlider>

#include "ADM_default.h"

#include "ADM_videoFilter.h"
#include "DIA_flyDialog.h"
#include "DIA_flyDialogQt4.h"
  ADM_flyDialogQt4::ADM_flyDialogQt4(uint32_t width, uint32_t height, AVDMGenericVideoStream *in,
                              void *canvas, void *slider, int yuv, ResizeMethod resizeMethod):
                                ADM_flyDialog(width,height,in,canvas,slider,yuv,resizeMethod) 
{
        EndConstructor();
}

void ADM_flyDialogQt4::postInit(uint8_t reInit)
{
	QWidget *graphicsView = ((ADM_QCanvas*)_canvas)->parentWidget();
	QSlider  *slider=(QSlider *)_slider;
	
	graphicsView->setMinimumSize(_w, _h);
	graphicsView->resize(_w, _h);
	uint32_t nbFrames=_in->getInfo()->nb_frames; 
	slider->setMaximum(nbFrames);
}

float ADM_flyDialogQt4::calcZoomFactor(void) {return 1;}

uint8_t  ADM_flyDialogQt4::display(void)
{
   ADM_QCanvas *view=(ADM_QCanvas *)_canvas;
   ADM_assert(view);
   view->dataBuffer=_rgbBufferOut;
   if(!_rgbBufferOut)
   {
      printf("flyDialog: No rgbuffer ??\n"); 
   } 
   view->repaint();
  return 1; 
}
uint32_t ADM_flyDialogQt4::sliderGet(void)
{
  QSlider  *slide=(QSlider *)_slider;
  ADM_assert(slide);
  return slide->value();
  
}
uint8_t     ADM_flyDialogQt4::sliderSet(uint32_t value)
{
  QSlider  *slide=(QSlider *)_slider;
  ADM_assert(slide);
  slide->setValue(value);
  return 1; 
}
uint8_t  ADM_flyDialogQt4::isRgbInverted(void)
{
  return 1; 
}


//EOF
