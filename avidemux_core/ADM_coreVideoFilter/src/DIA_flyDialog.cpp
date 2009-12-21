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
        _resizer=NULL;
        _rgbBufferDisplay=NULL;

	_rgb=NULL;

	_yuvBuffer=new ADMImage(_w,_h);

	if(_isYuvProcessing)
	{
		_yuvBufferOut=new ADMImage(_w,_h);
		_rgbBuffer=NULL;
	}
	else
	{
		_rgbBuffer =new uint8_t [_w*_h*4];
		_yuvBufferOut=NULL;
	}

	_rgbBufferOut =new uint8_t [_w*_h*4];

	

	
}
/**
 *      \fn Endconstructor
 *      \brief We call some virtual functions in the constructor; it does not work, so we do the 2nd part of the constructor here
 */
void ADM_flyDialog::EndConstructor(void)
  {
    if (isRgbInverted())
                _rgb=new ColYuvRgb(_w,_h,1);
        else
                _rgb=new ColYuvRgb(_w,_h);

        _rgb->reset(_w,_h);
        if (_resizeMethod == RESIZE_AUTO || _resizeMethod == RESIZE_LAST)
                {
                        _zoom = calcZoomFactor();

                        if (_zoom == 1)
                                _resizeMethod = RESIZE_NONE;
                        else
                        {
                                _zoomW = uint32_t (_w * _zoom);
                                _zoomH = uint32_t (_h * _zoom);
                        }
                }
                else
                        _zoom = 1;

                if (_resizeMethod == RESIZE_AUTO || _resizeMethod == RESIZE_LAST)
                {
                        PixelFormat sourceColour;

                        if (_resizeMethod == RESIZE_AUTO || _isYuvProcessing)
                                sourceColour = PIX_FMT_YUV420P;
                        else
                                sourceColour = PIX_FMT_RGB32;

                        _resizer = new ADMImageResizer(_w, _h, _zoomW, _zoomH, sourceColour, PIX_FMT_RGB32);
                        _rgbBufferDisplay = new uint8_t[_w * _h * 4];
                }
                else
                {
                        _zoomW = _w;
                        _zoomH = _h;

                        _resizer = NULL;
                        _rgbBufferDisplay = NULL;
                }
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
        printf ("Resisting zoom size change from %dx%d (zoom %.5f) to %dx%d (zoom %.5f)\n",
                _zoomW, _zoomH, _zoom, new_zoomW, new_zoomH, new_zoom);
        return;
    }

    printf ("Fixing zoom size from %dx%d (zoom %.5f) to correct %dx%d (zoom %.5f)\n",
            _zoomW, _zoomH, _zoom, new_zoomW, new_zoomH, new_zoom);

    _resizeMethod = new_resizeMethod;
    _zoom = new_zoom;
    _zoomW = new_zoomW;
    _zoomH = new_zoomH;

    delete _resizer;
    if (_resizeMethod == RESIZE_AUTO || _resizeMethod == RESIZE_LAST)
    {
        PixelFormat sourceColour;

        if (_resizeMethod == RESIZE_AUTO || _isYuvProcessing)
            sourceColour = PIX_FMT_YUV420P;
        else
            sourceColour = PIX_FMT_RGB32;

        _resizer = new ADMImageResizer(_w, _h, _zoomW, _zoomH, sourceColour, PIX_FMT_RGB32);
        if (!_rgbBufferDisplay)
            _rgbBufferDisplay = new uint8_t[_w * _h * 4];
    }
    else
    {
        _resizer = NULL;
        delete _rgbBufferDisplay;
        _rgbBufferDisplay = NULL;
    }

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
  
	DEL2(_yuvBufferOut);
	DEL2(_yuvBuffer);
	DEL1(_rgbBuffer);
	DEL1(_rgbBufferOut);
	DEL1(_rgbBufferDisplay);
	DEL2(_rgb);
	DEL2(_resizer);
}
/**
    \fn ~ADM_flyDialog
    \brief destructor
*/
ADM_flyDialog::~ADM_flyDialog(void)
{
  
  // FIXME cleanup2();
  cleanup(); 
}

/**
      \fn sliderChanged
      \brief callback to handle image changes
*/
uint8_t    ADM_flyDialog::sliderChanged(void)
{
  uint32_t fn= sliderGet();
  uint32_t len,flags;
  
    ADM_assert(_yuvBuffer);
    ADM_assert(_rgbBufferOut);
    ADM_assert(_in);

#warning    DISABLED

    //if(!_in->getFrameNumberNoAlloc(fn,&len,_yuvBuffer,&flags))
    {
      printf("[FlyDialog] Cannot get frame %u\n",fn); 
      return 0;
    }

    // Process...    
    if(_isYuvProcessing)
    {
        process();
		copyYuvFinalToRgb();
    }
	else // RGB Processing      
    {
        ADM_assert(_rgbBuffer);
		copyYuvScratchToRgb();
        process();
    }

    return display();
}
/**
    \fn    copyYuvFinalToRgb
    \brief
*/

void ADM_flyDialog::copyYuvFinalToRgb(void)
{
	if (_resizeMethod == RESIZE_AUTO || _resizeMethod == RESIZE_LAST)
		_resizer->resize(_yuvBufferOut->data, _rgbBufferOut);
	else
		_rgb->scale(_yuvBufferOut->data, _rgbBufferOut);
}
/**
    \fn    copyYuvScratchToRgb
    \brief
*/

void ADM_flyDialog::copyYuvScratchToRgb(void)
{
	if (_resizeMethod == RESIZE_AUTO)
		_resizer->resize(_yuvBuffer->data,_rgbBuffer);
	else
		_rgb->scale(_yuvBuffer->data,_rgbBuffer);
}
/**
    \fn    copyRgbFinalToDisplay
    \brief
*/

void ADM_flyDialog::copyRgbFinalToDisplay(void)
{
	if (_resizeMethod == RESIZE_LAST)
	{
		_resizer->resize(_rgbBufferOut, _rgbBufferDisplay);

		uint8_t* tempRgb = _rgbBufferDisplay;

		_rgbBufferDisplay = _rgbBufferOut;
		_rgbBufferOut = tempRgb;
	}
}

//EOF

