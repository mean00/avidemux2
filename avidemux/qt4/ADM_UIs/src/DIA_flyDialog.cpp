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
#include <QPushButton>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QApplication>
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
 * 
 * @param frame
 * @return 
 */
bool        ADM_flyDialog::addControl(QHBoxLayout *horizontalLayout_4)
{
        _parent->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
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
        
        radioButton_autoZoom = new QRadioButton();
        radioButton_autoZoom->setObjectName(QString("radioButton_autoZoom"));
        radioButton_autoZoom->setChecked(true);

        horizontalLayout_4->addWidget(radioButton_autoZoom);

        
        QSpacerItem  *horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
        horizontalLayout_4->addItem(horizontalSpacer_4);
        
        pushButton_back1mn->setToolTip(QApplication::translate("seekablePreviewDialog", "Back one minute", 0));
        pushButton_back1mn->setText(QApplication::translate("seekablePreviewDialog", "<<", 0));
        pushButton_play->setText(QApplication::translate("seekablePreviewDialog", "Play", 0));
        pushButton_next->setStatusTip(QApplication::translate("seekablePreviewDialog", "next image", 0));
        pushButton_next->setText(QApplication::translate("seekablePreviewDialog", ">", 0));
        pushButton_fwd1mn->setText(QApplication::translate("seekablePreviewDialog", ">>", 0));
        radioButton_autoZoom->setText(QApplication::translate("seekablePreviewDialog", "A&utoZoom", 0));
        
        QObject::connect(pushButton_next ,SIGNAL(clicked()),this,SLOT(nextImage()));
        QObject::connect(pushButton_back1mn ,SIGNAL(clicked()),this,SLOT(backOneMinute()));
        QObject::connect(pushButton_fwd1mn ,SIGNAL(clicked()),this,SLOT(fwdOneMinute()));
        QObject::connect(pushButton_play ,SIGNAL(toggled(bool )),this,SLOT(play(bool)));
        QObject::connect(radioButton_autoZoom ,SIGNAL(toggled(bool )),this,SLOT(autoZoom(bool)));
      
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
    lastPts=_yuvBuffer->Pts;
    setCurrentPts(lastPts);
    // Process...   
    process();
    return display(_rgbByteBufferDisplay.at(0));
}
//************************************
// Implement the specific part
// i.e. yuv processing or RGB processing
//************************************
  ADM_flyDialogYuv::ADM_flyDialogYuv(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, QSlider *slider, 
                                ResizeMethod resizeMethod) : ADM_flyDialog(parent,width,height,in,canvas,slider,resizeMethod)
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
ADM_flyDialogRgb::ADM_flyDialogRgb(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                                ADM_QCanvas *canvas, QSlider *slider, 
                                ResizeMethod resizeMethod) : ADM_flyDialog(parent,width,height,in,canvas,slider,resizeMethod)
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

  ADM_flyDialog::ADM_flyDialog(QDialog *parent ,uint32_t width, uint32_t height, ADM_coreVideoFilter *in,
                              ADM_QCanvas *canvas, QSlider *slider,  ResizeMethod resizeMethod)
{  
    ADM_assert(canvas);

    if (slider)
            ADM_assert(in);
    _parent=parent;
    _w = width;
    _h = height;    
    _in = in;
    _slider = slider;
    _canvas = canvas;
    _cookie = NULL;
    _resizeMethod = resizeMethod;
    _zoomChangeCount = 0;        
    _yuvBuffer=new ADMImageDefault(_w,_h);
    lastPts=0;
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
    
    connect(&timer,SIGNAL(timeout()),this,SLOT(timeout()));
    timer.setSingleShot(true);

    int incrementUs=getUnderlyingFilter()->getInfo()->frameIncrement;

    incrementUs=(incrementUs+501)/1000; // us => ms
    if(incrementUs<10) incrementUs=10;

    timer.setInterval(incrementUs);
    timer.stop();
    
}
/**
    \fn    postInit
    \brief
*/

void ADM_flyDialog::postInit(uint8_t reInit)
{
	QWidget *graphicsView = ((ADM_QCanvas*)_canvas)->parentWidget();
	QSlider  *slider=(QSlider *)_slider;

	if (reInit)
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
}
/**
 * 
 */
void ADM_flyDialog::fwdOneMinute(void)
{
    uint64_t pts=getCurrentPts();
    pts+=JUMP_LENGTH;
    goToTime(pts);

}
/**
 * 
 */
void ADM_flyDialog::play(bool state)
{
    if(state)
    {
       pushButton_back1mn->setEnabled(false);
       pushButton_fwd1mn->setEnabled(false);
       pushButton_next->setEnabled(false);
       timer.start();
    }else
    {
        timer.stop();
        pushButton_back1mn->setEnabled(true);
        pushButton_fwd1mn->setEnabled(true);
        pushButton_next->setEnabled(true);
    }
    
}

/**
 * 
 */
void ADM_flyDialog::autoZoom(bool state)
{
    ADM_info("*** AUTO ZOOM = %d\n",(int)state);
    if(!state)
    {
        disableZoom();
        _canvas->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum));
        _parent->adjustSize();
        
    }else
    {
        enableZoom();
        _parent->adjustSize();
    }

}

void ADM_flyDialog::timeout()
{
    
    bool r=nextImage();
    if(r)
    {
        timer.start();
    }
    else
    {
       pushButton_play->setChecked(false);
    }
}


//******************************
//EOF

