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
#include "ADM_default.h"
#include "Q_fadeFromImage.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidFadeFromImage.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_fadeFromImageWindow::Ui_fadeFromImageWindow(QWidget *parent, fadeFromImage *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;
        markerA = in->getInfo()->markerA;
        markerB = in->getInfo()->markerB;
        duration = in->getInfo()->totalDuration;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyFadeFromImage( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(fadeFromImage));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        
        connect(ui.pushButtonTManual,SIGNAL(clicked(bool)),this,SLOT(manualTimeEntry(bool)));
        connect(ui.pushButtonTMarker,SIGNAL(clicked(bool)),this,SLOT(timesFromMarkers(bool)));
        
        connect(ui.comboBoxEffect, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));
        connect(ui.comboBoxDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));

        QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        setModal(true);
}

void Ui_fadeFromImageWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}

void Ui_fadeFromImageWindow::manualTimeEntry(bool f)
{
    uint32_t mx=(uint32_t)(duration/1000LL);

    diaElemTimeStamp start(&(myFly->param.startTime),QT_TRANSLATE_NOOP("fadeFromImage","_Start time:"),0,mx);
    diaElemTimeStamp end(&(myFly->param.endTime),QT_TRANSLATE_NOOP("fadeFromImage","_End time:"),0,mx);
    diaElem *elems[2]={&start,&end};

    if(diaFactoryRun(QT_TRANSLATE_NOOP("fadeFromImage","Manual time entry"),2+0*1,elems))
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

void Ui_fadeFromImageWindow::timesFromMarkers(bool f)
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

void Ui_fadeFromImageWindow::gather(fadeFromImage *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(fadeFromImage));
}
Ui_fadeFromImageWindow::~Ui_fadeFromImageWindow()
{
    if(myFly) delete myFly;
    myFly=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
}

void Ui_fadeFromImageWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

/**
 * \fn reset
 */
void Ui_fadeFromImageWindow::reset( bool f )
{
    myFly->param.effect = 0;
    myFly->param.direction = 0;
    lock++;
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_fadeFromImageWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_fadeFromImageWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
    
    QFontMetrics fm = ui.labelTScope->fontMetrics();
    QString text = QString(QT_TRANSLATE_NOOP("fadeFromImage","Time scope: "));
    text += QString("000:00:00,000 - 000:00:00,000");
    ui.labelTScope->setMinimumWidth(1.05 * fm.boundingRect(text).width());
    text = QString(QT_TRANSLATE_NOOP("fadeFromImage","Duration: "));
    text += QString("000:00:00,000---");
    ui.labelDuration->setMinimumWidth(1.05 * fm.boundingRect(text).width());
    
}

#define MYCOMBOX(x) w->comboBox##x
#define MYDBLSPIN(x) w->doubleSpinBox##x
//************************
uint8_t flyFadeFromImage::upload()
{
    Ui_fadeFromImageDialog *w=(Ui_fadeFromImageDialog *)_cookie;
    MYCOMBOX(Effect)->setCurrentIndex(param.effect);
    MYCOMBOX(Direction)->setCurrentIndex(param.direction);
    MYCOMBOX(Direction)->setVisible(param.effect>=1 && param.effect<=3);
    
    QString tstr = QString(QT_TRANSLATE_NOOP("fadeFromImage","Time scope: "));
    tstr += QString(ADM_us2plain(param.startTime*1000LL));
    tstr += QString(" - ");
    tstr += QString(ADM_us2plain(param.endTime*1000LL));
    w->labelTScope->setText(tstr);
    
    tstr = QString(QT_TRANSLATE_NOOP("fadeFromImage","Duration: "));
    tstr += QString(ADM_us2plain((param.endTime - param.startTime)*1000LL));
    w->labelDuration->setText(tstr);

    return 1;
}
uint8_t flyFadeFromImage::download(void)
{
    Ui_fadeFromImageDialog *w=(Ui_fadeFromImageDialog *)_cookie;
    param.effect=MYCOMBOX(Effect)->currentIndex();
    param.direction=MYCOMBOX(Direction)->currentIndex();

    upload();
    return 1;
}
void flyFadeFromImage::setTabOrder(void)
{
    Ui_fadeFromImageDialog *w=(Ui_fadeFromImageDialog *)_cookie;
    std::vector<QWidget *> controls;

#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHDBLSPIN(x) controls.push_back(MYDBLSPIN(x));
    controls.push_back(w->pushButtonTManual);
    controls.push_back(w->pushButtonTMarker);
    PUSHCOMBOX(Effect)
    PUSHCOMBOX(Direction)


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
uint8_t DIA_getFadeFromImage(fadeFromImage *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_fadeFromImageWindow dialog(qtLastRegisteredDialog(), param,in);

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


