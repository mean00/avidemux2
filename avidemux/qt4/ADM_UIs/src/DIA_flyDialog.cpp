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
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QApplication>
#include "ADM_toolkitQt.h"
#include "ADM_vidMisc.h"
extern "C" {
#include "libavcodec/avcodec.h"
}
/**
 */
class flyControl
{
public:
        flyControl(QHBoxLayout *horizontalLayout_4)
        {
            
            pushButton_back1mn = new QPushButton();
            pushButton_back1mn->setObjectName(QString("pushButton_back1mn"));
            pushButton_back1mn->setAutoRepeat(true);
            pushButton_back1mn->setAutoRepeatDelay(1000);

            horizontalLayout_4->addWidget(pushButton_back1mn);

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
            labelTime=new QLabel();
            labelTime->setText("00:00:00.000 / 00:00:00.000");
            horizontalLayout_4->addWidget(labelTime);
            //
            QSpacerItem  *horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
            horizontalLayout_4->addItem(horizontalSpacer_4);

            pushButton_back1mn->setToolTip(QApplication::translate("seekablePreviewDialog", "Back one minute", 0));
            pushButton_back1mn->setText(QApplication::translate("seekablePreviewDialog", "<<", 0));
            pushButton_play->setText(QApplication::translate("seekablePreviewDialog", "Play", 0));
            pushButton_next->setToolTip(QApplication::translate("seekablePreviewDialog", "Next image", 0));
            pushButton_next->setText(QApplication::translate("seekablePreviewDialog", ">", 0));
            pushButton_fwd1mn->setText(QApplication::translate("seekablePreviewDialog", ">>", 0));
            pushButton_fwd1mn->setToolTip(QApplication::translate("seekablePreviewDialog", "Forward one minute", 0));
         }
        void disableButtons()
        {
            pushButton_back1mn->setEnabled(false);
            pushButton_fwd1mn->setEnabled(false);
            pushButton_next->setEnabled(false);
        }
        void enableButtons()
        {
            pushButton_back1mn->setEnabled(true);
            pushButton_fwd1mn->setEnabled(true);
            pushButton_next->setEnabled(true);
        }
public:
        QPushButton *pushButton_back1mn;
        QPushButton *pushButton_play;
        QPushButton *pushButton_next;
        QPushButton *pushButton_fwd1mn;
        QLabel       *labelTime;
};

/**
    \fn updateZoom
*/
void ADM_flyDialog::updateZoom(void)
{
        _rgbByteBufferDisplay.clean();
        _rgbByteBufferDisplay.setSize(ADM_IMAGE_ALIGN(_zoomW * 4) * _zoomH);
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
        updateZoom();
        postInit (true);
        sliderChanged();
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
 * 
 * @param frame
 * @return 
 */
bool        ADM_flyDialog::addControl(QHBoxLayout *horizontalLayout_4)
{
        _parent->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
        _control=new flyControl(horizontalLayout_4);
        QObject::connect(_control->pushButton_next ,SIGNAL(clicked()),this,SLOT(nextImage()));
        QObject::connect(_control->pushButton_back1mn ,SIGNAL(clicked()),this,SLOT(backOneMinute()));
        QObject::connect(_control->pushButton_fwd1mn ,SIGNAL(clicked()),this,SLOT(fwdOneMinute()));
        QObject::connect(_control->pushButton_play ,SIGNAL(toggled(bool )),this,SLOT(play(bool)));

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
    QString time=QString(ADM_us2plain(lastPts)) + QString(" / ") + QString(ADM_us2plain(duration));
    if(_control)
        _control->labelTime->setText(time);
    // Process...   
    process();
    return display(_rgbByteBufferDisplay.at(0));
}

/**
    \fn nextImage
*/
bool ADM_flyDialog::nextImage(void)
{
    ADM_QSlider  *slide=(ADM_QSlider *)_slider;
    ADM_assert(slide);
    bool oldState=slide->blockSignals(true);
    bool r=nextImageInternal();
    if(r)
        updateSlider();
    slide->blockSignals(oldState);
    return r;
}

/**
 * 
 * @return 
 */
bool ADM_flyDialog::initializeSize()
{
    _canvas->resize(1,1);
    QSize qsize= _canvas->parentWidget()->parentWidget()->size();
    //_usedWidth = qsize.width();
    // Normally there is nothing interesting left and right, we can use a hardcoded value
    _usedWidth=64;
    _usedHeight= 32+qsize.height(); // keep a border margin
    
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
                                ADM_QCanvas *canvas, ADM_QSlider *slider,
                                ResizeMethod resizeMethod) : ADM_flyDialog(parent,width,height,in,canvas,slider,resizeMethod)
{
       _control=NULL;
        _yuvBufferOut=new ADMImageDefault(_w,_h);
        yuvToRgb=NULL;  
        initializeSize();
        updateZoom();
        postInit(false);
        _nextRdv=0;
}
void ADM_flyDialogYuv::resetScaler(void)
{
    if(yuvToRgb) 
    {
        delete yuvToRgb;
        yuvToRgb=NULL;
    }
    
    yuvToRgb=new ADMColorScalerFull(ADM_CS_BICUBIC, 
                            _w,
                            _h,
                            _zoomW,
                            _zoomH,
                            ADM_COLOR_YV12,toRgbColor());
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
        delete _control;
        _control=NULL;
    }
}
bool ADM_flyDialogYuv::process(void)
{
         processYuv(_yuvBuffer,_yuvBufferOut);
        yuvToRgb->convertImage(_yuvBufferOut,_rgbByteBufferDisplay.at(0));
        return true;
}
//*****************************************
ADM_flyDialogRgb::ADM_flyDialogRgb(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, ADM_QSlider *slider,
                                ResizeMethod resizeMethod) : ADM_flyDialog(parent,width,height,in,canvas,slider,resizeMethod)
{
    uint32_t size = ADM_IMAGE_ALIGN(_w*4);
    size*=_h;
    _rgbByteBuffer.setSize(size);
    _rgbByteBufferOut.setSize(size);
     yuv2rgb =new ADMColorScalerSimple(_w,_h,ADM_COLOR_YV12,
                toRgbColor());
    rgb2rgb=NULL;
    initializeSize();
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

  ADM_flyDialog::ADM_flyDialog(QDialog *parent ,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              ADM_QCanvas *canvas, ADM_QSlider *slider,  ResizeMethod resizeMethod)
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
    _zoomChangeCount = 0;        
    _yuvBuffer=new ADMImageDefault(_w,_h);
    _usedWidth= _usedHeight=0;
    lastPts=0;

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
    
}
/**
    \fn    postInit
    \brief
*/

void ADM_flyDialog::postInit(uint8_t reInit)
{
	QWidget *graphicsView = ((ADM_QCanvas*)_canvas)->parentWidget();
	ADM_QSlider  *slider=(ADM_QSlider *)_slider;

	if (reInit)
	{
		FlyDialogEventFilter *eventFilter = new FlyDialogEventFilter(this);

		if (slider)
			slider->setMaximum(ADM_FLY_SLIDER_MAX);

		graphicsView->parentWidget()->installEventFilter(eventFilter);
	}

	((ADM_QCanvas*)_canvas)->changeSize(_zoomW, _zoomH);
	graphicsView->setMinimumSize(_zoomW, _zoomH);
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

    if(viewAr != ar)
    {
        uint32_t tmpZoomW = 0;
        uint32_t tmpZoomH = 0;
        if(viewAr > ar)
        {
            tmpZoomW = (uint32_t)((double)height * ar);
            tmpZoomH = height;
        }else
        {
            tmpZoomW = width;
            tmpZoomH = (uint32_t)((double)width / ar);
        }
        _resizeMethod = RESIZE_AUTO;
        _zoomW = tmpZoomW&0xfffffffe;
        _zoomH = tmpZoomH&0xfffffffe;
        _zoom = (float)_zoomW / _w;
        updateZoom();
        _canvas->changeSize(_zoomW, _zoomH);
        sameImage();
    }
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
  ADM_QSlider  *slide=(ADM_QSlider *)_slider;
  ADM_assert(slide);
  return slide->value();
  
}
/**
    \fn    sliderSet
    \brief
*/

uint8_t     ADM_flyDialog::sliderSet(uint32_t value)
{
  ADM_QSlider  *slide=(ADM_QSlider *)_slider;
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
    uint64_t dur=_in->getInfo()->totalDuration;
    uint64_t pts=getCurrentPts();
    double pos;
    pos=pts;
    pos/=dur;
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
    goToTime(pts);
    updateSlider();
}
/**
 * 
 */
void ADM_flyDialog::fwdOneMinute(void)
{
    uint64_t pts=getCurrentPts();
    pts+=JUMP_LENGTH;
    goToTime(pts);
    updateSlider();
}
/**
 * 
 */
void ADM_flyDialog::play(bool state)
{
    ADM_QSlider *slide=(ADM_QSlider *)_slider;
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
    
}

/**
    \fn timeout
    \brief play filtered video
*/
void ADM_flyDialog::timeout()
{
    bool r=nextImage();
    uint64_t duration=_in->getInfo()->totalDuration;
    QString time=QString(ADM_us2plain(_yuvBuffer->Pts)) + QString(" / ") + QString(ADM_us2plain(duration));
    _control->labelTime->setText(time);
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

