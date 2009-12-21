/***************************************************************************
    \file T_flyDialogQt4.cpp
    \brief QT4 variabt of fly dialogs
    \author mean (c) 2006, fixounet@free.fr
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************///


#include <QtCore/QEvent>
#include <QtCore/QCoreApplication>
#include <QtGui/QGraphicsView>
#include <QtGui/QSlider>

#include "ADM_default.h"
#include "DIA_flyDialog.h"
#include "DIA_flyDialogQt4.h"

extern float UI_calcZoomToFitScreen(QWidget* window, QWidget* canvas, uint32_t imageWidth, uint32_t imageHeight);
extern uint8_t UI_getPhysicalScreenSize(void* window, uint32_t *w, uint32_t *h);
/**
    \fn    FlyDialogEventFilter
    \brief
*/

FlyDialogEventFilter::FlyDialogEventFilter(ADM_flyDialog *flyDialog)
{
	recomputed = false;
	this->flyDialog = flyDialog;
}
/**
    \fn    eventFilter
    \brief
*/

bool FlyDialogEventFilter::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() == QEvent::Show && !recomputed)
	{
		recomputed = true;
		QWidget* parent = (QWidget*)obj;
		uint32_t screenWidth, screenHeight;

		UI_getPhysicalScreenSize(parent, &screenWidth, &screenHeight);
		flyDialog->recomputeSize();
		QCoreApplication::processEvents();
		parent->move((((int)screenWidth) - parent->frameSize().width()) / 2, (((int)screenHeight) - parent->frameSize().height()) / 2);
	}

	return QObject::eventFilter(obj, event);
}
/**
    \fn    ADM_flyDialog
    \brief
*/

  ADM_flyDialogQt4::ADM_flyDialogQt4(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              void *canvas, void *slider, int yuv, ResizeMethod resizeMethod):
                                ADM_flyDialog(width,height,in,canvas,slider,yuv,resizeMethod) 
{
        EndConstructor();
}
/**
    \fn    postInit
    \brief
*/

void ADM_flyDialogQt4::postInit(uint8_t reInit)
{
	QWidget *graphicsView = ((ADM_QCanvas*)_canvas)->parentWidget();
	QSlider  *slider=(QSlider *)_slider;

	if (!reInit)
	{
		FlyDialogEventFilter *eventFilter = new FlyDialogEventFilter(this);

		if (slider)
			slider->setMaximum(ADM_FLY_SLIDER_MAX);

		graphicsView->parentWidget()->installEventFilter(eventFilter);
	}

	((ADM_QCanvas*)_canvas)->changeSize(_zoomW, _zoomH);
	graphicsView->setMinimumSize(_zoomW, _zoomH);
	graphicsView->resize(_zoomW, _zoomH);
}
/**
    \fn    calcZoomFactor
    \brief
*/

float ADM_flyDialogQt4::calcZoomFactor(void)
{
	return UI_calcZoomToFitScreen(((ADM_QCanvas*)_canvas)->parentWidget()->parentWidget(), ((ADM_QCanvas*)_canvas)->parentWidget(), _w, _h);
}
/**
    \fn    display
    \brief
*/

uint8_t  ADM_flyDialogQt4::display(void)
{
   ADM_QCanvas *view=(ADM_QCanvas *)_canvas;
   ADM_assert(view);
   view->dataBuffer=_rgbBufferOut;
   if(!_rgbBufferOut)
   {
      ADM_info("flyDialog: No rgbuffer ??\n"); 
   } 
   view->repaint();
  return 1; 
}
/**
    \fn    sliderGet
    \brief
*/

uint32_t ADM_flyDialogQt4::sliderGet(void)
{
  QSlider  *slide=(QSlider *)_slider;
  ADM_assert(slide);
  return slide->value();
  
}
/**
    \fn    sliderSet
    \brief
*/

uint8_t     ADM_flyDialogQt4::sliderSet(uint32_t value)
{
  QSlider  *slide=(QSlider *)_slider;
  ADM_assert(slide);
  if(value>ADM_FLY_SLIDER_MAX) value=ADM_FLY_SLIDER_MAX;
  slide->setValue(value);
  return 1; 
}
/**
    \fn    isRgbInverted
    \brief
*/
uint8_t  ADM_flyDialogQt4::isRgbInverted(void)
{
  return 1; 
}


//EOF
