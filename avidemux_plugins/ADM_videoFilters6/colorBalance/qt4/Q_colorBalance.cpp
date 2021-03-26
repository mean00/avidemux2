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
#include "Q_colorBalance.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidColorBalance.h"
#include <cmath>
#include <QPalette>
#include <QColor>
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

//
//	Video is in YV12 Colorspace
//
//

Ui_colorBalanceWindow::Ui_colorBalanceWindow(QWidget *parent, colorBalance *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyColorBalance( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(colorBalance));
        myFly->showOriginal = false;
        myFly->showRanges = false;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.slider##x->setScale(1,y,z); \
        connect( ui.slider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(LoLuma,100,2)
        SPINNER(MdLuma,100,2)
        SPINNER(HiLuma,100,2)
        SPINNER(LoChromaShift,100,2)
        SPINNER(MdChromaShift,100,2)
        SPINNER(HiChromaShift,100,2)
        SPINNER(LoSaturation,100,2)
        SPINNER(MdSaturation,100,2)
        SPINNER(HiSaturation,100,2)

#define DIAL(x) connect( ui.dial##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        DIAL(LoHue)
        DIAL(MdHue)
        DIAL(HiHue)

        connect( ui.pushButtonPeek,SIGNAL(pressed()),this,SLOT(peekPressed()));
        connect( ui.pushButtonPeek,SIGNAL(released()),this,SLOT(peekReleased()));
        connect( ui.pushButtonRanges,SIGNAL(pressed()),this,SLOT(rangesPressed()));
        connect( ui.pushButtonRanges,SIGNAL(released()),this,SLOT(rangesReleased()));

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_colorBalanceWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_colorBalanceWindow::gather(colorBalance *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(colorBalance));
}
Ui_colorBalanceWindow::~Ui_colorBalanceWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_colorBalanceWindow::rgb2yuv(int * yuv, int * rgb)
{
    yuv[0] = std::round( 0.299*rgb[0] + 0.587*rgb[1] + 0.114*rgb[2]);
    yuv[1] = std::round(-0.169*rgb[0] - 0.331*rgb[1] + 0.500*rgb[2]);
    yuv[2] = std::round( 0.500*rgb[0] - 0.419*rgb[1] - 0.081*rgb[2]);
    if (yuv[0] <   0) yuv[0] = 0;
    if (yuv[0] > 255) yuv[0] = 255;
    if (yuv[1] < -128) yuv[1] = -128;
    if (yuv[1] >  127) yuv[1] = 127;
    if (yuv[2] < -128) yuv[2] = -128;
    if (yuv[2] >  127) yuv[2] = 127;
}
void Ui_colorBalanceWindow::yuv2rgb(int * rgb, int * yuv)
{
    rgb[0] = std::round(yuv[0]                +   1.4*yuv[2]);
    rgb[1] = std::round(yuv[0] - 0.343*yuv[1] - 0.711*yuv[2]);
    rgb[2] = std::round(yuv[0] + 1.765*yuv[1]               );
    for (int i=0; i<3; i++)
    {
        if (rgb[i] < 0) rgb[i] = 0;
        if (rgb[i] > 255) rgb[i] = 255;
    }
}
void Ui_colorBalanceWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_colorBalanceWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoColorBalance::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_colorBalanceWindow::peekPressed(void)
{
    myFly->showOriginal = true;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;
}
void Ui_colorBalanceWindow::peekReleased(void)
{
    myFly->showOriginal = false;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;
}
void Ui_colorBalanceWindow::rangesPressed(void)
{
    myFly->showRanges = true;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;
}
void Ui_colorBalanceWindow::rangesReleased(void)
{
    myFly->showRanges = false;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;
}
void Ui_colorBalanceWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_colorBalanceWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

void Ui_colorBalanceWindow::setHueColor(QDial * d, int angle)
{
    int rgb[3],yuv[3];
    yuv[0] = 160;//96;
    yuv[1] = cos(M_PI*(angle/180.0))*127;
    yuv[2] = sin(M_PI*(angle/180.0))*127;
    yuv2rgb(rgb, yuv);
    QPalette pal = d->palette();
    pal.setColor(QPalette::Button, QColor(rgb[0],rgb[1],rgb[2]));
    d->setAutoFillBackground(true);
    d->setPalette(pal);
    d->show();
    angle = 270-angle;
    while (angle < 0) angle += 360;
    d->setValue(angle);

}

int Ui_colorBalanceWindow::getHueColor(QDial * d)
{
    int angle = d->value();
    angle = 270-angle;
    while (angle < 0) angle += 360;
    int rgb[3],yuv[3];
    yuv[0] = 160;//96;
    yuv[1] = cos(M_PI*(angle/180.0))*127;
    yuv[2] = sin(M_PI*(angle/180.0))*127;
    yuv2rgb(rgb, yuv);
    QPalette pal = d->palette();
    pal.setColor(QPalette::Button, QColor(rgb[0],rgb[1],rgb[2]));
    d->setAutoFillBackground(true);
    d->setPalette(pal);
    d->show();
    return angle;
}

#define MYSPIN(x) w->slider##x
#define MYDIAL(x) w->dial##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyColorBalance::upload(void)
{
    Ui_colorBalanceDialog *w=(Ui_colorBalanceDialog *)_cookie;

    MYSPIN(LoLuma)->setValue((int)round(param.loLuma*100.0));
    MYSPIN(MdLuma)->setValue((int)round(param.mdLuma*100.0));
    MYSPIN(HiLuma)->setValue((int)round(param.hiLuma*100.0));
    Ui_colorBalanceWindow::setHueColor(MYDIAL(LoHue), (int)round(param.loAngle));
    Ui_colorBalanceWindow::setHueColor(MYDIAL(MdHue), (int)round(param.mdAngle));
    Ui_colorBalanceWindow::setHueColor(MYDIAL(HiHue), (int)round(param.hiAngle));
    MYSPIN(LoChromaShift)->setValue((int)round(param.loShift*100.0));
    MYSPIN(MdChromaShift)->setValue((int)round(param.mdShift*100.0));
    MYSPIN(HiChromaShift)->setValue((int)round(param.hiShift*100.0));
    MYSPIN(LoSaturation)->setValue((int)round(param.loSaturation*100.0));
    MYSPIN(MdSaturation)->setValue((int)round(param.mdSaturation*100.0));
    MYSPIN(HiSaturation)->setValue((int)round(param.hiSaturation*100.0));

    return 1;
}
uint8_t flyColorBalance::download(void)
{
    Ui_colorBalanceDialog *w=(Ui_colorBalanceDialog *)_cookie;

    param.loLuma=((float)MYSPIN(LoLuma)->value()) / 100.0;
    param.mdLuma=((float)MYSPIN(MdLuma)->value()) / 100.0;
    param.hiLuma=((float)MYSPIN(HiLuma)->value()) / 100.0;
    param.loAngle=Ui_colorBalanceWindow::getHueColor(MYDIAL(LoHue));
    param.mdAngle=Ui_colorBalanceWindow::getHueColor(MYDIAL(MdHue));
    param.hiAngle=Ui_colorBalanceWindow::getHueColor(MYDIAL(HiHue));
    param.loShift=((float)MYSPIN(LoChromaShift)->value()) / 100.0;
    param.mdShift=((float)MYSPIN(MdChromaShift)->value()) / 100.0;
    param.hiShift=((float)MYSPIN(HiChromaShift)->value()) / 100.0;
    param.loSaturation=((float)MYSPIN(LoSaturation)->value()) / 100.0;
    param.mdSaturation=((float)MYSPIN(MdSaturation)->value()) / 100.0;
    param.hiSaturation=((float)MYSPIN(HiSaturation)->value()) / 100.0;

    return 1;
}
void flyColorBalance::setTabOrder(void)
{
    Ui_colorBalanceDialog *w=(Ui_colorBalanceDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_DIAL(x) controls.push_back(MYDIAL(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    PUSH_SPIN(LoLuma)
    PUSH_DIAL(LoHue)
    PUSH_SPIN(LoChromaShift)
    PUSH_SPIN(LoSaturation)

    PUSH_SPIN(MdLuma)
    PUSH_DIAL(MdHue)
    PUSH_SPIN(MdChromaShift)
    PUSH_SPIN(MdSaturation)

    PUSH_SPIN(HiLuma)
    PUSH_DIAL(HiHue)
    PUSH_SPIN(HiChromaShift)
    PUSH_SPIN(HiSaturation)

    controls.push_back(w->pushButtonRanges);
    controls.push_back(w->pushButtonPeek);

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
uint8_t DIA_getColorBalance(colorBalance *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_colorBalanceWindow dialog(qtLastRegisteredDialog(), param,in);

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


