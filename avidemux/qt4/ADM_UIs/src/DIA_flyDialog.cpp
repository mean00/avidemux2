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

#include <QEvent>
#include <QGraphicsView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QLineEdit>
#include <QFontMetrics>
#include <QRect>
#include <QColor>

#include <cmath>

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_flyDialogQt4.h"

#include "ADM_toolkitQt.h"
#include "ADM_vidMisc.h"
#include "prefs.h"

#define ACCEL_CANVAS_FLAG_PROBED 1
#define ACCEL_CANVAS_FLAG_USABLE 2

/**
 * \fn      FlyDialogEventFilter
 * \brief   ctor
 */
FlyDialogEventFilter::FlyDialogEventFilter(ADM_flyDialog *flyDialog)
{
    recomputed = false;
    this->flyDialog = flyDialog;
}
/**
 * \fn      eventFilter
 * \brief   Default handling of show and resize events
 */
bool FlyDialogEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch(event->type())
    {
        case QEvent::Show:
            if (!recomputed)
            {
                recomputed = true;
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                flyDialog->adjustCanvasPosition();
                QWidget *view = flyDialog->_canvas->parentWidget();
                view->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
                flyDialog->refreshImage();
                QApplication::restoreOverrideCursor();
            }
            break;
        case QEvent::Resize:
            if (flyDialog->_canvas->height() > 0)
            {
                QWidget *view = flyDialog->_canvas->parentWidget();
                flyDialog->fitCanvasIntoView(view->width(), view->height());
                flyDialog->adjustCanvasPosition();
            }
            break;
        default:break;
    }

    return QObject::eventFilter(obj, event);
}

/**
 */
class flyControl
{
public:
        flyControl(QHBoxLayout *horizontalLayout_4, ControlOption controlOptions, QWidget * userWidget)
        {
            
            pushButton_back1mn = new QPushButton();
            pushButton_back1mn->setObjectName(QString("pushButton_back1mn"));
            pushButton_back1mn->setAutoRepeat(true);
            pushButton_back1mn->setAutoRepeatDelay(1000);

            horizontalLayout_4->addWidget(pushButton_back1mn);

            pushButton_gotosel_start = new QPushButton();
            pushButton_gotosel_start->setObjectName(QString("pushButton_gotosel_start"));

            horizontalLayout_4->addWidget(pushButton_gotosel_start);

            pushButton_gotosel_end = new QPushButton();
            pushButton_gotosel_end->setObjectName(QString("pushButton_gotosel_end"));

            horizontalLayout_4->addWidget(pushButton_gotosel_end);

            pushButton_play = new QPushButton();
            pushButton_play->setObjectName(QString("pushButton_play"));
            pushButton_play->setCheckable(true);

            horizontalLayout_4->addWidget(pushButton_play);

            pushButton_next = new QPushButton();
            pushButton_next->setObjectName(QString("pushButton_next"));
            pushButton_next->setAutoRepeat(true);
            pushButton_next->setAutoRepeatDelay(1000);

            horizontalLayout_4->addWidget(pushButton_next);

            pushButton_fwd1mn = new QPushButton();
            pushButton_fwd1mn->setObjectName(QString("pushButton_fwd1mn"));
            pushButton_fwd1mn->setAutoRepeat(true);
            pushButton_fwd1mn->setAutoRepeatDelay(1000);

            horizontalLayout_4->addWidget(pushButton_fwd1mn);
            
            //
            QString zeros = "00:00:00.000";
            currentTime = new QLineEdit(zeros);
            currentTime->setReadOnly(true);
            currentTime->setAlignment(Qt::AlignCenter);
#ifdef USE_CUSTOM_TIME_DISPLAY_FONT
            currentTime->setFont(QFont("ADM7SEG"));
#endif
            int ctWidth = 1.15 * currentTime->fontMetrics().boundingRect(zeros).width();
            currentTime->setMaximumWidth(ctWidth);
            currentTime->setMinimumWidth(ctWidth);
            currentTime->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

            labelDuration = new QLabel();
            labelDuration->setText(QString("/ ") + zeros);

            horizontalLayout_4->addWidget(currentTime);
            horizontalLayout_4->addWidget(labelDuration);

            if (controlOptions & ControlOption::UserWidgetAfterControls)
            {
                ADM_assert(userWidget != NULL);
                horizontalLayout_4->addWidget(userWidget);
                userWidget = NULL;
            }

            QSpacerItem  *horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
            horizontalLayout_4->addItem(horizontalSpacer_4);

            if (controlOptions & ControlOption::UserWidgetBeforePeekBtn)
            {
                ADM_assert(userWidget != NULL);
                horizontalLayout_4->addWidget(userWidget);
                userWidget = NULL;
            }
            
            if (controlOptions & ControlOption::PeekOriginalBtn)
            {
                pushButton_peekOriginal = new QPushButton();
                pushButton_peekOriginal->setObjectName(QString("pushButton_peekOriginal"));
                pushButton_peekOriginal->setAutoRepeat(false);
                pushButton_peekOriginal->setText(QApplication::translate("seekablePreviewDialog", "Peek Original", 0));
                pushButton_peekOriginal->setToolTip(QApplication::translate("seekablePreviewDialog", "Show unprocessed input", 0));

                horizontalLayout_4->addWidget(pushButton_peekOriginal);
            }
            else
                pushButton_peekOriginal = NULL;
            
            if (controlOptions & ControlOption::UserWidgetAfterPeekBtn)
            {
                ADM_assert(userWidget != NULL);
                horizontalLayout_4->addWidget(userWidget);
                userWidget = NULL;
            }

            ADM_assert(userWidget == NULL);	// should have been added to layout

            pushButton_back1mn->setToolTip(QApplication::translate("seekablePreviewDialog", "Back one minute", 0));
            pushButton_back1mn->setText(QApplication::translate("seekablePreviewDialog", "<<", 0));
            pushButton_gotosel_start->setText(QApplication::translate("seekablePreviewDialog", "A", 0));
            pushButton_gotosel_start->setToolTip(QApplication::translate("seekablePreviewDialog", "Go to the start of the selection", 0));
            pushButton_gotosel_end->setText(QApplication::translate("seekablePreviewDialog", "B", 0));
            pushButton_gotosel_end->setToolTip(QApplication::translate("seekablePreviewDialog", "Go to the end of the selection", 0));
            pushButton_play->setText(QApplication::translate("seekablePreviewDialog", "Play", 0));
            pushButton_next->setToolTip(QApplication::translate("seekablePreviewDialog", "Next image", 0));
            pushButton_next->setText(QApplication::translate("seekablePreviewDialog", ">", 0));
            pushButton_fwd1mn->setText(QApplication::translate("seekablePreviewDialog", ">>", 0));
            pushButton_fwd1mn->setToolTip(QApplication::translate("seekablePreviewDialog", "Forward one minute", 0));

            QSize bsize = pushButton_gotosel_start->sizeHint();
            if(bsize.isValid())
            {
                int w = bsize.width()/2;
                pushButton_gotosel_start->setMaximumWidth(w);
                pushButton_gotosel_end->setMaximumWidth(w);
            }
        }
        void disableButtons()
        {
            pushButton_gotosel_start->setEnabled(false);
            pushButton_gotosel_end->setEnabled(false);
            pushButton_back1mn->setEnabled(false);
            pushButton_fwd1mn->setEnabled(false);
            pushButton_next->setEnabled(false);
        }
        void enableButtons()
        {
            pushButton_gotosel_start->setEnabled(true);
            pushButton_gotosel_end->setEnabled(true);
            pushButton_back1mn->setEnabled(true);
            pushButton_fwd1mn->setEnabled(true);
            pushButton_next->setEnabled(true);
        }
public:
        QPushButton *pushButton_gotosel_start;
        QPushButton *pushButton_gotosel_end;
        QPushButton *pushButton_back1mn;
        QPushButton *pushButton_play;
        QPushButton *pushButton_next;
        QPushButton *pushButton_fwd1mn;
        QLineEdit   *currentTime;
        QLabel      *labelDuration;
        QPushButton *pushButton_peekOriginal;
};

/**
 * 
 * @return 
 */
bool ADM_flyDialog::disableZoom()
{
    _resizeMethod = RESIZE_NONE;
    recomputeSize();
    return true;
}
bool       ADM_flyDialog::enableZoom(void)
{
      _resizeMethod = RESIZE_AUTO;
    recomputeSize();
    return true;
}
/**
    \fn    recomputeSize
    \brief recompute zoom factor
*/

void ADM_flyDialog::recomputeSize(void)
{
    if(this->_resizeMethod==RESIZE_NONE)
    {
        _zoom = 1;
        _zoomW = _w;
        _zoomH = _h;
        _canvas->changeSize(_zoomW, _zoomH);
        _canvas->parentWidget()->setMinimumSize(_zoomW, _zoomH);
        updateZoom();
        refreshImage();
        return;
    }
    
    float new_zoom = calcZoomFactor();

    ResizeMethod new_resizeMethod;
    uint32_t new_zoomW;
    uint32_t new_zoomH;

    new_zoomW = uint32_t (_w * new_zoom);
    new_zoomH = uint32_t (_h * new_zoom);
    
    if ( new_zoom == _zoom && new_zoomW == _zoomW && new_zoomH == _zoomH)
        return;

    if ( new_zoomH < 30 || new_zoomW < 30)
    {
        ADM_info ("Resisting zoom size change from %dx%d (zoom %.5f) to %dx%d (zoom %.5f)\n",
                _zoomW, _zoomH, _zoom, new_zoomW, new_zoomH, new_zoom);
        return;
    }

    ADM_info ("Fixing zoom size from %dx%d (zoom %.5f) to correct %dx%d (zoom %.5f)\n",
            _zoomW, _zoomH, _zoom, new_zoomW, new_zoomH, new_zoom);
    _zoom = new_zoom;
    _zoomW = new_zoomW;
    _zoomH = new_zoomH;
    _canvas->changeSize(_zoomW, _zoomH);
    _canvas->parentWidget()->setMinimumSize(_zoomW, _zoomH);
    updateZoom();
    refreshImage();
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
    if(_control)
    {
        delete _control;
        _control=NULL;
    }
    clearEventFilter();
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
    \fn goToTime
*/
bool    ADM_flyDialog::goToTime(uint64_t tme)
{
    _in->goToTime(tme);
    return nextImageInternal();
}

/**
    \fn goToExactTime
*/
bool ADM_flyDialog::goToExactTime(uint64_t tme)
{
    _in->goToTime(tme,true);
    return nextImageInternal();
}

/**
    \fn refresh
    \brief Seek to the current position, re-process the image and sync the slider
*/
bool ADM_flyDialog::refreshImage(void)
{
    if(goToExactTime(lastPts))
    {
        updateSlider();
        return true;
    }
    return false;
}

/**
    \fn sliderChanged
    \brief callback to handle image changes
*/
bool ADM_flyDialog::sliderChanged(void)
{
    uint32_t fn= sliderGet();
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
    \fn toRgbPixFrmt
*/
ADM_pixelFormat ADM_flyDialog::toRgbPixFrmt(void)
{
    if(isRgbInverted()) return ADM_PIXFRMT_BGR32A;
    return ADM_PIXFRMT_RGB32A;
}
/**
 * 
 * @param frame
 * @return 
 */
bool ADM_flyDialog::addControl(QHBoxLayout *horizontalLayout_4, ControlOption controlOptions, QWidget * userWidget)
{
    _parent->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
    _control=new flyControl(horizontalLayout_4, controlOptions, userWidget);
    _parent->adjustSize(); // force currentTime size calculation
    _control->currentTime->setTextMargins(0,0,0,0); // counteract Adwaita messing with text margins

    QObject::connect(_control->pushButton_next ,SIGNAL(clicked()),this,SLOT(nextImage()));
    QObject::connect(_control->pushButton_back1mn ,SIGNAL(clicked()),this,SLOT(backOneMinute()));
    QObject::connect(_control->pushButton_fwd1mn ,SIGNAL(clicked()),this,SLOT(fwdOneMinute()));
    QObject::connect(_control->pushButton_gotosel_start,SIGNAL(clicked()),this,SLOT(gotoSelectionStart()));
    QObject::connect(_control->pushButton_gotosel_end,SIGNAL(clicked()),this,SLOT(gotoSelectionEnd()));
    QObject::connect(_control->pushButton_play ,SIGNAL(toggled(bool )),this,SLOT(play(bool)));
    if (controlOptions & ControlOption::PeekOriginalBtn)
    {
        QObject::connect(_control->pushButton_peekOriginal ,SIGNAL(pressed()),this,SLOT(peekOriginalPressed()));
        QObject::connect(_control->pushButton_peekOriginal ,SIGNAL(released()),this,SLOT(peekOriginalReleased()));
    }


    buttonList.push_back(_control->pushButton_back1mn);
    buttonList.push_back(_control->pushButton_gotosel_start);
    buttonList.push_back(_control->pushButton_gotosel_end);
    buttonList.push_back(_control->pushButton_play);
    buttonList.push_back(_control->pushButton_next);
    buttonList.push_back(_control->pushButton_fwd1mn);
    buttonList.push_back(_control->currentTime);
    if ((controlOptions & ControlOption::UserWidgetAfterControls) && (userWidget != NULL))
        buttonList.push_back(userWidget);
    if ((controlOptions & ControlOption::UserWidgetBeforePeekBtn) && (userWidget != NULL))
        buttonList.push_back(userWidget);
    if (controlOptions & ControlOption::PeekOriginalBtn)
        buttonList.push_back(_control->pushButton_peekOriginal);
    if ((controlOptions & ControlOption::UserWidgetAfterPeekBtn) && (userWidget != NULL))
        buttonList.push_back(userWidget);

    return true;
}

/**
    \fn sameImage
*/
bool ADM_flyDialog::sameImage(bool reprocess)
{
    if(!_gotPic)
        return true;
    _reprocess = reprocess;
    process();
    return display();
}
uint64_t ADM_flyDialog::getCurrentPts()
{
    return lastPts;
}
/**
    \fn nextImageInternal
*/
bool ADM_flyDialog::nextImageInternal(void)
{
    uint32_t frameNumber;
    if(!_in->getNextFrame(&frameNumber,_yuvBuffer))
    {
        ADM_warning("[FlyDialog] Cannot get frame %u\n",frameNumber); 
        return 0;
    }
    _gotPic = true;
    lastPts=_yuvBuffer->Pts;
    setCurrentPts(lastPts);
    uint64_t duration=_in->getInfo()->totalDuration;
    if(_control)
    {
        char text[80];
        uint32_t mm,hh,ss,ms;
        uint32_t milly = lastPts/1000;

        ms2time(milly,&hh,&mm,&ss,&ms);
        sprintf(text, "%02d:%02d:%02d.%03d", hh, mm, ss, ms);
        _control->currentTime->setText(text);

        milly = duration/1000;
        ms2time(milly,&hh,&mm,&ss,&ms);
        sprintf(text, "/ %02d:%02d:%02d.%03d", hh, mm, ss, ms);
        _control->labelDuration->setText(text);
    }
    // Process...
    return sameImage(true);
}

/**
    \fn nextImage
*/
bool ADM_flyDialog::nextImage(void)
{
    ADM_flyNavSlider  *slide=(ADM_flyNavSlider *)_slider;
    ADM_assert(slide);
    bool oldState=slide->blockSignals(true);
    bool r=nextImageInternal();
    if(r)
        updateSlider();
    slide->blockSignals(oldState);
    return r;
}

/**
 * \fn initializeSize
 * \brief Get the width and height of the parts of the dialog above and below the canvas
 */
bool ADM_flyDialog::initializeSize(void)
{
    _canvas->resize(1,1);
    _canvas->parentWidget()->parentWidget()->adjustSize();
    QSize qsize= _canvas->parentWidget()->parentWidget()->frameSize();
    // Normally there is nothing interesting left and right, we can use a hardcoded value
    _usedWidth = 32;
    _usedHeight = qsize.height();
    if (_usedHeight > 0) _usedHeight--;

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
    
    ADM_info("xAutoZoom : base size= %d x %d\n",_usedWidth,_usedHeight);
    return true;
}
/**
 * \brief Calculate the zoom ratio required to fit the whole image on the screen.
 * @param imageWidth
 * @param imageHeight
 * @return 
 */
float ADM_flyDialog::calcZoomToBeDisplayable( uint32_t imageWidth, uint32_t imageHeight)
{
    uint32_t screenWidth, screenHeight;
    QWidget *topWindow=_canvas->parentWidget()->parentWidget();
    UI_getPhysicalScreenSize(topWindow, &screenWidth, &screenHeight);

    // Usable width/height
    int usableWidth =(int)screenWidth -_usedWidth;
    int usableHeight=(int)screenHeight-_usedHeight;

    if(usableWidth<160) usableWidth=160;
    if(usableHeight<160) usableHeight=160;

    float widthRatio  = (float)usableWidth / (float)imageWidth;
    float heightRatio = (float)usableHeight / (float)imageHeight;

    ADM_info("autoZoom : Raw w=%f h=%f\n",widthRatio,heightRatio);

    float r= (widthRatio < heightRatio ? widthRatio : heightRatio);
    return r;
}

//************************************
// Implement the specific part
// i.e. yuv processing or RGB processing
//************************************
  ADM_flyDialogYuv::ADM_flyDialogYuv(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, ADM_flyNavSlider *slider,
                                ResizeMethod resizeMethod) : ADM_flyDialog(parent,width,height,in,canvas,slider,resizeMethod)
{
    _control=NULL;
    _yuvBufferOut=new ADMImageDefault(_w,_h);
    yuvInputToRgb = NULL;
    yuvToRgb=NULL;  
    accelCanvasFlags = 0;
    initializeSize();
    _canvas->parentWidget()->setMinimumSize(_zoomW, _zoomH);
    updateZoom();
}
void ADM_flyDialogYuv::resetScaler(void)
{
    if(yuvInputToRgb) 
    {
        delete yuvInputToRgb;
        yuvInputToRgb=NULL;
    }    
    if(yuvToRgb) 
    {
        delete yuvToRgb;
        yuvToRgb=NULL;
    }
    
    uint32_t displayW, displayH;
    _canvas->getDisplaySize(&displayW, &displayH);

    yuvInputToRgb=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                            _inW,
                            _inH,
                            displayW,
                            displayH,
                            ADM_PIXFRMT_YV12,toRgbPixFrmt());    
    yuvToRgb=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                            _w,
                            _h,
                            displayW,
                            displayH,
                            ADM_PIXFRMT_YV12,toRgbPixFrmt());
}
/**
 * 
 */
ADM_flyDialogYuv::~ADM_flyDialogYuv()
{
    if(yuvToRgb)
    {
        delete yuvToRgb;
        yuvToRgb=NULL;
    }
    if(_yuvBufferOut) delete _yuvBufferOut;
    _yuvBufferOut=NULL;
    if(_control)
    {
        buttonList.clear();
        delete _control;
        _control=NULL;
    }
    if (yuvInputToRgb)
    {
        delete yuvInputToRgb;
        yuvInputToRgb = NULL;    
    }
}
bool ADM_flyDialogYuv::process(void)
{
    if (!_bypassFilter)
    {
        if (_reprocess)
        {
            processYuv(_yuvBuffer,_yuvBufferOut);
            _reprocess = false;
        }
    }
    if (accelCanvasFlags & ACCEL_CANVAS_FLAG_USABLE)
        return true;
    if (_bypassFilter)
    {
        yuvInputToRgb->convertImage(_yuvBuffer, _rgbByteBufferDisplay.at(0));
    }
    else
    {
        yuvToRgb->convertImage(_yuvBufferOut, _rgbByteBufferDisplay.at(0));
    }
    return true;
}
/**
    \fn display
*/
bool ADM_flyDialogYuv::display(void)
{
    if(!_gotPic)
        return true;
    ADM_QCanvas *v = _canvas;
    if (!(accelCanvasFlags & ACCEL_CANVAS_FLAG_PROBED) && v->isVisible())
    {
        accelCanvasFlags |= ACCEL_CANVAS_FLAG_PROBED;
        if (v->initAccel())
            accelCanvasFlags |= ACCEL_CANVAS_FLAG_USABLE;
    }
    if (accelCanvasFlags & ACCEL_CANVAS_FLAG_USABLE)
    {
        v->dataBuffer = NULL;
        if (v->displayImage(_bypassFilter ? _yuvBuffer : _yuvBufferOut))
            return true;
        ADM_warning("Disabling accelerated canvas\n");
        accelCanvasFlags &= ~ACCEL_CANVAS_FLAG_USABLE;
        updateZoom(); // Setup scaler and display buffer, they may be invalid.
        if (_bypassFilter)
        {
            yuvInputToRgb->convertImage(_yuvBuffer, _rgbByteBufferDisplay.at(0));
        }
        else
        {
            yuvToRgb->convertImage(_yuvBufferOut, _rgbByteBufferDisplay.at(0));
        }
    }
    v->dataBuffer = _rgbByteBufferDisplay.at(0);
    v->repaint();
    return true;
}
/**
    \fn updateZoom
*/
void ADM_flyDialogYuv::updateZoom(void)
{
    if (accelCanvasFlags & ACCEL_CANVAS_FLAG_USABLE)
        return;

    uint32_t displayW, displayH;
    _canvas->getDisplaySize(&displayW, &displayH);
    _rgbByteBufferDisplay.clean();
    _rgbByteBufferDisplay.setSize(ADM_IMAGE_ALIGN(displayW * 4) * displayH);
    resetScaler();
}
//*****************************************
ADM_flyDialogRgb::ADM_flyDialogRgb(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, ADM_flyNavSlider *slider,
                                ResizeMethod resizeMethod) : ADM_flyDialog(parent,width,height,in,canvas,slider,resizeMethod)
{
    uint32_t size = ADM_IMAGE_ALIGN(_w*4);
    size*=_h;
    _scaledPts = ADM_NO_PTS;
    _rgbByteBuffer.setSize(size);
    _rgbByteBufferOut.setSize(size);
    _algo = ((_h > ADM_FLYRGB_ALGO_CHANGE_THRESHOLD_RESOLUTION) ? ADM_CS_FAST_BILINEAR : ADM_CS_BICUBIC);
     yuv2rgb =  new ADMColorScalerFull(_algo, 
                            _inW,
                            _inH,
                            _w,
                            _h,
                            ADM_PIXFRMT_YV12,toRgbPixFrmt());
    rgb2rgb=NULL;
    accelCanvasFlags = 0;
    initializeSize();
    _canvas->parentWidget()->setMinimumSize(_zoomW, _zoomH);
    updateZoom();
}
void ADM_flyDialogRgb::resetScaler(void)
{
    if(rgb2rgb) delete rgb2rgb;

    uint32_t displayW, displayH;
    _canvas->getDisplaySize(&displayW, &displayH);
    
#ifdef ADN_FLYRGB_USE_FASTER_RGB2RGB_SCALER    
    rgb2rgb=new ADMRGB32Scaler(
#else
    rgb2rgb=new ADMColorScalerFull(
#endif            
                                _algo, 
                                _w,
                                _h,
                                displayW,
                                displayH,
                                ADM_PIXFRMT_RGB32A,ADM_PIXFRMT_RGB32A);
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
    if (_bypassFilter)
    {
        uint8_t *target = (accelCanvasFlags & ACCEL_CANVAS_FLAG_USABLE) ? _rgbByteBuffer.at(0) : _rgbByteBufferDisplay.at(0);
        yuv2rgb->convertImage(_yuvBuffer, target);
    } else {
        if (_reprocess || _scaledPts != lastPts)
        {
            yuv2rgb->convertImage(_yuvBuffer,_rgbByteBuffer.at(0));
            _scaledPts = lastPts;
            _reprocess = true;
        }
        if (_resizeMethod != RESIZE_NONE)
        {
            if (_reprocess)
            {
                processRgb(_rgbByteBuffer.at(0),_rgbByteBufferOut.at(0));
                _reprocess = false;
            }
            if (accelCanvasFlags & ACCEL_CANVAS_FLAG_USABLE)
                return true;
            rgb2rgb->convert(_rgbByteBufferOut.at(0), _rgbByteBufferDisplay.at(0));
        }else
        {
            processRgb(_rgbByteBuffer.at(0),_rgbByteBufferDisplay.at(0));
        }
    }
    return true;
}
/**
    \fn display
*/
bool ADM_flyDialogRgb::display(void)
{
    ADM_QCanvas *v = _canvas;
    if (!(accelCanvasFlags & ACCEL_CANVAS_FLAG_PROBED) && v->isVisible())
    {
        accelCanvasFlags |= ACCEL_CANVAS_FLAG_PROBED;
        if (v->initAccel(false))
            accelCanvasFlags |= ACCEL_CANVAS_FLAG_USABLE;
    }
    if (accelCanvasFlags & ACCEL_CANVAS_FLAG_USABLE)
    {
        v->dataBuffer = NULL;
        ADMImageRef ref(_w,_h);
        ref._planes[0] = _bypassFilter ? _rgbByteBuffer.at(0) : _rgbByteBufferOut.at(0);
        ref._planeStride[0] = ADM_IMAGE_ALIGN(_w * 4);
        if (v->displayImage(&ref))
            return true;
        ADM_warning("Disabling accelerated canvas\n");
        accelCanvasFlags &= ~ACCEL_CANVAS_FLAG_USABLE;
        updateZoom();
        rgb2rgb->convert(_bypassFilter ? _rgbByteBuffer.at(0) : _rgbByteBufferOut.at(0),
                         _rgbByteBufferDisplay.at(0));
    }
    _canvas->dataBuffer = _rgbByteBufferDisplay.at(0);
    _canvas->repaint();
    return true;
}
/**
    \fn updateZoom
*/
void ADM_flyDialogRgb::updateZoom(void)
{
    if (accelCanvasFlags & ACCEL_CANVAS_FLAG_USABLE)
        return;

    uint32_t displayW, displayH;
    _canvas->getDisplaySize(&displayW, &displayH);
    _rgbByteBufferDisplay.clean();
    _rgbByteBufferDisplay.setSize(ADM_IMAGE_ALIGN(displayW * 4) * displayH);
    resetScaler();
}
/*********************************************************/

/**
    \fn    ADM_flyDialog
    \brief
*/

  ADM_flyDialog::ADM_flyDialog(QDialog *parent ,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              ADM_QCanvas *canvas, ADM_flyNavSlider *slider,  ResizeMethod resizeMethod)
{  
    ADM_assert(canvas);
    {
        ADM_assert(in);
        slider->setMaximum(ADM_FLY_SLIDER_MAX);
    }
    _parent=parent;
    _w = width;
    _h = height;    
    _in = in;
    _slider = slider;
    _canvas = canvas;
    _cookie = NULL;
    _computedZoom=0;
    _resizeMethod = resizeMethod;
    _inW = in->getInfo()->width;
    _inH = in->getInfo()->height;
    _yuvBuffer=new ADMImageDefault(_inW,_inH);
    _usedWidth= _usedHeight=0;
    _oldViewWidth = _oldViewHeight = 0;
    _nextRdv=0;
    lastPts= _in->getInfo()->markerA;
    uint64_t startTime = _in->getAbsoluteStartTime();
    printf("[ADM_flyDialog::ctor] Bridge start time: %s\n",ADM_us2plain(startTime));
    if(lastPts > startTime)
        lastPts -= startTime;
    setCurrentPts(lastPts);
    // Seek is delegated to the user
    _bypassFilter=false;
    _reprocess = true;
    _gotPic = false;
    _darkMode = _parent->palette().base().color().value() < 128;

    QGraphicsScene *sc=new QGraphicsScene(this);
    sc->setBackgroundBrush(QBrush(Qt::darkGray, Qt::SolidPattern));
    qobject_cast<QGraphicsView*>(_canvas->parentWidget())->setScene(sc);
    qobject_cast<QFrame*>(_canvas->parentWidget())->setFrameStyle(QFrame::NoFrame);

    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));
    timer.setSingleShot(true);

    int incrementUs=getUnderlyingFilter()->getInfo()->frameIncrement;

    incrementUs=(incrementUs+501)/1000; // us => ms
    if(incrementUs<10) incrementUs=10;
    _frameIncrement=incrementUs;
    timer.setInterval(_frameIncrement);
    
    ADM_info("Interval = %d ms\n",incrementUs);
    timer.stop();
    
    bool swapWheel = false;
    prefs->get(FEATURES_SWAP_MOUSE_WHEEL,&swapWheel);
    slider->setInvertedWheel(swapWheel);
    slider->setMarkers(_in->getInfo()->totalDuration,_in->getInfo()->markerA,_in->getInfo()->markerB);

    _eventFilter = new FlyDialogEventFilter(this);
    _canvas->parentWidget()->parentWidget()->installEventFilter(_eventFilter);
}

/**
    \fn clearEventFilter
    \brief Convenience function for dialogs which need to handle show and resize events in a different way
*/
void ADM_flyDialog::clearEventFilter(void)
{
    if(!_eventFilter) return;

    _canvas->parentWidget()->parentWidget()->removeEventFilter(_eventFilter);

    if(_eventFilter)
    {
        delete _eventFilter;
        _eventFilter = NULL;
    }
}
/**
    \fn adjustCanvasPosition
    \brief center canvas within the viewport (graphicsView)
*/
void ADM_flyDialog::adjustCanvasPosition(void)
{
    uint32_t graphicsViewWidth = _canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = _canvas->parentWidget()->height();
    uint32_t canvasWidth = _canvas->width();
    uint32_t canvasHeight = _canvas->height();
    int h = 0;
    int v = 0;
    if(graphicsViewWidth > canvasWidth)
        h = (graphicsViewWidth - canvasWidth)/2;
    if(graphicsViewHeight > canvasHeight)
        v = (graphicsViewHeight - canvasHeight)/2;
    if(h||v)
        _canvas->move(h,v);
}

/**
    \fn fitCanvasIntoView
*/
void ADM_flyDialog::fitCanvasIntoView(uint32_t width, uint32_t height)
{
    double ar = (double)_w / _h;
    double viewAr = (double)width / height;

    uint32_t tmpZoomW = 0;
    uint32_t tmpZoomH = 0;
    bool skip = false;
    if(viewAr > ar)
    {
        tmpZoomW = (uint32_t)((double)height * ar);
        tmpZoomH = height;
        if(_oldViewHeight &&
           _oldViewHeight == height &&
           _oldViewHeight == _canvas->height())
            skip = true;
    }else
    {
        tmpZoomW = width;
        tmpZoomH = (uint32_t)((double)width / ar);
        if(_oldViewWidth &&
           _oldViewWidth == width &&
           _oldViewWidth == _canvas->width())
            skip = true;
    }
    _oldViewWidth = width;
    _oldViewHeight = height;
    if(skip) return;

    _resizeMethod = RESIZE_AUTO;
    _zoomW = tmpZoomW;
    _zoomH = tmpZoomH;
    _zoom = (float)_zoomW / _w;
    _canvas->changeSize(_zoomW, _zoomH);
    updateZoom();
    sameImage(_reprocess);
}

/**
    \fn    calcZoomFactor
    \brief
*/
float ADM_flyDialog::calcZoomFactor(void)
{
#define APPROXIMATE 20.
    if(_computedZoom) return _computedZoom;
    double zoom;
    zoom=calcZoomToBeDisplayable(_w, _h);
    // Find the closest integer
    // zoom it ?
    if((zoom)>1)
    {
        _computedZoom=1.; // never upscale automatically
        return _computedZoom;
    }
    double invertZoom=1/zoom;
    _computedZoom=APPROXIMATE/floor((1+APPROXIMATE*(invertZoom)));
    ADM_info("AutoZoom 1/%f\n",(float)(1./_computedZoom));
    return _computedZoom;
    
}

/**
    \fn    sliderGet
    \brief
*/

uint32_t ADM_flyDialog::sliderGet(void)
{
    ADM_flyNavSlider  *slide=(ADM_flyNavSlider *)_slider;
    ADM_assert(slide);
    return slide->value();
  
}
/**
    \fn    sliderSet
    \brief
*/

uint8_t     ADM_flyDialog::sliderSet(uint32_t value)
{
    ADM_flyNavSlider  *slide=(ADM_flyNavSlider *)_slider;
    ADM_assert(slide);
    if(value>ADM_FLY_SLIDER_MAX) value=ADM_FLY_SLIDER_MAX;
    slide->blockSignals(true);
    slide->setValue(value);
    slide->blockSignals(false);
    return 1;
}

/**
    \fn    updateSlider
    \brief
*/
void ADM_flyDialog::updateSlider(void)
{
    ADM_assert(_in);
    double pos=lastPts;
    pos/=_in->getInfo()->totalDuration;
    pos*=ADM_FLY_SLIDER_MAX;
    pos+=0.5; // round up
    sliderSet((uint32_t)pos);
}

/**
    \fn    isRgbInverted
    \brief
*/
bool  ADM_flyDialog::isRgbInverted(void)
{
    return 1;
}


/**
 * 
 */
#define JUMP_LENGTH (60LL*1000LL*1000LL)

void ADM_flyDialog::backOneMinute(void)
{
    uint64_t pts=getCurrentPts();
    if(pts<JUMP_LENGTH) pts=0;
    else pts-=JUMP_LENGTH;

    ADM_assert(_slider);

    bool oldState = _slider->blockSignals(true);
    goToTime(pts);
    updateSlider();
    _slider->blockSignals(oldState);
}
/**
 * 
 */
void ADM_flyDialog::fwdOneMinute(void)
{
    uint64_t pts=getCurrentPts();
    pts+=JUMP_LENGTH;

    ADM_assert(_slider);

    bool oldState = _slider->blockSignals(true);
    goToTime(pts);
    updateSlider();
    _slider->blockSignals(oldState);
}

/**
 * \fn gotoSelectionStart
 */
void ADM_flyDialog::gotoSelectionStart(void)
{
    ADM_assert(_slider);
    bool oldState = _slider->blockSignals(true);

    uint64_t pts = _in->getInfo()->markerA;
    if (_in->getInfo()->markerB < _in->getInfo()->markerA)
        pts = _in->getInfo()->markerB;
    if (pts > _in->getAbsoluteStartTime())
        pts -= _in->getAbsoluteStartTime();

    goToExactTime(pts);

    updateSlider();
    _slider->blockSignals(oldState);
}

/**
 * \fn gotoSelectionEnd
 */
void ADM_flyDialog::gotoSelectionEnd(void)
{
    ADM_assert(_slider);
    bool oldState = _slider->blockSignals(true);

    uint64_t pts = _in->getInfo()->markerB;
    if (_in->getInfo()->markerA > _in->getInfo()->markerB)
        pts = _in->getInfo()->markerA;
    if (pts > _in->getAbsoluteStartTime())
        pts -= _in->getAbsoluteStartTime();

    goToExactTime(pts);

    updateSlider();
    _slider->blockSignals(oldState);
}

/**
 * 
 */
void ADM_flyDialog::play(bool state)
{
    ADM_flyNavSlider *slide=(ADM_flyNavSlider *)_slider;
    ADM_assert(slide);
    if(state)
    {
        _control->disableButtons();
       slide->setEnabled(false);
       _clock.reset();
       timer.setInterval(_frameIncrement);
       _nextRdv=_frameIncrement;
       timer.start();
    }else
    {
        timer.stop();
        _control->enableButtons();
        slide->setEnabled(true);
    }
    // Allow to stop or start playback from the parent dialog cleanly.
    if(_control->pushButton_play->isChecked() != state)
    {
        _control->pushButton_play->blockSignals(true);
        _control->pushButton_play->setChecked(state);
        _control->pushButton_play->blockSignals(false);
    }
}
/**
 * 
 */
void ADM_flyDialog::peekOriginalPressed(void)
{
    if (!_bypassFilter)
    {
        _bypassFilter = true;
        this->sameImage();
    }
}
/**
 * 
 */
void ADM_flyDialog::peekOriginalReleased(void)
{
    if (_bypassFilter)
    {
        _bypassFilter = false;
        this->sameImage();
    }
}

/**
    \fn timeout
    \brief play filtered video
*/
void ADM_flyDialog::timeout(void)
{
    bool r=nextImage();
    if(_control)
    {
        char text[80];
        uint32_t mm,hh,ss,ms;
        uint32_t milly = _yuvBuffer->Pts/1000;

        ms2time(milly,&hh,&mm,&ss,&ms);
        sprintf(text, "%02d:%02d:%02d.%03d", hh, mm, ss, ms);
        _control->currentTime->setText(text);
    }
    if(r)
    {
        int now=_clock.getElapsedMS();
        //printf("Now = %d, next Rdv=%d, delta =%d\n",now,_nextRdv,_nextRdv-now);        
        _nextRdv+=_frameIncrement;
        if(_nextRdv<=now) timer.setInterval(1);
        else timer.setInterval(_nextRdv-now);
        timer.start();
    }
    else
    {
       _control->pushButton_play->setChecked(false);
    }
}


//******************************
//EOF

