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

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#define ADM_FLY_INTERNAL
#include "DIA_flyDialogQt4.h"
#include "ADM_assert.h"
#include <QtCore/QEvent>
#include <QtCore/QCoreApplication>
#include <QGraphicsView>
#include <QSlider>
#include "ADM_toolkitQt.h"
extern "C" {
#include "libavcodec/avcodec.h"
}

/**
    \fn updateZoom
*/
void ADM_flyDialog::updateZoom(void)
{   
        _rgbByteBufferDisplay.clean();
        _rgbByteBufferDisplay.setSize(_zoomW * _zoomH * 4);
        resetScaler();
}
/**
 * 
 * @return 
 */
bool ADM_flyDialog::disableZoom()
{
    _resizeMethod = RESIZE_NONE;
    _zoom = 1;
    _zoomW = _w;
    _zoomH = _h;  
    updateZoom();
    sliderChanged();
    return true;
}

/**
    \fn    recomputeSize
    \brief recompute zoom factor
*/

void ADM_flyDialog::recomputeSize(void)
{
    float new_zoom = calcZoomFactor();

    ResizeMethod new_resizeMethod;
    uint32_t new_zoomW;
    uint32_t new_zoomH;

    if (new_zoom == 1)
    {
        new_resizeMethod = RESIZE_NONE;
        new_zoomW = _w;
        new_zoomH = _h;
    }
    else
    {
        new_resizeMethod = RESIZE_AUTO;
        new_zoomW = uint32_t (_w * new_zoom);
        new_zoomH = uint32_t (_h * new_zoom);
    }

    if (new_resizeMethod == _resizeMethod && new_zoom == _zoom
        && new_zoomW == _zoomW && new_zoomH == _zoomH)
        return;

    if (++_zoomChangeCount > 3 || new_zoomH < 30 || new_zoomW < 30)
    {
        ADM_info ("Resisting zoom size change from %dx%d (zoom %.5f) to %dx%d (zoom %.5f)\n",
                _zoomW, _zoomH, _zoom, new_zoomW, new_zoomH, new_zoom);
        return;
    }

    ADM_info ("Fixing zoom size from %dx%d (zoom %.5f) to correct %dx%d (zoom %.5f)\n",
            _zoomW, _zoomH, _zoom, new_zoomW, new_zoomH, new_zoom);

    _resizeMethod = new_resizeMethod;
    _zoom = new_zoom;
    _zoomW = new_zoomW;
    _zoomH = new_zoomH;
    updateZoom();
    postInit (true);
    sliderChanged();
}

/**
    \fn cleanup
    \brief deallocate
*/
uint8_t ADM_flyDialog::cleanup(void)
{
#define DEL1(x)    if(x) {delete [] x;x=NULL;}
#define DEL2(x)    if(x) {delete  x;x=NULL;}
	DEL2(_yuvBuffer);
	_rgbByteBufferDisplay.clean();
	return 1;
}
/**
    \fn ~ADM_flyDialog
    \brief destructor
*/
ADM_flyDialog::~ADM_flyDialog(void)
{
  cleanup(); 
}

/**
      \fn sliderChanged
      \brief callback to handle image changes
*/
bool    ADM_flyDialog::goToTime(uint64_t tme)
{
     _in->goToTime(tme);
     return nextImage();
}
uint8_t    ADM_flyDialog::sliderChanged(void)
{
  uint32_t fn= sliderGet();
  uint32_t frameNumber;
  uint32_t len,flags;
  
    ADM_assert(_yuvBuffer);
    ADM_assert(_in);


    double time;
    time=fn;
    time/=ADM_FLY_SLIDER_MAX;
    time*=_in->getInfo()->totalDuration;
    return goToTime(time);
    
}
/**
    \fn toRgbColor
*/
ADM_colorspace ADM_flyDialog::toRgbColor(void)
{
    if(isRgbInverted()) return ADM_COLOR_BGR32A;
    return ADM_COLOR_RGB32A;
}
/**
    \fn sameImage
*/
bool ADM_flyDialog::sameImage(void)
{
    process();
    return display(_rgbByteBufferDisplay.at(0));
}
/**
    \fn nextImage
*/
bool ADM_flyDialog::nextImage(void)
{
    uint32_t frameNumber;
    if(!_in->getNextFrame(&frameNumber,_yuvBuffer))
    {
      ADM_warning("[FlyDialog] Cannot get frame %u\n",frameNumber); 
      return 0;
    }
    setCurrentPts(_yuvBuffer->Pts);
    // Process...   
    process();
    return display(_rgbByteBufferDisplay.at(0));
}
//************************************
// Implement the specific part
// i.e. yuv processing or RGB processing
//************************************
  ADM_flyDialogYuv::ADM_flyDialogYuv(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, QSlider *slider, 
                                ResizeMethod resizeMethod) : ADM_flyDialog(width,height,in,canvas,slider,resizeMethod)
{
        _yuvBufferOut=new ADMImageDefault(_w,_h);
        yuvToRgb=NULL;  
        updateZoom();
        postInit(false);
}
void ADM_flyDialogYuv::resetScaler(void)
{
    if(yuvToRgb) delete yuvToRgb;
    yuvToRgb=NULL;
    yuvToRgb=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                            _w,
                            _h,
                            _zoomW,
                            _zoomH,
                            ADM_COLOR_YV12,toRgbColor());
}
ADM_flyDialogYuv::~ADM_flyDialogYuv()
{
    if(_yuvBufferOut) delete _yuvBufferOut;
    _yuvBufferOut=NULL;
}
bool ADM_flyDialogYuv::process(void)
{
         processYuv(_yuvBuffer,_yuvBufferOut);
        yuvToRgb->convertImage(_yuvBufferOut,_rgbByteBufferDisplay.at(0));
        return true;
}
//*****************************************
ADM_flyDialogRgb::ADM_flyDialogRgb(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, QSlider *slider, 
                                ResizeMethod resizeMethod) : ADM_flyDialog(width,height,in,canvas,slider,resizeMethod)
{
    uint32_t size=_w*_h*4;
    _rgbByteBuffer.setSize(size);
    _rgbByteBufferOut.setSize(size);
     yuv2rgb =new ADMColorScalerSimple(_w,_h,ADM_COLOR_YV12,
                toRgbColor());
    rgb2rgb=NULL;
    updateZoom();
    postInit(false);

}
void ADM_flyDialogRgb::resetScaler(void)
{
    if(rgb2rgb) delete rgb2rgb;
    rgb2rgb=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                            _w,
                            _h,
                            _zoomW,
                            _zoomH,
                            ADM_COLOR_RGB32A,ADM_COLOR_RGB32A);
}
/**
 * 
 */
ADM_flyDialogRgb::~ADM_flyDialogRgb()
{
    _rgbByteBuffer.clean();
    _rgbByteBufferOut.clean();
    if(rgb2rgb) delete rgb2rgb;
    if(yuv2rgb) delete yuv2rgb;
    rgb2rgb=NULL;
    yuv2rgb=NULL;
    
}
bool ADM_flyDialogRgb::process(void)
{
    yuv2rgb->convertImage(_yuvBuffer,_rgbByteBuffer.at(0));
    if (_resizeMethod != RESIZE_NONE)
    {
        processRgb(_rgbByteBuffer.at(0),_rgbByteBufferOut.at(0));
        rgb2rgb->convert(_rgbByteBufferOut.at(0), _rgbByteBufferDisplay.at(0));
    }else
    {
        processRgb(_rgbByteBuffer.at(0),_rgbByteBufferDisplay.at(0));
    }
    return true;
}

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
    updateZoom();
    postInit(false);

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


//******************************
//EOF

