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
#include "Q_colorTemp.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidColorTemp.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_colorTempWindow::Ui_colorTempWindow(QWidget *parent, colorTemp *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyColorTemp( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(colorTemp));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \
        connect( ui.doubleSpinBox##x,SIGNAL(valueChanged(double)),this,SLOT(valueChangedSpinBox(double)));
        SPINNER(Temperature)
        SPINNER(Angle)

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_colorTempWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_colorTempWindow::gather(colorTemp *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(colorTemp));
}
Ui_colorTempWindow::~Ui_colorTempWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
#define COPY_VALUE_TO_SPINBOX(x,y) \
        ui.doubleSpinBox##x->blockSignals(true); \
        ui.doubleSpinBox##x->setValue((float)ui.horizontalSlider##x->value() / y); \
        ui.doubleSpinBox##x->blockSignals(false);
void Ui_colorTempWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SPINBOX(Temperature, 100.0);
    COPY_VALUE_TO_SPINBOX(Angle, 1.0);
    myFly->download();
    myFly->sameImage();
    lock--;
}
#define COPY_VALUE_TO_SLIDER(x,y) \
        ui.horizontalSlider##x->blockSignals(true); \
        ui.horizontalSlider##x->setValue((int)round(ui.doubleSpinBox##x->value() * y)); \
        ui.horizontalSlider##x->blockSignals(false);
void Ui_colorTempWindow::valueChangedSpinBox(double foo)
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SLIDER(Temperature, 100.0);
    COPY_VALUE_TO_SLIDER(Angle, 1.0);
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_colorTempWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoColorTemp::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}

void Ui_colorTempWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_colorTempWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYDBLSPIN(x) w->doubleSpinBox##x
#define MYCHECK(x) w->checkBox##x
#define UPLOADDBLSPIN(x, value) \
        w->doubleSpinBox##x->blockSignals(true); \
        w->doubleSpinBox##x->setValue(value); \
        w->doubleSpinBox##x->blockSignals(false);
//************************
uint8_t flyColorTemp::upload(void)
{
    Ui_colorTempDialog *w=(Ui_colorTempDialog *)_cookie;
    MYSPIN(Temperature)->setValue((int)round(param.temperature*100.0));
    UPLOADDBLSPIN(Temperature, param.temperature);
    MYSPIN(Angle)->setValue((int)param.angle);
    UPLOADDBLSPIN(Angle, param.angle);
    return 1;
}
uint8_t flyColorTemp::download(void)
{
    Ui_colorTempDialog *w=(Ui_colorTempDialog *)_cookie;
    param.temperature=((float)MYSPIN(Temperature)->value()) / 100.0;
    param.angle=MYSPIN(Angle)->value();
    return 1;
}
void flyColorTemp::setTabOrder(void)
{
    Ui_colorTempDialog *w=(Ui_colorTempDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHSLIDER(x) controls.push_back(MYSPIN(x));
#define PUSHDBLSPIN(x) controls.push_back(MYDBLSPIN(x));
    PUSHSLIDER(Temperature)
    PUSHDBLSPIN(Temperature)
    PUSHSLIDER(Angle)
    PUSHDBLSPIN(Angle)

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
uint8_t DIA_getColorTemp(colorTemp *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_colorTempWindow dialog(qtLastRegisteredDialog(), param,in);

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


