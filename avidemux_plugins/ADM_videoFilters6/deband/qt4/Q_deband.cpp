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
#include "Q_deband.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidDeband.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_debandWindow::Ui_debandWindow(QWidget *parent, deband *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyDeband( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(deband));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \
        connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSpinBox(int)));
        SPINNER(Range)
        SPINNER(LumaThreshold)
        SPINNER(ChromaThreshold)

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_debandWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_debandWindow::gather(deband *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(deband));
}
Ui_debandWindow::~Ui_debandWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
#define COPY_VALUE_TO_SPINBOX(x) \
        ui.spinBox##x->blockSignals(true); \
        ui.spinBox##x->setValue(ui.horizontalSlider##x->value()); \
        ui.spinBox##x->blockSignals(false);
void Ui_debandWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SPINBOX(Range);
    COPY_VALUE_TO_SPINBOX(LumaThreshold);
    COPY_VALUE_TO_SPINBOX(ChromaThreshold);
    myFly->download();
    myFly->sameImage();
    lock--;
}
#define COPY_VALUE_TO_SLIDER(x) \
        ui.horizontalSlider##x->blockSignals(true); \
        ui.horizontalSlider##x->setValue(ui.spinBox##x->value()); \
        ui.horizontalSlider##x->blockSignals(false);
void Ui_debandWindow::valueChangedSpinBox(int foo)
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SLIDER(Range);
    COPY_VALUE_TO_SLIDER(LumaThreshold);
    COPY_VALUE_TO_SLIDER(ChromaThreshold);
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_debandWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoDeband::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_debandWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_debandWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSLIDER(x) w->horizontalSlider##x
#define MYSPIN(x) w->spinBox##x
#define MYCHECK(x) w->checkBox##x
#define UPLOADSPIN(x, value) \
        w->spinBox##x->blockSignals(true); \
        w->spinBox##x->setValue(value); \
        w->spinBox##x->blockSignals(false);
//************************
uint8_t flyDeband::upload(void)
{
    Ui_debandDialog *w=(Ui_debandDialog *)_cookie;

    MYSLIDER(Range)->setValue((int)param.range);
    UPLOADSPIN(Range, param.range);
    MYSLIDER(LumaThreshold)->setValue((int)param.lumaThreshold);
    UPLOADSPIN(LumaThreshold, param.lumaThreshold);
    MYSLIDER(ChromaThreshold)->setValue((int)param.chromaThreshold);
    UPLOADSPIN(ChromaThreshold, param.chromaThreshold);
    return 1;
}
uint8_t flyDeband::download(void)
{
    Ui_debandDialog *w=(Ui_debandDialog *)_cookie;
    param.range=(int)MYSLIDER(Range)->value();
    param.lumaThreshold=(int)MYSLIDER(LumaThreshold)->value();
    param.chromaThreshold=(int)MYSLIDER(ChromaThreshold)->value();
    return 1;
}
void flyDeband::setTabOrder(void)
{
    Ui_debandDialog *w=(Ui_debandDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    PUSH_SLIDER(Range)
    PUSH_SPIN(Range)
    PUSH_SLIDER(LumaThreshold)
    PUSH_SPIN(LumaThreshold)
    PUSH_SLIDER(ChromaThreshold)
    PUSH_SPIN(ChromaThreshold)

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
uint8_t DIA_getDeband(deband *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_debandWindow dialog(qtLastRegisteredDialog(), param,in);

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


