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
#include <QHBoxLayout>
#include <QApplication>
#include <QLineEdit>
#include <QFontMetrics>
#include <QRect>
#include <QSizePolicy>

#include <cmath>

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "DIA_flyDialogQt4.h"

#include "ADM_toolkitQt.h"
#include "ADM_vidMisc.h"
#include "prefs.h"

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

            pushButton_gotosel = new QPushButton();
            pushButton_gotosel->setObjectName(QString("pushButton_gotosel"));
            pushButton_gotosel->setAutoRepeat(false);
            
            horizontalLayout_4->addWidget(pushButton_gotosel);
            
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

            if (controlOptions & ControlOption::AnalyzerBtn)
            {
                pushButton_analyzer = new QPushButton();
                pushButton_analyzer->setObjectName(QString("pushButton_analyzer"));
                pushButton_analyzer->setAutoRepeat(false);
                pushButton_analyzer->setText(QApplication::translate("seekablePreviewDialog", "Analyzer", 0));
                pushButton_analyzer->setToolTip(QApplication::translate("seekablePreviewDialog", "Show scopes", 0));

                horizontalLayout_4->addWidget(pushButton_analyzer);
            }
            else
                pushButton_analyzer = NULL;

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
            pushButton_gotosel->setText(QApplication::translate("seekablePreviewDialog", "[<", 0));
            pushButton_gotosel->setToolTip(QApplication::translate("seekablePreviewDialog", "Go to the start of the selection", 0));
            pushButton_play->setText(QApplication::translate("seekablePreviewDialog", "Play", 0));
            pushButton_next->setToolTip(QApplication::translate("seekablePreviewDialog", "Next image", 0));
            pushButton_next->setText(QApplication::translate("seekablePreviewDialog", ">", 0));
            pushButton_fwd1mn->setText(QApplication::translate("seekablePreviewDialog", ">>", 0));
            pushButton_fwd1mn->setToolTip(QApplication::translate("seekablePreviewDialog", "Forward one minute", 0));
         }
        void disableButtons()
        {
            pushButton_gotosel->setEnabled(false);
            pushButton_back1mn->setEnabled(false);
            pushButton_fwd1mn->setEnabled(false);
            pushButton_next->setEnabled(false);
        }
        void enableButtons()
        {
            pushButton_gotosel->setEnabled(true);
            pushButton_back1mn->setEnabled(true);
            pushButton_fwd1mn->setEnabled(true);
            pushButton_next->setEnabled(true);
        }
public:
        QPushButton *pushButton_gotosel;
        QPushButton *pushButton_back1mn;
        QPushButton *pushButton_play;
        QPushButton *pushButton_next;
        QPushButton *pushButton_fwd1mn;
        QLineEdit   *currentTime;
        QLabel      *labelDuration;
        QPushButton *pushButton_analyzer;
        QPushButton *pushButton_peekOriginal;
};

/**
    \fn updateZoom
*/
void ADM_flyDialog::updateZoom(void)
{
    uint32_t displayW, displayH;
    _canvas->getDisplaySize(&displayW, &displayH);
    _rgbByteBufferDisplay.clean();
    _rgbByteBufferDisplay.setSize(ADM_IMAGE_ALIGN(displayW * 4) * displayH);
    resetScaler();
}
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
    return 1;
}
/**
    \fn ~ADM_flyDialog
    \brief destructor
*/
ADM_flyDialog::~ADM_flyDialog(void)
{
    delete _flyAnalyzer;
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
bool        ADM_flyDialog::addControl(QHBoxLayout *horizontalLayout_4, ControlOption controlOptions, QWidget * userWidget)
{
    _parent->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
    _control=new flyControl(horizontalLayout_4, controlOptions, userWidget);
    _parent->adjustSize(); // force currentTime size calculation
    _control->currentTime->setTextMargins(0,0,0,0); // counteract Adwaita messing with text margins

    QObject::connect(_control->pushButton_next ,SIGNAL(clicked()),this,SLOT(nextImage()));
    QObject::connect(_control->pushButton_back1mn ,SIGNAL(clicked()),this,SLOT(backOneMinute()));
    QObject::connect(_control->pushButton_fwd1mn ,SIGNAL(clicked()),this,SLOT(fwdOneMinute()));
    QObject::connect(_control->pushButton_gotosel ,SIGNAL(clicked()),this,SLOT(gotoSelectionStart()));
    QObject::connect(_control->pushButton_play ,SIGNAL(toggled(bool )),this,SLOT(play(bool)));
    
    if (controlOptions & ControlOption::AnalyzerBtn)
    {
        QObject::connect(_control->pushButton_analyzer ,SIGNAL(released()),this,SLOT(analyzerReleased()));
    }
    if (controlOptions & ControlOption::PeekOriginalBtn)
    {
        QObject::connect(_control->pushButton_peekOriginal ,SIGNAL(pressed()),this,SLOT(peekOriginalPressed()));
        QObject::connect(_control->pushButton_peekOriginal ,SIGNAL(released()),this,SLOT(peekOriginalReleased()));
    }


    buttonList.push_back(_control->pushButton_back1mn);
    buttonList.push_back(_control->pushButton_gotosel);
    buttonList.push_back(_control->pushButton_play);
    buttonList.push_back(_control->pushButton_next);
    buttonList.push_back(_control->pushButton_fwd1mn);
    buttonList.push_back(_control->currentTime);
    if ((controlOptions & ControlOption::UserWidgetAfterControls) && (userWidget != NULL))
        buttonList.push_back(userWidget);
    if (controlOptions & ControlOption::AnalyzerBtn)
        buttonList.push_back(_control->pushButton_analyzer);
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
bool ADM_flyDialog::sameImage(void)
{
    process();
    return display(_rgbByteBufferDisplay.at(0));
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
    process();
    return display(_rgbByteBufferDisplay.at(0));
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
    yuvToRgb=NULL;  
    initializeSize();
    _canvas->parentWidget()->setMinimumSize(_zoomW, _zoomH);
    updateZoom();
}
void ADM_flyDialogYuv::resetScaler(void)
{
    if(yuvToRgb) 
    {
        delete yuvToRgb;
        yuvToRgb=NULL;
    }
    
    uint32_t displayW, displayH;
    _canvas->getDisplaySize(&displayW, &displayH);
    
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
}
bool ADM_flyDialogYuv::process(void)
{
    if (_bypassFilter)
    {
        yuvToRgb->convertImage(_yuvBuffer,_rgbByteBufferDisplay.at(0));
        if (_analyze)
            _flyAnalyzer->analyze(_yuvBuffer,_analyzerScenes[0],_analyzerScenes[1],_analyzerScenes[2],_analyzerScenes[3]);
    } else {
        processYuv(_yuvBuffer,_yuvBufferOut);
        if (_analyze)
            _flyAnalyzer->analyze(_yuvBufferOut,_analyzerScenes[0],_analyzerScenes[1],_analyzerScenes[2],_analyzerScenes[3]);
        yuvToRgb->convertImage(_yuvBufferOut,_rgbByteBufferDisplay.at(0));
    }
    return true;
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
                            _w,
                            _h,
                            _w,
                            _h,
                            ADM_PIXFRMT_YV12,toRgbPixFrmt());
    rgb2rgb=NULL;
    initializeSize();
    _canvas->parentWidget()->setMinimumSize(_zoomW, _zoomH);
    updateZoom();
}
void ADM_flyDialogRgb::resetScaler(void)
{
    if(rgb2rgb) delete rgb2rgb;

    uint32_t displayW, displayH;
    _canvas->getDisplaySize(&displayW, &displayH);
    
    rgb2rgb=new ADMColorScalerFull(_algo, 
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
        yuv2rgb->convertImage(_yuvBuffer,_rgbByteBufferDisplay.at(0));
    } else {
        if (_scaledPts != lastPts)
        {
            yuv2rgb->convertImage(_yuvBuffer,_rgbByteBuffer.at(0));
            _scaledPts = lastPts;
        }
        if (_resizeMethod != RESIZE_NONE)
        {
            processRgb(_rgbByteBuffer.at(0),_rgbByteBufferOut.at(0));
            rgb2rgb->convert(_rgbByteBufferOut.at(0), _rgbByteBufferDisplay.at(0));
        }else
        {
            processRgb(_rgbByteBuffer.at(0),_rgbByteBufferDisplay.at(0));
        }
    }
    return true;
}

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
    _yuvBuffer=new ADMImageDefault(_w,_h);
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
    _analyze=false;
    _analyzerDialog=NULL;
    _analyzerScenes[0]=_analyzerScenes[1]=_analyzerScenes[2]=_analyzerScenes[3]=NULL;
    _flyAnalyzer = new flyDialogsAnalyzer(width,height);

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
    sameImage();
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
    \fn    display
    \brief
*/

uint8_t  ADM_flyDialog::display(uint8_t *rgbData)
{
    ADM_QCanvas *view=_canvas;
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
    slide->setValue(value);
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
 * 
 */
void ADM_flyDialog::gotoSelectionStart(void)
{
    ADM_flyNavSlider  *slide=(ADM_flyNavSlider *)_slider;
    ADM_assert(slide);
    bool oldState=slide->blockSignals(true);
    uint64_t pts = _in->getInfo()->markerA;
    if (_in->getInfo()->markerB < _in->getInfo()->markerA)
        pts = _in->getInfo()->markerB;
    if (pts > _in->getAbsoluteStartTime())
        pts -= _in->getAbsoluteStartTime();
    goToExactTime(pts);
    updateSlider();
    slide->blockSignals(oldState);
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
void ADM_flyDialog::analyzerReleased(void)
{
    _control->pushButton_analyzer->setEnabled(false);
    if (!_analyze)
    {
        _analyzerDialog = new ADM_analyzerDialog(_parent);
        for (int i=0; i<4; i++)
            _analyzerScenes[i] = _analyzerDialog->gsc[i];
        QObject::connect(_analyzerDialog ,SIGNAL(destroyed()),this,SLOT(analyzerClosed()));
        _analyzerDialog->show();
        _analyze=true;
    }
    this->sameImage();
}
void ADM_flyDialog::analyzerClosed(void)
{
    _analyze = false;
    _analyzerScenes[0]=_analyzerScenes[1]=_analyzerScenes[2]=_analyzerScenes[3]=NULL;
    _control->pushButton_analyzer->setEnabled(true);
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



/**
 * 
 */
ADM_analyzerDialog::ADM_analyzerDialog(QWidget *parent) : QDialog(parent, Qt::Tool)
{
    vboxlayout = new QVBoxLayout(this);
    hboxlayout = new QHBoxLayout(this);
    this->setWindowTitle(QApplication::translate("seekablePreviewDialog", "Analyzer", 0));
    this->setAttribute(Qt::WA_DeleteOnClose);   // delete objets on closing the dialog
    for (int i=0; i<4; i++)
    {
        btns[i] = new QPushButton(this);
        switch(i)
        {
            case 0: btns[i]->setText(QApplication::translate("seekablePreviewDialog", "Vectorscope", 0)); break;
            case 1: btns[i]->setText(QApplication::translate("seekablePreviewDialog", "YUV waveform", 0)); break;
            case 2: btns[i]->setText(QApplication::translate("seekablePreviewDialog", "RGB waveform", 0)); break;
            case 3: btns[i]->setText(QApplication::translate("seekablePreviewDialog", "Histograms", 0)); break;
        }
        btns[i]->setCheckable(true);
        btns[i]->setChecked(i%2==0);    // vectorscope + RGB
        btns[i]->setStyleSheet("QPushButton { background-color: #888888; border: none; padding: 8px;}\nQPushButton:checked { background-color: #33FF33; border: none; padding: 8px;}");
        connect(btns[i],SIGNAL(toggled(bool)),this,SLOT(btnToggled(bool)));
        hboxlayout->addWidget(btns[i]);
        btnChkd[i] = btns[i]->isChecked();
    }
    vboxlayout->addLayout(hboxlayout);
    for (int i=0; i<4; i++)
    {
        gv[i] = new QGraphicsView(this);
        gv[i]->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gv[i]->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        gv[i]->setRenderHints(QPainter::Antialiasing|QPainter::SmoothPixmapTransform|QPainter::TextAntialiasing);
        gv[i]->setBackgroundBrush(QBrush(QColor::fromRgb(20,20,20),Qt::SolidPattern));
        gv[i]->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
        gsc[i] = new QGraphicsScene(this);
        if (i==0)
            gsc[i]->setSceneRect(0,0,620,600);
        else if (i<=2)
            gsc[i]->setSceneRect(0,0,772,258);
        else
            gsc[i]->setSceneRect(0,0,772,259);
        gv[i]->setScene(gsc[i]);
        gv[i]->scale(1/3.,1/3.);
        vboxlayout->addWidget(gv[i]);
        gv[i]->setVisible(btns[i]->isChecked());
    }
    this->setLayout(vboxlayout);
}

ADM_analyzerDialog::~ADM_analyzerDialog()
{
    
}

void ADM_analyzerDialog::btnToggled(bool f)
{
    bool allOff = true;
    for (int i=0; i<4; i++)
        if(btns[i]->isChecked())
            allOff = false;
            
    if (allOff)
        for (int i=0; i<4; i++)
            btns[i]->setChecked(btnChkd[i]);
    
    for (int i=0; i<4; i++)
    {
        gv[i]->setVisible(btns[i]->isChecked());
        btnChkd[i] = btns[i]->isChecked();
    }
    QCoreApplication::processEvents ();
    adjustGraphs();
}

void ADM_analyzerDialog::adjustGraphs()
{
    QRectF bounds;
    for (int i=0; i<4; i++)
    {
        bounds = gsc[i]->itemsBoundingRect();
        gv[i]->fitInView(bounds, Qt::KeepAspectRatio);
    }
}

void ADM_analyzerDialog::resizeEvent(QResizeEvent *event)
{
    adjustGraphs();
}

void ADM_analyzerDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    QCoreApplication::processEvents ();
    adjustGraphs();
}

/**
 * 
 */
flyDialogsAnalyzer::flyDialogsAnalyzer(int width, int height)
{
    this->width = width;
    this->height = height;
    wrkVectorScope = new uint32_t [256*256];
    bufVectorScope = new uint32_t [620*600];
    scaleVectorScope = new uint32_t [620*600];
    imgVectorScope = new QImage((uchar *)bufVectorScope, 620, 600, 620*sizeof(uint32_t), QImage::Format_RGB32);
    
    for (int y=0; y<600; y++)
    {
        for (int x=0; x<620; x++)
        {
            double xc = x-(54.0+256.0);
            double yc = y-(44.0+256.0);
            double r = std::sqrt(xc*xc + yc*yc);
            uint32_t c = 0;
            if ((r <= 300.0) && (r >= 284.0))   // hue ring
            {
                xc *= 127.0/r;
                yc *= 127.0/r;
                yc *= -1.0;
                r = (8.0-std::abs(r-292.0))/8.0;  // 0..1..0 triangle
                r = std::sqrt(r); // nonlinearity
                r *= 166;
                if (r > 128.0)
                    r = 128.0;
                int rgb[3];
                rgb[0] = std::round(r            +   1.4*yc);
                rgb[1] = std::round(r - 0.343*xc - 0.711*yc);
                rgb[2] = std::round(r + 1.765*xc           );
                for (int i=0; i<3; i++)
                {
                    if (rgb[i] < 0) rgb[i] = 0;
                    if (rgb[i] > 255) rgb[i] = 255;
                }
                c = (rgb[0]<<16) + (rgb[1]<<8) + rgb[2];
            }
            
            for (int pri=1; pri<=6; pri++)
            {
                int rgb[3];
                rgb[0] = (pri&1)? 1:0;
                rgb[1] = (pri&2)? 1:0;
                rgb[2] = (pri&4)? 1:0;
                double u = 54 + 256 + 2*224.0*(-0.1146*rgb[0] + -0.3854*rgb[1] +  0.5   *rgb[2]);
                double v = 44 + 256 - 2*224.0*( 0.5   *rgb[0] + -0.4542*rgb[1] + -0.0458*rgb[2]);
                u = x-u;
                v = y-v;
                r = std::sqrt(u*u + v*v);
                if ((r <= 16.1) && (r >= 13.3))
                {
                    c = 0;
                    if (pri&1)
                        c += 0xFF0000;
                    if (pri&2)
                        c += 0x00FF00;
                    if (pri&4)
                        c += 0x0000FF;
                }
            }
            
            scaleVectorScope[y*620+x] = c;
        }
    }

    for (int i=0; i<3; i++)
        wrkYUVparade[i] = new uint32_t [256*256];
    bufYUVparade = new uint32_t [772*258];
    imgYUVparade = new QImage((uchar *)bufYUVparade, 772, 258, 772*sizeof(uint32_t), QImage::Format_RGB32);

    for (int i=0; i<3; i++)
        wrkRGBparade[i] = new uint32_t [256*256];
    bufRGBparade = new uint32_t [772*258];
    imgRGBparade = new QImage((uchar *)bufRGBparade, 772, 258, 772*sizeof(uint32_t), QImage::Format_RGB32);

    for (int i=0; i<6; i++)
        wrkHistograms[i] = new uint32_t [256];
    bufHistograms = new uint32_t [772*259];
    imgHistograms = new QImage((uchar *)bufHistograms, 772, 259, 772*sizeof(uint32_t), QImage::Format_RGB32);
    
    paradeIndex = new int [width];
    for (int i=0; i<width; i++)
    {
        double fpos = i;
        fpos /= width;
        fpos *= 256.0;
        paradeIndex[i] = fpos;
        if (paradeIndex[i] > 255)
            paradeIndex[i] = 255;
    }
    paradeIndexHalf = new int [width/2];
    for (int i=0; i<width/2; i++)
    {
        double fpos = i;
        fpos *= 2.0;
        fpos /= width;
        fpos *= 256.0;
        paradeIndexHalf[i] = fpos;
        if (paradeIndexHalf[i] > 255)
            paradeIndexHalf[i] = 255;
    }

    rgbBufStride = ADM_IMAGE_ALIGN(width * 4);
    rgbBufRaw = new ADM_byteBuffer();
    rgbBufRaw->setSize(rgbBufStride * height);
    convertYuvToRgb = new ADMColorScalerFull(ADM_CS_BILINEAR,width,height,width,height,ADM_PIXFRMT_YV12,ADM_PIXFRMT_RGB32A);
}
/**
 * 
 */
flyDialogsAnalyzer::~flyDialogsAnalyzer()
{
    delete [] wrkVectorScope;
    delete [] bufVectorScope;
    delete [] scaleVectorScope;
    delete imgVectorScope;
    for (int i=0; i<3; i++)
        delete [] wrkYUVparade[i];
    delete [] bufYUVparade;
    delete imgYUVparade;
    for (int i=0; i<3; i++)
        delete [] wrkRGBparade[i];
    delete [] bufRGBparade;
    delete imgRGBparade;
    for (int i=0; i<6; i++)
        delete [] wrkHistograms[i];
    delete [] bufHistograms;
    delete imgHistograms;
    
    delete [] paradeIndex;
    delete [] paradeIndexHalf;

    delete convertYuvToRgb;
    rgbBufRaw->clean();
    delete rgbBufRaw;

}
/**
 * 
 */
void flyDialogsAnalyzer::analyze(ADMImage *in, QGraphicsScene * sceneVectorScope, QGraphicsScene * sceneYUVparade, QGraphicsScene * sceneRGBparade, QGraphicsScene * sceneHistograms)
{
    if (in == NULL) return;
    if (sceneVectorScope == NULL) return;
    if (sceneYUVparade == NULL) return;
    if (sceneRGBparade == NULL) return;
    if (sceneHistograms == NULL) return;
    #define FRAME_COLOR	(0xFF000000) //(0xFF7F7F7F)
    // Make Y plane statistics
    {
        memset(wrkYUVparade[0],0,256*256*sizeof(uint32_t));
        memset(wrkHistograms[3],0,256*sizeof(uint32_t));
        uint8_t * yp=in->GetReadPtr(PLANAR_Y);
        int stride=in->GetPitch(PLANAR_Y);
        uint8_t * ptr;
        int value;
        
        for (int y=0; y<height; y++)
        {
            ptr = yp + y*stride;
            for (int x=0; x<width; x++)
            {
                value = *ptr++;
                wrkHistograms[3][value]++;
                wrkYUVparade[0][value*256 + paradeIndex[x]]++;
            }
        }
    }

    // Make U-V plane statistics
    {
        memset(wrkVectorScope,0,256*256*sizeof(uint32_t));
        memset(wrkYUVparade[1],0,256*256*sizeof(uint32_t));
        memset(wrkYUVparade[2],0,256*256*sizeof(uint32_t));
        memset(wrkHistograms[4],0,256*sizeof(uint32_t));
        memset(wrkHistograms[5],0,256*sizeof(uint32_t));
        uint8_t * up=in->GetReadPtr(PLANAR_U);
        uint8_t * vp=in->GetReadPtr(PLANAR_V);
        int ustride=in->GetPitch(PLANAR_U);
        int vstride=in->GetPitch(PLANAR_V);
        uint8_t * uptr, * vptr;
        int uvalue, vvalue;
        int width=in->GetWidth(PLANAR_U); 
        int height=in->GetHeight(PLANAR_U);
        
        for (int y=0; y<height; y++)
        {
            uptr = up + y*ustride;
            vptr = vp + y*vstride;
            for (int x=0; x<width; x++)
            {
                uvalue = *uptr++;
                vvalue = *vptr++;
                wrkHistograms[4][uvalue]+=4;    // num of chroma pixels are quarter of luma pixels
                wrkHistograms[5][vvalue]+=4;
                wrkYUVparade[1][uvalue*256 + paradeIndexHalf[x]]+=4;
                wrkYUVparade[2][vvalue*256 + paradeIndexHalf[x]]+=4;
                wrkVectorScope[vvalue*256 + uvalue]++;
            }
        }
    }
    
    // Make RGB statistics
    {
        convertYuvToRgb->convertImage(in,rgbBufRaw->at(0));
        memset(wrkRGBparade[0],0,256*256*sizeof(uint32_t));
        memset(wrkRGBparade[1],0,256*256*sizeof(uint32_t));
        memset(wrkRGBparade[2],0,256*256*sizeof(uint32_t));
        memset(wrkHistograms[0],0,256*sizeof(uint32_t));
        memset(wrkHistograms[1],0,256*sizeof(uint32_t));
        memset(wrkHistograms[2],0,256*sizeof(uint32_t));
        uint8_t * ptr;
        int rvalue, gvalue, bvalue;
 
        for (int y=0; y<height; y++)
        {
            ptr = rgbBufRaw->at(y*rgbBufStride);
            for (int x=0; x<width; x++)
            {
                rvalue = *ptr++;
                gvalue = *ptr++;
                bvalue = *ptr++;
                ptr++;
                wrkHistograms[0][rvalue]++;
                wrkHistograms[1][gvalue]++;
                wrkHistograms[2][bvalue]++;
                wrkRGBparade[0][rvalue*256 + paradeIndex[x]]++;
                wrkRGBparade[1][gvalue*256 + paradeIndex[x]]++;
                wrkRGBparade[2][bvalue*256 + paradeIndex[x]]++;
            }
        }
    }
    
    // Normalize histograms to 0 .. 124
    {
        for (int csp=0; csp<2; csp++)
        {
            for (int ch=0; ch<3; ch++)
                for (int i=0; i<256; i++)
                {
                    double value = wrkHistograms[csp*3+ch][i];
                    value /= height;
                    value /= width;
                    value *= 124*10;
                    if (value > 124)
                        value = 124;
                    wrkHistograms[csp*3+ch][i] = value;
                }
        }
    }
    
    // Normalize parades
    {
        uint32_t norm = 2147483648ULL/(width*height);
        for (int ch=0; ch<3; ch++)
            for (int i=0; i<256; i++)
            {
                wrkYUVparade[ch][i] = (wrkYUVparade[ch][i]*norm)>>8;
                wrkRGBparade[ch][i] = (wrkRGBparade[ch][i]*norm)>>8;
            }
    }

    // Normalize vectorscope
    {
        uint32_t norm = 1073741824ULL/(width*height);
        for (int y=0; y<256; y++)
            for (int x=0; x<256; x++)
                wrkVectorScope[y*256 + x] = (wrkVectorScope[y*256 + x]*norm)>>8;
    }

    // Draw histograms
    {
        memset(bufHistograms, 0, 772*259*sizeof(uint32_t));
        uint32_t q,color;

        for (int csp=0; csp<2; csp++)
        {
            for (int ch=0; ch<3; ch++)
            {
                switch(ch+csp*3)
                {
                    case 0:    //R
                            color = 0xFFFF0000;
                        break;
                    case 1:    //G
                            color = 0xFF00FF00;
                        break;
                    case 2:    //B
                            color = 0xFF0000FF;
                        break;
                    case 3:    //Y
                            color = 0xFFFFFFFF;
                        break;
                    case 4:    //U
                            color = 0xFF7F3FFF;
                        break;
                    case 5:    //V
                            color = 0xFFFF3F7F;
                        break;
                }
                for (int x=0; x<256; x++)
                {
                    q = wrkHistograms[csp*3+ch][x];
                    for (int y=0; y<128;y++)
                    {
                        bufHistograms[(csp*129+y+1)*772+(ch*257+1)+x] = ((((127-y)-(int)q) > 0) ? 0xFF000000 : color);
                    }
                }
            }
        }

        // add frame
        for (int x=0; x<772; x++)
        {
            bufHistograms[772*0+x] = FRAME_COLOR;
            bufHistograms[772*129+x] = FRAME_COLOR;
            bufHistograms[772*258+x] = FRAME_COLOR;
        }
        for (int y=1; y<257; y++)
        {
            bufHistograms[772*y+0] = FRAME_COLOR;
            bufHistograms[772*y+257] = FRAME_COLOR;
            bufHistograms[772*y+514] = FRAME_COLOR;
            bufHistograms[772*y+771] = FRAME_COLOR;
        }

        sceneHistograms->clear();
        sceneHistograms->addPixmap( QPixmap::fromImage(*imgHistograms));
    }
    
    // Draw YUV parade
    {
        uint32_t p,c;
        // convert to color 0xffRRGGBB
        for (int y=1; y<257; y++)
        {
            //Y
            for (int x=1; x<257; x++)
            {
                p = wrkYUVparade[0][(256-y)*256 + x-1];
                c = 0;
                if (p)
                {
                    p /= 2;
                    if (p >= 256)
                        c = 0x00FFFFFF;
                    else
                        c = (p << 16) + (p << 8) + (p << 0);
                }
                bufYUVparade[772*y+x] = 0xFF000000 | c;
            }
            //U
            for (int x=258; x<514; x++)
            {
                p = wrkYUVparade[1][(256-y)*256 + x-258];
                c = 0;
                if (p)
                {
                    if (p >= 1020)
                        c = 0x00FFFFFF;
                    else
                    if (p >= 510)
                        c = 0x00FF00FF + ((p/4) << 8);
                    else
                    if (p >= 256)
                        c = 0x000000FF + ((p/2) << 16) + ((p/4) << 8);
                    else
                        c = ((p/2) << 16) + ((p/4) << 8) + (p << 0);
                }
                bufYUVparade[772*y+x] = 0xFF000000 | c;
            }
            //V
            for (int x=515; x<771; x++)
            {
                p = wrkYUVparade[2][(256-y)*256 + x-515];
                c = 0;
                if (p)
                {
                    if (p >= 1020)
                        c = 0x00FFFFFF;
                    else
                    if (p >= 510)
                        c = 0x00FF00FF + ((p/4) << 8);
                    else
                    if (p >= 256)
                        c = 0x00FF0000 + ((p/4) << 8) + ((p/2) << 0);
                    else
                        c = (p << 16) + ((p/4) << 8) + ((p/2) << 0);
                }
                bufYUVparade[772*y+x] = 0xFF000000 | c;
            }
        }

        // add frame
        for (int x=0; x<772; x++)
        {
            bufYUVparade[772*0+x] = FRAME_COLOR;
            bufYUVparade[772*257+x] = FRAME_COLOR;
        }
        for (int y=1; y<257; y++)
        {
            bufYUVparade[772*y+0] = FRAME_COLOR;
            bufYUVparade[772*y+257] = FRAME_COLOR;
            bufYUVparade[772*y+514] = FRAME_COLOR;
            bufYUVparade[772*y+771] = FRAME_COLOR;
        }

        sceneYUVparade->clear();
        sceneYUVparade->addPixmap( QPixmap::fromImage(*imgYUVparade));
    }
    
    // Draw RGB parade
    {
         uint32_t p,c;
       // convert to color 0xffRRGGBB
        for (int y=1; y<257; y++)
        {
            //R
            for (int x=1; x<257; x++)
            {
                p = wrkRGBparade[0][(256-y)*256 + x-1];
                if (p >= 765)
                    c = 0x00FFFFFF;
                else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    c = 0x00FF0000 + (p<<8) + p;
                } else
                    c = p << 16;
                bufRGBparade[772*y+x] = 0xFF000000 | c;
            }
            //G
            for (int x=258; x<514; x++)
            {
                p =wrkRGBparade[1][(256-y)*256 + x-258];
                if (p >= 765)
                    c = 0x00FFFFFF;
                else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    c = 0x0000FF00 + (p<<16) + p;
                } else
                    c = p << 8;
                bufRGBparade[772*y+x] = 0xFF000000 | c;
            }
            //B
            for (int x=515; x<771; x++)
            {
                p = wrkRGBparade[2][(256-y)*256 + x-515];
                if (p >= 765)
                    c = 0x00FFFFFF;
                else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    c = 0x000000FF + (p<<16) + (p<<8);
                } else
                    c = p << 0;
                bufRGBparade[772*y+x] = 0xFF000000 | c;
            }
        }

        // add frame
        for (int x=0; x<772; x++)
        {
            bufRGBparade[772*0+x] = FRAME_COLOR;
            bufRGBparade[772*257+x] = FRAME_COLOR;
        }
        for (int y=1; y<257; y++)
        {
            bufRGBparade[772*y+0] = FRAME_COLOR;
            bufRGBparade[772*y+257] = FRAME_COLOR;
            bufRGBparade[772*y+514] = FRAME_COLOR;
            bufRGBparade[772*y+771] = FRAME_COLOR;
        }

        sceneRGBparade->clear();
        sceneRGBparade->addPixmap( QPixmap::fromImage(*imgRGBparade));
    }
    
    // Draw vectorscope
    {
        uint32_t p,q,c;
        uint8_t argb[4];
        memset(bufVectorScope,0,620*600*sizeof(uint32_t));
        
        for (int y=0; y<256; y++)
            for (int x=0; x<256; x++)
                bufVectorScope[(2*y + 44)*620 + 2*x + 54] = wrkVectorScope[(255-y)*256+x];

        // interpolate histogram
        for (int y=44; y<(44+512); y+=2)
        {
            uint32_t * ptr = bufVectorScope+620*y;
            for (int x=(54-1); x<(54+512+1); x+=2)
            {
                *(ptr+x) = ( (*(ptr+x-1)) + (*(ptr+x+1)) )/2;
            }
        }
        for (int y=(44-1); y<(44+512+1); y+=2)
        {
            uint32_t * ptrm = bufVectorScope+620*(y-1);
            uint32_t * ptr  = bufVectorScope+620*y;
            uint32_t * ptrp = bufVectorScope+620*(y+1);
            for (int x=54; x<(54+512); x+=1)
            {
                *(ptr+x) = ( (*(ptrm+x)) + (*(ptrp+x)) )/2;
            }
        }

        // convert to color 0xffRRGGBB and apply scale
        for (int i=0; i<620*600; i++)
        {
            p = bufVectorScope[i];
            q = scaleVectorScope[i];
            if (p)
            {
                if (p >= 765)
                {
                    c = 0x00FFFFFF;
                } else
                if (p >= 256)
                {
                    p = (p-255)/2;
                    c = 0x0000FF00 + (p<<16) + p;
                } else {
                    c = p<<8;
                }
                
                if (q)
                {
                    c = (c>>1) & 0x007F7F7F;
                    q = (q>>1) & 0x007F7F7F;
                    q += c;
                }
                else
                    q = c;
            }
            bufVectorScope[i] = 0xFF000000 | q;
        }

        sceneVectorScope->clear();
        sceneVectorScope->addPixmap( QPixmap::fromImage(*imgVectorScope));
    }
    #undef FRAME_COLOR
}

//******************************
//EOF

