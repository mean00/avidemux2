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
#include "Q_fadeInOut.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidFadeInOut.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
#include <cmath>
//
//	Video is in YV12 Colorspace
//
//
Ui_fadeInOutWindow::Ui_fadeInOutWindow(QWidget *parent, fadeInOut *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        if (ADMVideoFadeInOut::FadeDirIsIn())
            this->setWindowTitle(QString(QT_TRANSLATE_NOOP("fadeInOut","Fade in")));
        else
            this->setWindowTitle(QString(QT_TRANSLATE_NOOP("fadeInOut","Fade out")));
            
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;
        markerA = in->getInfo()->markerA;
        markerB = in->getInfo()->markerB;
        duration = in->getInfo()->totalDuration;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyFadeInOut( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(fadeInOut));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        
        connect(ui.pushButtonTManual,SIGNAL(clicked(bool)),this,SLOT(manualTimeEntry(bool)));
        connect(ui.pushButtonTMarker,SIGNAL(clicked(bool)),this,SLOT(timesFromMarkers(bool)));
        
        connect(ui.pushButtonColor,SIGNAL(released()),this,SLOT(pushedColor()));

        QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        setModal(true);
}

void Ui_fadeInOutWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}

void Ui_fadeInOutWindow::manualTimeEntry(bool f)
{
    uint32_t mx=(uint32_t)(duration/1000LL);

    diaElemTimeStamp start(&(myFly->param.startTime),QT_TRANSLATE_NOOP("fadeInOut","_Start time:"),0,mx);
    diaElemTimeStamp end(&(myFly->param.endTime),QT_TRANSLATE_NOOP("fadeInOut","_End time:"),0,mx);
    diaElem *elems[2]={&start,&end};

    if(diaFactoryRun(QT_TRANSLATE_NOOP("fadeInOut","Manual time entry"),2+0*1,elems))
    {
        if(myFly->param.startTime > myFly->param.endTime)
        {
            uint32_t tmp=myFly->param.startTime;
            myFly->param.startTime=myFly->param.endTime;
            myFly->param.endTime=tmp;
        }
        if(lock) return;
        lock++;
        myFly->download();
        myFly->sameImage();
        lock--;
    }
}

void Ui_fadeInOutWindow::timesFromMarkers(bool f)
{
    myFly->param.startTime = markerA / 1000LL;
    myFly->param.endTime = markerB / 1000LL;
    if(myFly->param.startTime > myFly->param.endTime)
    {
        uint32_t tmp=myFly->param.startTime;
        myFly->param.startTime=myFly->param.endTime;
        myFly->param.endTime=tmp;
    }
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_fadeInOutWindow::gather(fadeInOut *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(fadeInOut));
}
Ui_fadeInOutWindow::~Ui_fadeInOutWindow()
{
    if(myFly) delete myFly;
    myFly=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
}

void Ui_fadeInOutWindow::pushedColor()
{
    QPalette indctrPalette(ui.lineEditColor->palette());
    QColor startColor = indctrPalette.color(QPalette::Window);
    QColor color = QColorDialog::getColor(startColor, this );
    if( color.isValid() )
    {
        int rgb[3];
        color.getRgb(rgb+0,rgb+1,rgb+2, NULL);
        myFly->param.rgbColor = (rgb[0] << 16) + (rgb[1] << 8) + (rgb[2] << 0);
        indctrPalette.setColor(QPalette::Window,color);
        indctrPalette.setColor(QPalette::Base,color);
        indctrPalette.setColor(QPalette::AlternateBase,color);
        ui.lineEditColor->setPalette(indctrPalette);
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
void Ui_fadeInOutWindow::reset( bool f )
{
    myFly->param.rgbColor = 0;
    lock++;
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_fadeInOutWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_fadeInOutWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
    
    QFontMetrics fm = ui.labelTScope->fontMetrics();
    QString text = QString(QT_TRANSLATE_NOOP("fadeInOut","Time scope: "));
    text += QString("000:00:00,000 - 000:00:00,000");
    ui.labelTScope->setMinimumWidth(1.05 * fm.boundingRect(text).width());
    text = QString(QT_TRANSLATE_NOOP("fadeInOut","Duration: "));
    text += QString("000:00:00,000---");
    ui.labelDuration->setMinimumWidth(1.05 * fm.boundingRect(text).width());
    
}

#define MYCOMBOX(x) w->comboBox##x
#define MYDBLSPIN(x) w->doubleSpinBox##x
//************************
uint8_t flyFadeInOut::upload()
{
    Ui_fadeInOutDialog *w=(Ui_fadeInOutDialog *)_cookie;
    int rgb[3];
    rgb[0] = (param.rgbColor>>16)&0xFF;
    rgb[1] = (param.rgbColor>>8)&0xFF;
    rgb[2] = (param.rgbColor>>0)&0xFF;
    
    QLineEdit * indctr = w->lineEditColor;
    QPalette indctrPalette(indctr->palette());
    QColor color;
    color.setRgb(rgb[0],rgb[1],rgb[2],255);
    indctrPalette.setColor(QPalette::Window,color);
    indctrPalette.setColor(QPalette::Base,color);
    indctrPalette.setColor(QPalette::AlternateBase,color);
    indctr->setPalette(indctrPalette);

    QString tstr = QString(QT_TRANSLATE_NOOP("fadeInOut","Time scope: "));
    tstr += QString(ADM_us2plain(param.startTime*1000LL));
    tstr += QString(" - ");
    tstr += QString(ADM_us2plain(param.endTime*1000LL));
    w->labelTScope->setText(tstr);
    
    tstr = QString(QT_TRANSLATE_NOOP("fadeInOut","Duration: "));
    tstr += QString(ADM_us2plain((param.endTime - param.startTime)*1000LL));
    w->labelDuration->setText(tstr);

    return 1;
}
uint8_t flyFadeInOut::download(void)
{
    //Ui_fadeInOutDialog *w=(Ui_fadeInOutDialog *)_cookie;
    upload();
    return 1;
}
void flyFadeInOut::setTabOrder(void)
{
    Ui_fadeInOutDialog *w=(Ui_fadeInOutDialog *)_cookie;
    std::vector<QWidget *> controls;

#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHDBLSPIN(x) controls.push_back(MYDBLSPIN(x));
    controls.push_back(w->pushButtonTManual);
    controls.push_back(w->pushButtonTMarker);
    controls.push_back(w->pushButtonColor);


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
uint8_t DIA_getFadeInOut(fadeInOut *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_fadeInOutWindow dialog(qtLastRegisteredDialog(), param,in);

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


