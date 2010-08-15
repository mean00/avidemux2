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

#include "ADM_includeFfmpeg.h"
#include "ADM_default.h"

#include "ADM_coreVideoFilter.h"
#define ADM_FLY_INTERNAL
#include "DIA_flyDialog.h"
#include "ADM_assert.h"

extern "C" {
#include "ADM_ffmpeg/libavcodec/avcodec.h"
}
/**
    \fn    ADM_flyDialog
    \brief Constructor
*/
ADM_flyDialog::ADM_flyDialog(uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                void *canvas, void *slider,int yuv, ResizeMethod resizeMethod)
{
	ADM_assert(canvas);

	if (slider)
		ADM_assert(in);

	_w = width;
	_h = height;
	_isYuvProcessing = yuv;
	_in = in;
	_slider = slider;
	_canvas = canvas;
	_cookie = NULL;
	_resizeMethod = resizeMethod;
    _zoomChangeCount = 0;
    _rgbBufferDisplay=NULL;

	_yuvBuffer=new ADMImageDefault(_w,_h);

	_currentPts=0;	
}
/**
    \fn updateZoom
*/
void ADM_flyDialog::updateZoom(void)
{   
        if(_rgbBufferDisplay) delete _rgbBufferDisplay;
        _rgbBufferDisplay = new uint8_t[_zoomW * _zoomH * 4];
        action->resetScaler();
}

/**
 *      \fn Endconstructor
 *      \brief We need to call some virtual functions in the constructor; 
        it does not work, so we do the 2nd part of the constructor here
 */
void ADM_flyDialog::EndConstructor(void)
 {
	if(_isYuvProcessing)
	{
        action=new ADM_flyDialogActionYuv(this);
	}
	else
	{
		action=new ADM_flyDialogActionRgb(this);
	}

        if (_resizeMethod != RESIZE_NONE)
        {
                _zoom = calcZoomFactor();
                if (_zoom == 1)
                {
                        _resizeMethod = RESIZE_NONE;
                }
        }
        if(_resizeMethod==RESIZE_NONE)
        {
            _zoom=1;
            _zoomW=_w;
            _zoomH=_h;
        }
        else
        {
                _zoomW = uint32_t (_w * _zoom);
                _zoomH = uint32_t (_h * _zoom);
        }
        updateZoom();
        postInit (false);
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
	DEL1(_rgbBufferDisplay);
    DEL2(action);
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
    _in->goToTime(time);
   

   return nextImage();
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
    action->process();
    return display(_rgbBufferDisplay);
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
    action->process();
    return display(_rgbBufferDisplay);
}
//************************************
// Implement the specific part
// i.e. yuv processing or RGB processing
//************************************
ADM_flyDialogActionYuv::ADM_flyDialogActionYuv(ADM_flyDialog *p) : 
                     ADM_flyDialogAction(p)
{
        _yuvBufferOut=new ADMImageDefault(parent->_w,parent->_h);
        yuvToRgb=NULL;
        
}
void ADM_flyDialogActionYuv::resetScaler(void)
{
    if(yuvToRgb) delete yuvToRgb;
    yuvToRgb=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                            parent->_w,
                            parent->_h,
                            parent->_zoomW,
                            parent->_zoomH,
                            ADM_COLOR_YV12,parent->toRgbColor());
}
ADM_flyDialogActionYuv::~ADM_flyDialogActionYuv()
{
    if(_yuvBufferOut) delete _yuvBufferOut;
    _yuvBufferOut=NULL;
}
bool ADM_flyDialogActionYuv::process(void)
{
        parent-> processYuv(parent->_yuvBuffer,_yuvBufferOut);
        yuvToRgb->convertImage(_yuvBufferOut,parent->_rgbBufferDisplay);
        return true;
}
//*****************************************
ADM_flyDialogActionRgb::ADM_flyDialogActionRgb(ADM_flyDialog *p) :  ADM_flyDialogAction(p)
{
    uint32_t size=parent->_w*parent->_h*4;
    _rgbBuffer=new uint8_t [size];
    _rgbBufferOut=new uint8_t [size];
     yuv2rgb =new ADMColorScalerSimple(parent->_w,parent->_h,ADM_COLOR_YV12,
                parent->toRgbColor());
    rgb2rgb=NULL;
}
void ADM_flyDialogActionRgb::resetScaler(void)
{
    if(rgb2rgb) delete rgb2rgb;
    rgb2rgb=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                            parent->_w,
                            parent->_h,
                            parent->_zoomW,
                            parent->_zoomH,
                            ADM_COLOR_RGB32A,ADM_COLOR_RGB32A);
}
ADM_flyDialogActionRgb::~ADM_flyDialogActionRgb()
{
    if(_rgbBuffer) delete [] _rgbBuffer;
    if(_rgbBufferOut) delete [] _rgbBufferOut;
    if(rgb2rgb) delete rgb2rgb;
    if(yuv2rgb) delete yuv2rgb;
    rgb2rgb=NULL;
    yuv2rgb=NULL;
    _rgbBuffer=NULL;
    _rgbBufferOut=NULL;
    
}
bool ADM_flyDialogActionRgb::process(void)
{
    yuv2rgb->convertImage(parent->_yuvBuffer,_rgbBuffer);
    if (parent->_resizeMethod != RESIZE_NONE)
    {
        parent->processRgb(_rgbBuffer,_rgbBufferOut);
        rgb2rgb->convert(_rgbBufferOut, parent->_rgbBufferDisplay);
    }else
    {
        parent->processRgb(_rgbBuffer,parent->_rgbBufferDisplay);
    }
    return true;
}
//******************************
//EOF

