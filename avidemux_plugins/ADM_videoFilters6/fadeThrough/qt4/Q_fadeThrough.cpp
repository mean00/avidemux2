/***************************************************************************
                          DIA_crop.cpp  -  description
                             -------------------

			    GUI for cropping including autocrop
			    +Revisted the Gtk2 way
			     +Autocrop now in RGB space (more accurate)

    begin                : Fri May 3 2002
    copyright            : (C) 2002/2007 by mean
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

#include <QPushButton>
#include <QPalette>
#include <QColorDialog>
#include "ADM_default.h"
#include "Q_fadeThrough.h"
#include "DIA_coreToolkit.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidFadeThrough.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_fadeThroughWindow::Ui_fadeThroughWindow(QWidget *parent, fadeThrough *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        if (ADMVideoFadeThrough::IsFadeIn())
            this->setWindowTitle(QString(QT_TRANSLATE_NOOP("fadeThrough","Fade in")));
        if (ADMVideoFadeThrough::IsFadeOut())
            this->setWindowTitle(QString(QT_TRANSLATE_NOOP("fadeThrough","Fade out")));

        if (ADMVideoFadeThrough::IsFadeIn() || ADMVideoFadeThrough::IsFadeOut())
        {
        #define SETMAX(x) \
            { \
                ui.horizontalSlider##x->setMaximum(100); \
                ui.doubleSpinBox##x->setMaximum(1.0); \
            }

            SETMAX(TransientDurationBright);
            SETMAX(TransientDurationSat);
            SETMAX(TransientDurationBlend);
            SETMAX(TransientDurationBlur);
            SETMAX(TransientDurationRot);
            SETMAX(TransientDurationZoom);
            SETMAX(TransientDurationVignette);
        #undef SETMAX
        }

        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;
        markerA = in->getInfo()->markerA;
        markerB = in->getInfo()->markerB;
        duration = in->getInfo()->totalDuration;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        scene=new QGraphicsScene(this);
        scene->setSceneRect(0,0,256,128);
        ui.graphicsViewTransient->setScene(scene);
        ui.graphicsViewTransient->scale(1.0,1.0);
        
        myFly=new flyFadeThrough( this,width, height,in,canvas,ui.horizontalSlider,scene);
        memcpy(&(myFly->param),param,sizeof(fadeThrough));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        
        connect(ui.pushButtonTManual,SIGNAL(clicked(bool)),this,SLOT(manualTimeEntry(bool)));
        connect(ui.pushButtonTMarker,SIGNAL(clicked(bool)),this,SLOT(timesFromMarkers(bool)));
        if (!(ADMVideoFadeThrough::IsFadeIn() || ADMVideoFadeThrough::IsFadeOut()))
        {
            connect(ui.pushButtonTCenter,SIGNAL(clicked(bool)),this,SLOT(centeredTimesFromMarkers(bool)));
        }
        else
        {
            ui.pushButtonTCenter->setVisible(false);
            ui.labelCenter->setVisible(false);
        }
        
        connect(ui.tabEffects,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
        
#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        CHKBOX(EnableBright);
        CHKBOX(EnableSat);
        CHKBOX(EnableBlend);
        CHKBOX(EnableBlur);
        CHKBOX(EnableRot);
        CHKBOX(EnableZoom);
        CHKBOX(EnableVignette);

        connect(ui.pushButtonColorBlend,SIGNAL(released()),this,SLOT(pushedColorBlend()));
        connect(ui.pushButtonColorVignette,SIGNAL(released()),this,SLOT(pushedColorVignette()));
        
#define SPINNER(x) \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \
        connect( ui.doubleSpinBox##x,SIGNAL(valueChanged(double)),this,SLOT(valueChangedSpinBox(double)));
        
        SPINNER(PeakBright);
        SPINNER(PeakSat);
        SPINNER(PeakBlend);
        SPINNER(PeakBlur);
        SPINNER(PeakRot);
        SPINNER(PeakZoom);
        SPINNER(PeakVignette);

#define COMBOX(x) connect(ui.comboBox##x,SIGNAL(currentIndexChanged(int)),this,SLOT(valueChanged(int)));

        COMBOX(TransientBright);
        COMBOX(TransientSat);
        COMBOX(TransientBlend);
        COMBOX(TransientBlur);
        COMBOX(TransientRot);
        COMBOX(TransientZoom);
        COMBOX(TransientVignette);

        SPINNER(TransientDurationBright);
        SPINNER(TransientDurationSat);
        SPINNER(TransientDurationBlend);
        SPINNER(TransientDurationBlur);
        SPINNER(TransientDurationRot);
        SPINNER(TransientDurationZoom);
        SPINNER(TransientDurationVignette);
        
        QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        setModal(true);
}

void Ui_fadeThroughWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}

void Ui_fadeThroughWindow::manualTimeEntry(bool f)
{
    uint32_t mx=(uint32_t)(duration/1000LL);

    diaElemTimeStamp start(&(myFly->param.startTime),QT_TRANSLATE_NOOP("fadeThrough","_Start time:"),0,mx);
    diaElemTimeStamp end(&(myFly->param.endTime),QT_TRANSLATE_NOOP("fadeThrough","_End time:"),0,mx);
    diaElem *elems[2]={&start,&end};

    if(diaFactoryRun(QT_TRANSLATE_NOOP("fadeThrough","Manual time entry"),2+0*1,elems))
    {
        if(myFly->param.startTime > myFly->param.endTime)
        {
            uint32_t tmp=myFly->param.startTime;
            myFly->param.startTime=myFly->param.endTime;
            myFly->param.endTime=tmp;
        }
        valueChanged(0);
    }
}

void Ui_fadeThroughWindow::timesFromMarkers(bool f)
{
    myFly->param.startTime = markerA / 1000LL;
    myFly->param.endTime = markerB / 1000LL;
    if(myFly->param.startTime > myFly->param.endTime)
    {
        uint32_t tmp=myFly->param.startTime;
        myFly->param.startTime=myFly->param.endTime;
        myFly->param.endTime=tmp;
    }
    valueChanged(0);
}

void Ui_fadeThroughWindow::centeredTimesFromMarkers(bool f)
{
    int64_t mstart,mend;
    mstart = (int64_t)markerA - abs((int64_t)markerB - (int64_t)markerA);
    mend = (int64_t)markerB;
    if ((mstart<0) || (mstart > duration) || (mend<0) || (mend > duration))
    {
        GUI_Error_HIG(QT_TRANSLATE_NOOP("fadeThrough","Not possible!\nStart and/or end time would be out of range"), NULL);
        return;
    }

    myFly->param.startTime = mstart / 1000LL;
    myFly->param.endTime = mend / 1000LL;
    if(myFly->param.startTime > myFly->param.endTime)
    {
        uint32_t tmp=myFly->param.startTime;
        myFly->param.startTime=myFly->param.endTime;
        myFly->param.endTime=tmp;
    }
    valueChanged(0);
}

void Ui_fadeThroughWindow::tabChanged(int foo)
{
    myFly->redrawScene();
}

void Ui_fadeThroughWindow::gather(fadeThrough *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(fadeThrough));
}
Ui_fadeThroughWindow::~Ui_fadeThroughWindow()
{
    if(myFly) delete myFly;
    myFly=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
    scene=NULL;
}

void Ui_fadeThroughWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_fadeThroughWindow::valueChangedSpinBox(double f)
{
#define UPDATESILDERSFROMSPINBOXES(x,scale) \
    ui.horizontalSliderPeak##x->setValue( std::round(ui.doubleSpinBoxPeak##x->value()*scale)); \
    ui.horizontalSliderTransientDuration##x->setValue( std::round(ui.doubleSpinBoxTransientDuration##x->value()*100.0));

    UPDATESILDERSFROMSPINBOXES(Bright,100);
    UPDATESILDERSFROMSPINBOXES(Sat,100);
    UPDATESILDERSFROMSPINBOXES(Blend,100);
    UPDATESILDERSFROMSPINBOXES(Blur,1);
    UPDATESILDERSFROMSPINBOXES(Rot,100);
    UPDATESILDERSFROMSPINBOXES(Zoom,100);
    UPDATESILDERSFROMSPINBOXES(Vignette,100);
#undef UPDATESILDERSFROMSPINBOXES

    valueChanged(0);
}

void Ui_fadeThroughWindow::pushedColorBlend()
{
    QPalette indctrPalette(ui.lineEditColorBlend->palette());
    QColor startColor = indctrPalette.color(QPalette::Window);
    QColor color = QColorDialog::getColor(startColor, this );
    if( color.isValid() )
    {
        int rgb[3];
        color.getRgb(rgb+0,rgb+1,rgb+2, NULL);
        myFly->param.rgbColorBlend = (rgb[0] << 16) + (rgb[1] << 8) + (rgb[2] << 0);
        indctrPalette.setColor(QPalette::Window,color);
        indctrPalette.setColor(QPalette::Base,color);
        indctrPalette.setColor(QPalette::AlternateBase,color);
        ui.lineEditColorBlend->setPalette(indctrPalette);
        if(!lock)
        {
            lock++;
            myFly->download();
            myFly->sameImage();
            lock--;
        }
    }
}

void Ui_fadeThroughWindow::pushedColorVignette()
{
    QPalette indctrPalette(ui.lineEditColorVignette->palette());
    QColor startColor = indctrPalette.color(QPalette::Window);
    QColor color = QColorDialog::getColor(startColor, this );
    if( color.isValid() )
    {
        int rgb[3];
        color.getRgb(rgb+0,rgb+1,rgb+2, NULL);
        myFly->param.rgbColorVignette = (rgb[0] << 16) + (rgb[1] << 8) + (rgb[2] << 0);
        indctrPalette.setColor(QPalette::Window,color);
        indctrPalette.setColor(QPalette::Base,color);
        indctrPalette.setColor(QPalette::AlternateBase,color);
        ui.lineEditColorVignette->setPalette(indctrPalette);
        if(!lock)
        {
            lock++;
            myFly->download();
            myFly->sameImage();
            lock--;
        }
    }
}

/**
 * \fn reset
 */
void Ui_fadeThroughWindow::reset( bool f )
{
    myFly->param.enableBright = false;
    myFly->param.enableSat = false;
    myFly->param.enableBlend = false;
    myFly->param.enableBlur = false;
    myFly->param.enableRot = false;
    myFly->param.enableZoom = false;
    myFly->param.enableVignette = false;
    myFly->param.rgbColorBlend = 0;
    myFly->param.rgbColorVignette = 0;
    myFly->param.peakBright = 1.0;
    myFly->param.peakSat = 1.0;
    myFly->param.peakBlend = 1.0;
    myFly->param.peakBlur = 0;
    myFly->param.peakRot = 0;
    myFly->param.peakZoom = 1.0;
    myFly->param.peakVignette = 0;
    myFly->param.transientBright = 0;
    myFly->param.transientSat = 0;
    myFly->param.transientBlend = 0;
    myFly->param.transientBlur = 0;
    myFly->param.transientRot = 0;
    myFly->param.transientZoom = 0;
    myFly->param.transientVignette = 0;
    
    float defaultTrD = 0.5;
    if (ADMVideoFadeThrough::IsFadeIn() || ADMVideoFadeThrough::IsFadeOut())
        defaultTrD = 1.0;
    myFly->param.transientDurationBright = defaultTrD;
    myFly->param.transientDurationSat = defaultTrD;
    myFly->param.transientDurationBlend = defaultTrD;
    myFly->param.transientDurationBlur = defaultTrD;
    myFly->param.transientDurationRot = defaultTrD;
    myFly->param.transientDurationZoom = defaultTrD;
    myFly->param.transientDurationVignette = defaultTrD;
    lock++;
    myFly->upload();
    myFly->sameImage();
    lock--;
}

void Ui_fadeThroughWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_fadeThroughWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();

    QFontMetrics fm = ui.labelTScope->fontMetrics();
    QString text = QString(QT_TRANSLATE_NOOP("fadeThrough","Time scope: "));
    text += QString("000:00:00,000 - 000:00:00,000");
    ui.labelTScope->setMinimumWidth(1.05 * fm.boundingRect(text).width());
    text = QString(QT_TRANSLATE_NOOP("fadeThrough","Duration: "));
    text += QString("000:00:00,000---");
    if (!(ADMVideoFadeThrough::IsFadeIn() || ADMVideoFadeThrough::IsFadeOut()))
        ui.labelCenter->setMinimumWidth(1.05 * fm.boundingRect(text).width());
    ui.labelDuration->setMinimumWidth(1.05 * fm.boundingRect(text).width());

    adjustSize();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}


bool flyFadeThrough::getTabEnabled(int tabIndex)
{
    switch(tabIndex)
    {
        case 0:
            return param.enableBright;
        case 1:
            return param.enableSat;
        case 2:
            return param.enableBlend;
        case 3:
            return param.enableBlur;
        case 4:
            return param.enableRot;
        case 5:
            return param.enableZoom;
        case 6:
            return param.enableVignette;
        default:
            return false;
    }
}

int flyFadeThrough::getTabTransient(int tabIndex)
{
    switch(tabIndex)
    {
        case 0:
            return param.transientBright;
        case 1:
            return param.transientSat;
        case 2:
            return param.transientBlend;
        case 3:
            return param.transientBlur;
        case 4:
            return param.transientRot;
        case 5:
            return param.transientZoom;
        case 6:
            return param.transientVignette;
        default:
            return 0;
    }
}

double flyFadeThrough::getTabTransientDuration(int tabIndex)
{
    switch(tabIndex)
    {
        case 0:
            return (double)param.transientDurationBright;
        case 1:
            return (double)param.transientDurationSat;
        case 2:
            return (double)param.transientDurationBlend;
        case 3:
            return (double)param.transientDurationBlur;
        case 4:
            return (double)param.transientDurationRot;
        case 5:
            return (double)param.transientDurationZoom;
        case 6:
            return (double)param.transientDurationVignette;
        default:
            return 0;
    }
}

void flyFadeThrough::redrawScene()
{
    if (!scene)
        return;
    Ui_fadeThroughDialog *w=(Ui_fadeThroughDialog *)_cookie;
    int activeTab = w->tabEffects->currentIndex();
    
    scene->clear();
    for (int i=0; i<=7; i++)
    {
        if (i==activeTab)
            continue;
        if (i==7)
            i = activeTab;

        QColor color((i == activeTab) ? Qt::red : Qt::lightGray);
        QPen pen(color);
        QLineF qline;
        
        int transient = getTabTransient(i);
        double transientDuration = getTabTransientDuration(i);
        int prevx=0,prevy, px,py;
        if (getTabEnabled(i))
        {
            for (int t=0; t<=256; t++)
            {
                px = t+1;
                py = 127.0*(1.0-ADMVideoFadeThrough::TransientPoint(t/256.0, transient, transientDuration));
                if (t==0)
                    prevy=py;
                qline = QLineF(prevx,prevy,px,py);
                prevx = px;
                prevy = py;
                scene->addLine(qline,pen);
            }
        } else {
            qline = QLineF(0,127,255,127);
            scene->addLine(qline,pen);
        }
        
        if (i == activeTab)
            break;
    }
}

#define MYCOMBOX(x) w->comboBox##x
#define MYDBLSPIN(x) w->doubleSpinBox##x
//************************
uint8_t flyFadeThrough::upload()
{
    Ui_fadeThroughDialog *w=(Ui_fadeThroughDialog *)_cookie;

#define UPLOADALIKES(x,scale) \
    w->checkBoxEnable##x->setChecked(param.enable##x); \
    w->comboBoxTransient##x->setCurrentIndex(param.transient##x); \
    w->horizontalSliderPeak##x->setValue( std::round(param.peak##x*scale)); \
    w->doubleSpinBoxPeak##x->setValue(param.peak##x); \
    w->horizontalSliderTransientDuration##x->setValue( std::round(param.transientDuration##x*100.0)); \
    w->doubleSpinBoxTransientDuration##x->setValue(param.transientDuration##x);

    UPLOADALIKES(Bright,100);
    UPLOADALIKES(Sat,100);
    UPLOADALIKES(Blend,100);
    UPLOADALIKES(Blur,1);
    UPLOADALIKES(Rot,100);
    UPLOADALIKES(Zoom,100);
    UPLOADALIKES(Vignette,100);
#undef UPLOADALIKES

    int rgb[3];

    {
        rgb[0] = (param.rgbColorBlend>>16)&0xFF;
        rgb[1] = (param.rgbColorBlend>>8)&0xFF;
        rgb[2] = (param.rgbColorBlend>>0)&0xFF;
    
        QLineEdit * indctr = w->lineEditColorBlend;
        QPalette indctrPalette(indctr->palette());
        QColor color;
        color.setRgb(rgb[0],rgb[1],rgb[2],255);
        indctrPalette.setColor(QPalette::Window,color);
        indctrPalette.setColor(QPalette::Base,color);
        indctrPalette.setColor(QPalette::AlternateBase,color);
        indctr->setPalette(indctrPalette);
    }

    {
        rgb[0] = (param.rgbColorVignette>>16)&0xFF;
        rgb[1] = (param.rgbColorVignette>>8)&0xFF;
        rgb[2] = (param.rgbColorVignette>>0)&0xFF;
        
        QLineEdit * indctr = w->lineEditColorVignette;
        QPalette indctrPalette(indctr->palette());
        QColor color;
        color.setRgb(rgb[0],rgb[1],rgb[2],255);
        indctrPalette.setColor(QPalette::Window,color);
        indctrPalette.setColor(QPalette::Base,color);
        indctrPalette.setColor(QPalette::AlternateBase,color);
        indctr->setPalette(indctrPalette);
    }
    
    redrawScene();
    
    QString tstr = QString(QT_TRANSLATE_NOOP("fadeThrough","Time scope: "));
    tstr += QString(ADM_us2plain(param.startTime*1000LL));
    tstr += QString(" - ");
    tstr += QString(ADM_us2plain(param.endTime*1000LL));
    w->labelTScope->setText(tstr);
    
    if (!(ADMVideoFadeThrough::IsFadeIn() || ADMVideoFadeThrough::IsFadeOut()))
    {
        tstr = QString(QT_TRANSLATE_NOOP("fadeThrough","Center: "));
        tstr += QString(ADM_us2plain(((param.endTime + param.startTime)*1000LL)/2));
        w->labelCenter->setText(tstr);
    }
    
    tstr = QString(QT_TRANSLATE_NOOP("fadeThrough","Duration: "));
    tstr += QString(ADM_us2plain((param.endTime - param.startTime)*1000LL));
    w->labelDuration->setText(tstr);

    return 1;
}
uint8_t flyFadeThrough::download(void)
{
    Ui_fadeThroughDialog *w=(Ui_fadeThroughDialog *)_cookie;

#define DOWNLOADALIKES(x,scale) \
    param.enable##x = w->checkBoxEnable##x->isChecked(); \
    param.transient##x = w->comboBoxTransient##x->currentIndex(); \
    param.peak##x = (double)w->horizontalSliderPeak##x->value() / scale; \
    param.transientDuration##x = (double)w->horizontalSliderTransientDuration##x->value() / 100.0;

    DOWNLOADALIKES(Bright,100);
    DOWNLOADALIKES(Sat,100);
    DOWNLOADALIKES(Blend,100);
    DOWNLOADALIKES(Blur,1);
    DOWNLOADALIKES(Rot,100);
    DOWNLOADALIKES(Zoom,100);
    DOWNLOADALIKES(Vignette,100);
#undef DOWNLOADALIKES

    upload();
    return 1;
}
void flyFadeThrough::setTabOrder(void)
{
    Ui_fadeThroughDialog *w=(Ui_fadeThroughDialog *)_cookie;
    std::vector<QWidget *> controls;

#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHDBLSPIN(x) controls.push_back(MYDBLSPIN(x));
    controls.push_back(w->pushButtonTManual);
    controls.push_back(w->pushButtonTMarker);
    if (!(ADMVideoFadeThrough::IsFadeIn() || ADMVideoFadeThrough::IsFadeOut()))
        controls.push_back(w->pushButtonTCenter);
    controls.push_back(w->tabEffects);
    
#define PUSHWCOLOR(x) \
    controls.push_back(w->checkBoxEnable##x); \
    controls.push_back(w->pushButtonColor##x); \
    controls.push_back(w->doubleSpinBoxPeak##x); \
    controls.push_back(w->horizontalSliderPeak##x); \
    controls.push_back(w->comboBoxTransient##x); \
    controls.push_back(w->doubleSpinBoxTransientDuration##x); \
    controls.push_back(w->horizontalSliderTransientDuration##x);

#define PUSHWOCOLOR(x) \
    controls.push_back(w->checkBoxEnable##x); \
    controls.push_back(w->doubleSpinBoxPeak##x); \
    controls.push_back(w->horizontalSliderPeak##x); \
    controls.push_back(w->comboBoxTransient##x); \
    controls.push_back(w->doubleSpinBoxTransientDuration##x); \
    controls.push_back(w->horizontalSliderTransientDuration##x);

    PUSHWOCOLOR(Bright);
    PUSHWOCOLOR(Sat);
    PUSHWCOLOR(Blend);
    PUSHWOCOLOR(Blur);
    PUSHWOCOLOR(Rot);
    PUSHWOCOLOR(Zoom);
    PUSHWCOLOR(Vignette);


    controls.insert(controls.end(), buttonList.begin(), buttonList.end());
    controls.push_back(w->horizontalSlider);

    QWidget *first, *second;

    for(std::vector<QWidget *>::iterator tor = controls.begin(); tor != controls.end(); ++tor)
    {
        if(tor+1 == controls.end()) break;
        first = *tor;
        second = *(tor+1);
        _parent->setTabOrder(first,second);
        //ADM_info("Tab order: %p (%s) --> %p (%s)\n",first,first->objectName().toUtf8().constData(),second,second->objectName().toUtf8().constData());
    }
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_getFadeThrough(fadeThrough *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_fadeThroughWindow dialog(qtLastRegisteredDialog(), param,in);

    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=1;
    }

    qtUnregisterDialog(&dialog);

    return ret;
}
//____________________________________
// EOF


