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
#include <QImage>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "ADM_default.h"
#include "ADM_rgb.h"
#include "ADM_colorspace.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_imageResizer.h"
#define ADM_FLY_SLIDER_MAX 1000
#define ADM_FLYRGB_ALGO_CHANGE_THRESHOLD_RESOLUTION 720

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
    AnalyzerBtn = 1 << 1,
    UserWidgetAfterControls = 1 << 2,
    UserWidgetBeforePeekBtn = 1 << 3,
    UserWidgetAfterPeekBtn = 1 << 4
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
public:
        uint8_t *dataBuffer;

        ADM_QCanvas(QWidget *z, uint32_t w, uint32_t h);
        ~ADM_QCanvas();
        void paintEvent(QPaintEvent *ev);
        void changeSize(uint32_t w, uint32_t h);
        void getDisplaySize(uint32_t *w, uint32_t *h);
};

class flyControl;
class ADM_analyzerDialog;
class flyDialogsAnalyzer;
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
          bool         _bypassFilter;
      // Analyzer
          bool                 _analyze;
          ADM_analyzerDialog * _analyzerDialog;
          QGraphicsScene     * _analyzerScenes[4];
          flyDialogsAnalyzer * _flyAnalyzer;



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
          virtual bool       sameImage(void);
                  uint64_t   getCurrentPts();
          ADM_coreVideoFilter *getUnderlyingFilter() {return _in;}
                  bool        addControl(QHBoxLayout *layout, ControlOption controlOptions = ControlOption::None, QWidget * userWidget = NULL);
protected:
  virtual ADM_pixelFormat     toRgbPixFrmt(void);
          void               updateZoom(void);
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
  virtual bool       bandMoved(int x, int y, int w, int h) { return true; }


// UI dependant part : They are implemented in ADM_flyDialogGtk/Qt/...
public:  

  virtual bool     isRgbInverted(void);
  virtual uint8_t  display(uint8_t *rgbData);
  virtual float    calcZoomFactor(void);  
  virtual uint32_t sliderGet(void);             // Return the slider value between 0 and ADM_FLY_SLIDER_MAX
  virtual uint8_t  sliderSet(uint32_t value);  // Set slider value between 0 and ADM_FLY_SLIDE_MAX

// Either refreshImage(), sliderChanged() or gotoSelectionStart() must be called
// before valueChanged(int) signal from the slider is connected to its slot.

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
        virtual void play(bool status);
        virtual void analyzerReleased(void);
        virtual void analyzerClosed(void);
        virtual void peekOriginalPressed(void);
        virtual void peekOriginalReleased(void);
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
                    ADMColorScaler_algo _algo;
                    uint64_t           _scaledPts;
public:
                    ADM_byteBuffer     _rgbByteBuffer;
                    ADM_byteBuffer     _rgbByteBufferOut;
                    ADMColorScalerFull *yuv2rgb;
                    ADMColorScalerFull *rgb2rgb;
public:
          virtual    bool process(void);
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

class ADM_UIQT46_EXPORT ADM_analyzerDialog : public QDialog
{
    Q_OBJECT
  private:
    QHBoxLayout     * hboxlayout;
    QVBoxLayout     * vboxlayout;
    QGraphicsView   * gv[4];
    QPushButton     * btns[4];
    bool              btnChkd[4];
    void adjustGraphs();
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
  private slots:
    void btnToggled(bool f);
  public:
    QGraphicsScene  * gsc[4];
    ADM_analyzerDialog(QWidget *parent);
    ~ADM_analyzerDialog();
    
};

class ADM_UIQT46_EXPORT flyDialogsAnalyzer
{
  private:
    int                  width, height;
    int                  rgbBufStride;
    ADM_byteBuffer *     rgbBufRaw;
    ADMColorScalerFull * convertYuvToRgb;
    uint32_t * wrkVectorScope;
    uint32_t * bufVectorScope;
    uint32_t * scaleVectorScope;
    QImage   * imgVectorScope;
    uint32_t * wrkYUVparade[3];
    uint32_t * bufYUVparade;
    QImage   * imgYUVparade;
    uint32_t * wrkRGBparade[3];
    uint32_t * bufRGBparade;
    QImage   * imgRGBparade;
    uint32_t * wrkHistograms[6];
    uint32_t * bufHistograms;
    QImage   * imgHistograms;
    int      * paradeIndex;
    int      * paradeIndexHalf;
  public:
    flyDialogsAnalyzer(int width, int height);
    virtual    ~flyDialogsAnalyzer() ;
    void analyze(ADMImage *in, QGraphicsScene * sceneVectorScope, QGraphicsScene * sceneYUVparade, QGraphicsScene * sceneRGBparade, QGraphicsScene * sceneHistograms);
};

//EOF
