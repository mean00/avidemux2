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
#include <QGraphicsView>
#include <QSlider>

#include "ADM_default.h"
#include "DIA_flyDialogQt4.h"
#include "ADM_toolkitQt.h"

extern float UI_calcZoomToFitScreen(QWidget* window, QWidget* canvas, uint32_t imageWidth, uint32_t imageHeight);

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

  ADM_flyDialog::ADM_flyDialog(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              ADM_QCanvas *canvas, QSlider *slider,  ResizeMethod resizeMethod)
{  
    ADM_assert(canvas);

    if (slider)
            ADM_assert(in);

    _w = width;
    _h = height;    
    _in = in;
    _slider = slider;
    _canvas = canvas;
    _cookie = NULL;
    _resizeMethod = resizeMethod;
    _zoomChangeCount = 0;        
    _yuvBuffer=new ADMImageDefault(_w,_h);
    _currentPts=0;	
  
    if (_resizeMethod != RESIZE_NONE) 
    {
        _zoom = calcZoomFactor();
        if (_zoom == 1) 
        {
            _resizeMethod = RESIZE_NONE;
        }
    }
    if (_resizeMethod == RESIZE_NONE) 
    {
        _zoom = 1;
        _zoomW = _w;
        _zoomH = _h;
    } else 
    {
        _zoomW = uint32_t(_w * _zoom);
        _zoomH = uint32_t(_h * _zoom);
    }

}
/**
    \fn    postInit
    \brief
*/

void ADM_flyDialog::postInit(uint8_t reInit)
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

float ADM_flyDialog::calcZoomFactor(void)
{
	return UI_calcZoomToFitScreen(((ADM_QCanvas*)_canvas)->parentWidget()->parentWidget(), ((ADM_QCanvas*)_canvas)->parentWidget(), _w, _h);
}
/**
    \fn    display
    \brief
*/

uint8_t  ADM_flyDialog::display(uint8_t *rgbData)
{
   ADM_QCanvas *view=(ADM_QCanvas *)_canvas;
   ADM_assert(view);
   view->dataBuffer=rgbData;
   if(!rgbData)
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

uint32_t ADM_flyDialog::sliderGet(void)
{
  QSlider  *slide=(QSlider *)_slider;
  ADM_assert(slide);
  return slide->value();
  
}
/**
    \fn    sliderSet
    \brief
*/

uint8_t     ADM_flyDialog::sliderSet(uint32_t value)
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
bool  ADM_flyDialog::isRgbInverted(void)
{
  return 0; 
}


//EOF
