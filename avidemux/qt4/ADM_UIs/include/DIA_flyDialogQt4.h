/** **************************************************************************
        \fn DIA_flyDialogQt4.h
 copyright            : (C) 2007 by mean
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


#include "ADM_UIQT46_export.h"
#include "ADM_image.h"

#include "ADM_coreVideoFilter6_export.h"


#include <QWidget>
#include <QDialog>
#include <QSlider>

#include "ADM_default.h"
#include "ADM_rgb.h"
#include "ADM_colorspace.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_imageResizer.h"
#define ADM_FLY_SLIDER_MAX 1000

enum ResizeMethod 
{
    RESIZE_NONE = 0,    // No automatic resize
    RESIZE_AUTO = 1,    // Resize image when convenient (YUV: after filter, RGB: before applying filter)
    RESIZE_LAST = 2        // Resize image after filter has been applied (slower for RGB)
};

class ADM_flyDialog;



class ADM_UIQT46_EXPORT ADM_QCanvas : public QWidget
{
protected:
	uint32_t _w,_h;
public:
	uint8_t *dataBuffer;

	ADM_QCanvas(QWidget *z, uint32_t w, uint32_t h);
	~ADM_QCanvas();
	void paintEvent(QPaintEvent *ev);
	void changeSize(uint32_t w, uint32_t h);
};



/**
    \class ADM_flyDialogAction
*/
class ADM_flyDialogAction
{
protected:
            ADM_flyDialog *parent;
            ADM_flyDialogAction(ADM_flyDialog *parent) {this->parent=parent;}
public:
  virtual   ~ADM_flyDialogAction() {}
  virtual    bool process(void)=0;
  virtual    void resetScaler(void)=0;
};

class ADM_flyDialogActionYuv: public ADM_flyDialogAction
{
protected:
          ADMImage              *_yuvBufferOut;
          ADMColorScalerFull    *yuvToRgb;
public:
            ADM_flyDialogActionYuv(ADM_flyDialog *parent);
            ~ADM_flyDialogActionYuv();
        void resetScaler(void);
        bool process(void);
};

class ADM_flyDialogActionRgb: public ADM_flyDialogAction
{
protected:
          ADM_byteBuffer     _rgbByteBuffer;
          ADM_byteBuffer     _rgbByteBufferOut;
          ADMColorScalerFull *yuv2rgb;
          ADMColorScalerFull *rgb2rgb;
public:
            ADM_flyDialogActionRgb(ADM_flyDialog *parent);
            ~ADM_flyDialogActionRgb();
        bool process(void);
        void resetScaler(void);
};
//***************************************
typedef float gfloat;





/**
    \class ADM_flyDialog
    \brief Base class for flyDialog
*/
class ADM_COREVIDEOFILTER6_EXPORT ADM_flyDialog
{
    friend class ADM_flyDialogAction;
    friend class ADM_flyDialogActionYuv;
    friend class ADM_flyDialogActionRgb;
  protected:
   virtual ADM_colorspace toRgbColor(void);
          void          updateZoom(void);
          ADM_flyDialogAction *action;
          uint64_t      _currentPts;
          uint32_t      _w, _h, _zoomW, _zoomH;
          float         _zoom;
          uint32_t      _zoomChangeCount;
          ADM_coreVideoFilter *_in;
      
          ADMImage      *_yuvBuffer;
          ADM_byteBuffer _rgbByteBufferDisplay;


          
          uint8_t       _isYuvProcessing;
          ResizeMethod  _resizeMethod;
  
          void EndConstructor(void);
          
  public:
          void               recomputeSize(void);
          virtual bool       disableZoom(void);
          virtual bool       nextImage(void);
          virtual bool       sameImage(void);
  public:
          void    *_cookie; // whatever
          void    *_slider; // widget
          void    *_canvas; // Drawing zone
          
  
  
  /* Filter dependant : you have to implement them*/
  virtual uint8_t    processYuv(ADMImage* in, ADMImage *out) {ADM_assert(0);return 1;}
  virtual uint8_t    processRgb(uint8_t *in, uint8_t *out) {ADM_assert(0);return 1;}
  virtual uint8_t    download(void)=0;
  virtual uint8_t    upload(void)=0;
  virtual bool       setCurrentPts(uint64_t pts)=0;
  /* /filter dependant */
  
  
  virtual uint8_t  update(void) {return 1;};
            uint8_t  cleanup(void);
  


          ADM_flyDialog(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                             void *canvas, void *slider, int yuv, 
                             ResizeMethod resizeMethod);
  virtual ~ADM_flyDialog(void);
// UI dependant part : They are implemented in ADM_flyDialogGtk/Qt/...
public:  
  virtual bool     goToTime(uint64_t tme);
  virtual bool     isRgbInverted(void)=0;
  virtual uint8_t  display(uint8_t *rgbData)=0;
  virtual float    calcZoomFactor(void)=0;  
  virtual uint32_t sliderGet(void)=0;             // Return the slider value between 0 and ADM_FLY_SLIDER_MAX
  virtual uint8_t  sliderSet(uint32_t value) =0;  // Set slider value between 0 and ADM_FLY_SLIDE_MAX
  virtual void     postInit(uint8_t reInit)=0;
  virtual uint8_t  sliderChanged(void);
};


class FlyDialogEventFilter : public QObject
{
	ADM_flyDialog *flyDialog;
	bool recomputed;

public:
	FlyDialogEventFilter(ADM_flyDialog *flyDialog);

protected:
	bool eventFilter(QObject *obj, QEvent *event);
};

class ADM_UIQT46_EXPORT ADM_flyDialogQt4 : public ADM_flyDialog
{
public:
  ADM_flyDialogQt4(uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              ADM_QCanvas *canvas, QSlider *slider, int yuv, ResizeMethod resizeMethod);

  
  virtual bool     isRgbInverted(void);
  virtual uint8_t  display(uint8_t *rgbData);
  virtual float    calcZoomFactor(void);
  virtual uint32_t sliderGet(void);
  virtual uint8_t  sliderSet(uint32_t value);
  virtual void     postInit(uint8_t reInit);
    bool           setCurrentPts(uint64_t pts) {return 1;}
};
