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
#include <QMouseEvent>
#include <QPoint>
#include <QRect>
#include <QGraphicsScene>

#include "ADM_default.h"
#include "ADM_rgb.h"
#include "ADM_colorspace.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_imageResizer.h"
#define ADM_FLY_SLIDER_MAX 1000
#define ADM_FLYRGB_ALGO_CHANGE_THRESHOLD_RESOLUTION 720
#define ADN_FLYRGB_USE_FASTER_RGB2RGB_SCALER

enum ResizeMethod 
{
    RESIZE_NONE = 0,    // No automatic resize
    RESIZE_AUTO = 1,    // Resize image when convenient (YUV: after filter, RGB: before applying filter)
    RESIZE_LAST = 2        // Resize image after filter has been applied (slower for RGB)
};

enum class ControlOption
{
    None = 0,
    PeekOriginalBtn = 1 << 0,
    UserWidgetAfterControls = 1 << 1,
    UserWidgetBeforePeekBtn = 1 << 2,
    UserWidgetAfterPeekBtn = 1 << 3
};

inline ControlOption operator|(ControlOption a, ControlOption b)
{
    return static_cast<ControlOption>(static_cast<int>(a) | static_cast<int>(b));
}
inline ControlOption operator+(ControlOption a, ControlOption b)
{
    return static_cast<ControlOption>(static_cast<int>(a) + static_cast<int>(b));
}
inline int operator&(ControlOption a, ControlOption b)
{
    return (static_cast<int>(a) & static_cast<int>(b));
}

class ADM_flyDialog;

class QHBoxLayout;
class QPushButton;
class QRadioButton;

class ADM_UIQT46_EXPORT ADM_QCanvas : public QWidget
{
protected:
        uint32_t _w,_h;
        uint32_t _l; // bytes per line
        void *accel;
public:
        uint8_t *dataBuffer;

        ADM_QCanvas(QWidget *z, uint32_t w, uint32_t h);
virtual ~ADM_QCanvas();
virtual bool initAccel(bool inputIsYuv = true);
virtual void uninitAccel(void);
virtual bool displayImage(ADMImage *pic);
        void paintEvent(QPaintEvent *ev);
        void changeSize(uint32_t w, uint32_t h);
        void getDisplaySize(uint32_t *w, uint32_t *h);
};

/**
 * \class FlyDialogEventFilter
 * \brief Default handling for show and resize events
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
          uint32_t      _w, _h, _zoomW, _zoomH, _inW, _inH;
          float         _zoom;
          ResizeMethod  _resizeMethod;
          uint64_t      lastPts;
          double        _computedZoom;
          int           _usedWidth, _usedHeight;
          int           _oldViewWidth, _oldViewHeight;
          int           _frameIncrement; // time between image in ms
          Clock         _clock;
          int           _nextRdv;

          ADM_coreVideoFilter *_in;

          ADMImage      *_yuvBuffer;
          ADM_byteBuffer _rgbByteBufferDisplay;

          flyControl  *_control;
          std::vector<QWidget *> buttonList; // useful for manipulating tab order
          QDialog     *_parent;
          FlyDialogEventFilter *_eventFilter;
          bool         _bypassFilter;
          bool         _reprocess;
          bool         _gotPic;
          bool         _darkMode;

  public:
          void          *_cookie; // whatever, usually the ui_xxx component
          ADM_flyNavSlider   *_slider; // widget
          ADM_QCanvas   *_canvas; // Drawing zone

  public:
                            ADM_flyDialog(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                 ADM_QCanvas *canvas, ADM_flyNavSlider *slider, ResizeMethod resizeMethod);
          virtual           ~ADM_flyDialog(void);    
          void               recomputeSize(void);
          virtual bool       disableZoom(void);
          virtual bool       enableZoom(void);          
          virtual bool       sameImage(bool reprocess = true);
                  uint64_t   getCurrentPts();
          ADM_coreVideoFilter *getUnderlyingFilter() {return _in;}
                  bool        addControl(QHBoxLayout *layout, ControlOption controlOptions = ControlOption::None, QWidget * userWidget = NULL);
protected:
  virtual ADM_pixelFormat     toRgbPixFrmt(void);
  virtual void               updateZoom(void) = 0;
          uint8_t            cleanup(void);  
          bool               initializeSize();
          float              calcZoomToBeDisplayable(uint32_t imageWidth, uint32_t imageHeight);
          void               clearEventFilter(void);

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
  virtual bool       bandMoved(int x, int y, int w, int h) { return true; }


// UI dependant part : They are implemented in ADM_flyDialogGtk/Qt/...
public:  

  virtual bool     isRgbInverted(void);
  virtual bool     display(void) = 0;
  virtual float    calcZoomFactor(void);  
  virtual uint32_t sliderGet(void);             // Return the slider value between 0 and ADM_FLY_SLIDER_MAX
  virtual uint8_t  sliderSet(uint32_t value);  // Set slider value between 0 and ADM_FLY_SLIDE_MAX
  virtual void     adjustCanvasPosition(void);
  virtual void     fitCanvasIntoView(uint32_t width, uint32_t height);

  virtual bool     refreshImage(void);
  virtual bool     sliderChanged(void);
  virtual void     updateSlider(void);
  virtual bool     goToTime(uint64_t tme);
  virtual bool     goToExactTime(uint64_t tme);

private:
  virtual bool     nextImageInternal(void);

public slots:
        virtual bool nextImage(void);
        virtual void backOneMinute(void);
        virtual void fwdOneMinute(void);
        virtual void gotoSelectionStart(void);
        virtual void gotoSelectionEnd(void);
        virtual void play(bool status);
        virtual void peekOriginalPressed(void);
        virtual void peekOriginalReleased(void);
        virtual void timeout(void);
};
/**
 * \class ADM_flyDialogYuv
 */
class ADM_UIQT46_EXPORT ADM_flyDialogYuv: public  ADM_flyDialog
{
  Q_OBJECT
protected:
                    int                  accelCanvasFlags;
                    ADMImage              *_yuvBufferOut;
                    ADMColorScalerFull    *yuvToRgb;  
                    ADMColorScalerFull    *yuvInputToRgb;  

            virtual void updateZoom(void);

public:
          virtual    bool process(void);
          virtual    bool display(void);
          virtual    void resetScaler(void);

                                ADM_flyDialogYuv(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, ADM_flyNavSlider *slider,
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
protected:
                    int                 accelCanvasFlags;
                    ADMColorScaler_algo _algo;
                    uint64_t           _scaledPts;

                    ADM_byteBuffer     _rgbByteBuffer;
                    ADM_byteBuffer     _rgbByteBufferOut;
                    ADMColorScalerFull *yuv2rgb;
#ifdef ADN_FLYRGB_USE_FASTER_RGB2RGB_SCALER
                    ADMRGB32Scaler     *rgb2rgb;
#else
                    ADMColorScalerFull     *rgb2rgb;
#endif
            virtual void updateZoom(void);

public:
          virtual    bool process(void);
          virtual    bool display(void);
          virtual    void resetScaler(void);

                            ADM_flyDialogRgb(QDialog *parent,uint32_t width, uint32_t height, 
                                 ADM_coreVideoFilter *in,  ADM_QCanvas *canvas,  ADM_flyNavSlider *slider, ResizeMethod resizeMethod);
           virtual          ~ADM_flyDialogRgb();
           virtual uint8_t  processRgb(uint8_t *in, uint8_t *out) =0;
};

/**
    \fn ADM_QRubberBand
    \brief Override platform-dependent appearance of QRubberBand
*/
class ADM_UIQT46_EXPORT ADM_QRubberBand : public QRubberBand
{
public:
        typedef enum {
             ADM_RUBBER_BAND_GRIPS_NONE=0,
             ADM_RUBBER_BAND_GRIPS_FIRST=1,
             ADM_RUBBER_BAND_GRIPS_SECOND=2,
             ADM_RUBBER_BAND_GRIPS_MASK=3
        } ADM_rubberBandFlags;

        ADM_QRubberBand(QWidget *parent);
        ~ADM_QRubberBand();
        void drawGrips(int flags) { mode = (ADM_rubberBandFlags)(flags & ADM_RUBBER_BAND_GRIPS_MASK); }
private:
        ADM_rubberBandFlags mode;
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
        void sizeGripEnable(bool topLeftEnabled, bool bottomRightEnabled);
private:
        QWidget * rubberControlParent;
        void *grip1ptr;
        void *grip2ptr;
        bool drag;
        QPoint dragOffset;
        QRect dragGeometry;
        void resizeEvent(QResizeEvent *);
        void showEvent(QShowEvent *);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        void enterEvent(QEvent *);
#else
        void enterEvent(QEnterEvent *);
#endif
        void leaveEvent(QEvent *);
        void mousePressEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);
};

//EOF
