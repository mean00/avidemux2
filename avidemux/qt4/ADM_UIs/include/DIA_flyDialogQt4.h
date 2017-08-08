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
#include "ADM_toolkitQt.h"

#include <QWidget>
#include <QDialog>
#include <QFrame>
#include <QTimer>
#include <QDialog>
#include <QLabel>
#include <QRubberBand>

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

class QHBoxLayout;
class QPushButton;
class QRadioButton;


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

class flyControl;
/**
    \class ADM_flyDialog
    \brief Base class for flyDialog
*/
class ADM_UIQT46_EXPORT ADM_flyDialog : public QObject
{
  Q_OBJECT
 protected:     
          QTimer        timer;
          uint32_t      _w, _h, _zoomW, _zoomH;
          float         _zoom;
          uint32_t      _zoomChangeCount;
          ResizeMethod  _resizeMethod;
          uint64_t      lastPts;
          double        _computedZoom;
          int           _usedWidth, _usedHeight;
          int           _frameIncrement; // time between image in ms
          Clock         _clock;
          int           _nextRdv;

          ADM_coreVideoFilter *_in;

          ADMImage      *_yuvBuffer;
          ADM_byteBuffer _rgbByteBufferDisplay;

          flyControl  *_control;
          
          QDialog     *_parent;



  public:
          void          *_cookie; // whatever, usually the ui_xxx component
          ADM_QSlider   *_slider; // widget
          ADM_QCanvas   *_canvas; // Drawing zone

  public:
                            ADM_flyDialog(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                 ADM_QCanvas *canvas, ADM_QSlider *slider, ResizeMethod resizeMethod);
          virtual           ~ADM_flyDialog(void);    
          void               recomputeSize(void);
          virtual bool       disableZoom(void);
          virtual bool       enableZoom(void);          
          virtual bool       sameImage(void);
                  uint64_t   getCurrentPts();
          ADM_coreVideoFilter *getUnderlyingFilter() {return _in;}
                  bool        addControl(QHBoxLayout *layout);
protected:
  virtual ADM_colorspace     toRgbColor(void);
          void               updateZoom(void);
          void               EndConstructor(void);
          uint8_t            cleanup(void);  
          bool               initializeSize();
          float              calcZoomToBeDisplayable(uint32_t imageWidth, uint32_t imageHeight);
  /* Filter dependant : you have to implement them*/
//
           virtual           void resetScaler(void)=0; // dont bother, implemented by yuv or rgb subclass
           virtual           bool process(void)=0; // dont bother, implemented by yuv or rgb subclass
public:

  virtual uint8_t    download(void)=0;
  virtual uint8_t    upload(void)=0;
  //virtual uint8_t  update(void)=0;

  virtual bool       setCurrentPts(uint64_t pts) {return true;};
  virtual bool       bandResized(int x, int y, int w, int h) { return true; }


// UI dependant part : They are implemented in ADM_flyDialogGtk/Qt/...
public:  

  virtual bool     isRgbInverted(void);
  virtual uint8_t  display(uint8_t *rgbData);
  virtual float    calcZoomFactor(void);  
  virtual uint32_t sliderGet(void);             // Return the slider value between 0 and ADM_FLY_SLIDER_MAX
  virtual uint8_t  sliderSet(uint32_t value);  // Set slider value between 0 and ADM_FLY_SLIDE_MAX
  virtual void     postInit(uint8_t reInit);
public:  
  virtual uint8_t  sliderChanged(void);
  virtual void     updateSlider(void);
  virtual bool     goToTime(uint64_t tme);

private:
  virtual bool     nextImageInternal(void);

public slots:
        virtual bool nextImage(void);
        virtual void backOneMinute(void);
        virtual void fwdOneMinute(void);
        virtual void play(bool status);
        virtual void timeout(void);
        virtual void adjustCanvasPosition(void);
        virtual void fitCanvasIntoView(uint32_t width, uint32_t height);
};
/**
 * \class ADM_flyDialogYuv
 */
class ADM_UIQT46_EXPORT ADM_flyDialogYuv: public  ADM_flyDialog
{
  Q_OBJECT
public:
                    ADMImage              *_yuvBufferOut;
                    ADMColorScalerFull    *yuvToRgb;  
public:
          virtual    bool process(void);
          virtual    void resetScaler(void);

                                ADM_flyDialogYuv(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, ADM_QSlider *slider,
                                ResizeMethod resizeMethod);
            virtual             ~ADM_flyDialogYuv();
            virtual uint8_t    processYuv(ADMImage* in, ADMImage *out) =0;

};

/**
 * \class ADM_flyDialogRgb
 */
class ADM_UIQT46_EXPORT ADM_flyDialogRgb: public  ADM_flyDialog
{
  Q_OBJECT
public:
                    ADM_byteBuffer     _rgbByteBuffer;
                    ADM_byteBuffer     _rgbByteBufferOut;
                    ADMColorScalerFull *yuv2rgb;
                    ADMColorScalerFull *rgb2rgb;
public:
          virtual    bool process(void);
          virtual    void resetScaler(void);

                            ADM_flyDialogRgb(QDialog *parent,uint32_t width, uint32_t height, 
                                 ADM_coreVideoFilter *in,  ADM_QCanvas *canvas,  ADM_QSlider *slider, ResizeMethod resizeMethod);
           virtual          ~ADM_flyDialogRgb();
           virtual uint8_t  processRgb(uint8_t *in, uint8_t *out) =0;
};

/**
 * \fn ADM_flyDialogYuv
 */
class ADM_UIQT46_EXPORT FlyDialogEventFilter : public QObject
{
        ADM_flyDialog *flyDialog;
        bool recomputed;

public:
        FlyDialogEventFilter(ADM_flyDialog *flyDialog);

protected:
        bool eventFilter(QObject *obj, QEvent *event);
};

/**
    \fn ADM_QRubberBand
    \brief Override platform-dependent appearance of QRubberBand
*/
class ADM_UIQT46_EXPORT ADM_QRubberBand : public QRubberBand
{
public:
        ADM_QRubberBand(QWidget *parent);
        ~ADM_QRubberBand();
private:
        void paintEvent(QPaintEvent *event);
};

/**
    \fn ADM_rubberControl
    \brief http://stackoverflow.com/questions/19066804/implementing-resize-handles-on-qrubberband-is-qsizegrip-relevant
*/
class ADM_UIQT46_EXPORT ADM_rubberControl : public QWidget
{
public:
        ADM_rubberControl(ADM_flyDialog *fly, QWidget *parent);
        ADM_flyDialog *flyParent;
        int nestedIgnore;

public:
        ADM_QRubberBand *rubberband;
        void blockSignals(bool block)
        {
            rubberband->blockSignals(block);
        }
private:
        void resizeEvent(QResizeEvent *);
};

//EOF
