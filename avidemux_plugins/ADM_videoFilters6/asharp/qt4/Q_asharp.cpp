/***************************************************************************
                         Q_Asharp.cpp  -  description
                             -------------------

			     flyDialog for Asharp
			     +Revisted the Gtk2 way

    copyright            : (C) 2004/2007 by mean
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
#include "Q_asharp.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidAsharp.h"
#include "math.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_asharpWindow::Ui_asharpWindow(QWidget *parent, asharp *param, ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
    ui.setupUi(this);
    lock=0;
    // Allocate space for green-ised video
    width=in->getInfo()->width;
    height=in->getInfo()->height;

    canvas=new ADM_QCanvas(ui.graphicsView,width,height);
    myCrop=new flyASharp(this,width, height,in,canvas,ui.horizontalSlider);

    memcpy(&(myCrop->param),param,sizeof(asharp));
    myCrop->_cookie=&ui;
    myCrop->addControl(ui.toolboxLayout, true);
    myCrop->setTabOrder();
    myCrop->upload();
    myCrop->sliderChanged();

    connect(ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.doubleSpinBox##x,SIGNAL(valueChanged(double)),this,SLOT(valueChanged(double))); \
                   connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSlider(int)));
    SPINNER(Threshold)
    SPINNER(Strength)
    SPINNER(Block)

#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged2(int)));
    CHKBOX(Strength);
    CHKBOX(Block);
    CHKBOX(HQBF);

    QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
    connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

    setModal(true);
}
void Ui_asharpWindow::sliderUpdate(int foo)
{
    myCrop->sliderChanged();
}
void Ui_asharpWindow::gather(asharp *param)
{
    myCrop->download();
    memcpy(param,&(myCrop->param),sizeof(asharp));
}
Ui_asharpWindow::~Ui_asharpWindow()
{
    if(myCrop) delete myCrop;
    myCrop=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_asharpWindow::valueChanged2( int f )
{
    valueChanged(0);
}
void Ui_asharpWindow::valueChanged( double f )
{
    if(lock) return;
    lock++;
    myCrop->download();
    myCrop->sameImage();
    lock--;
}
void Ui_asharpWindow::reset(void)
{
    if(lock) return;
    lock++;
    ASharp::reset(&myCrop->param);
    myCrop->upload();
    myCrop->sameImage();
    lock--;
}
void Ui_asharpWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myCrop->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myCrop->adjustCanvasPosition();
}

void Ui_asharpWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myCrop->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->doubleSpinBox##x
#define MYSLIDER(x) w->horizontalSlider##x
#define MYCHKBOX(x) w->checkBox##x

void Ui_asharpWindow::valueChangedSlider(int f)
{
    Ui_asharpDialog *w=(Ui_asharpDialog *)myCrop->_cookie;
    myCrop->blockChanges(true);

    MYSPIN(Threshold)->setValue((double)MYSLIDER(Threshold)->value() / 100.0);
    MYSPIN(Strength)->setValue((double)MYSLIDER(Strength)->value() / 100.0);
    MYSPIN(Block)->setValue((double)MYSLIDER(Block)->value() / 100.0);

    myCrop->blockChanges(false);
    valueChanged(0);
}

//************************
#define APPLY_TO_ALL(x) { \
    w->horizontalSliderThreshold->x; w->doubleSpinBoxThreshold->x; \
    w->horizontalSliderStrength->x; w->doubleSpinBoxStrength->x; \
    w->horizontalSliderBlock->x; w->doubleSpinBoxBlock->x; \
}
void flyASharp::blockChanges(bool block)
{
    Ui_asharpDialog *w=(Ui_asharpDialog *)_cookie;
    APPLY_TO_ALL(blockSignals(block));
}
#define ENABLE_NUM_INPUT(x,b) { \
    w->doubleSpinBox##x->setEnabled(b); \
    w->horizontalSlider##x->setEnabled(b); \
}
uint8_t flyASharp::upload(void)
{
    Ui_asharpDialog *w=(Ui_asharpDialog *)_cookie;
    blockChanges(true);

    MYSPIN(Threshold)->setValue(param.t);
    MYSLIDER(Threshold)->setValue(floor(param.t * 100.0));

    MYCHKBOX(Strength)->setChecked(param.d_enabled);
    ENABLE_NUM_INPUT(Strength, param.d_enabled);
    MYSPIN(Strength)->setValue(param.d);
    MYSLIDER(Strength)->setValue(floor(param.d * 100.0));

    MYCHKBOX(Block)->setChecked(param.b_enabled);
    ENABLE_NUM_INPUT(Block, param.b_enabled);
    MYSPIN(Block)->setValue(param.b);
    MYSLIDER(Block)->setValue(floor(param.b * 100.0));

    MYCHKBOX(HQBF)->setChecked(param.bf);
    blockChanges(false);
    sameImage();
    return 1;
}
uint8_t flyASharp::download(void)
{
    Ui_asharpDialog *w=(Ui_asharpDialog *)_cookie;
    param.t= MYSPIN(Threshold)->value();
    param.d= MYSPIN(Strength)->value();
    param.b= MYSPIN(Block)->value();
    param.bf=MYCHKBOX(HQBF)->isChecked();

    param.d_enabled=MYCHKBOX(Strength)->isChecked();
    param.b_enabled=MYCHKBOX(Block)->isChecked();

    blockChanges(true);

    MYSLIDER(Threshold)->setValue(floor(MYSPIN(Threshold)->value() * 100.0));
    MYSLIDER(Strength)->setValue(floor(MYSPIN(Strength)->value() * 100.0));
    MYSLIDER(Block)->setValue(floor(MYSPIN(Block)->value() * 100.0));

    ENABLE_NUM_INPUT(Strength, param.d_enabled)
    ENABLE_NUM_INPUT(Block, param.b_enabled)

    blockChanges(false);
    return 1;
}
void flyASharp::setTabOrder(void)
{
    Ui_asharpDialog *w=(Ui_asharpDialog *)_cookie;
    std::vector<QWidget *> controls;
    controls.push_back(MYSLIDER(Threshold));
    controls.push_back(MYSPIN(Threshold));
    controls.push_back(MYCHKBOX(Strength));
    controls.push_back(MYSLIDER(Strength));
    controls.push_back(MYSPIN(Strength));
    controls.push_back(MYCHKBOX(Block));
    controls.push_back(MYSLIDER(Block));
    controls.push_back(MYSPIN(Block));
    controls.push_back(MYCHKBOX(HQBF));

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
uint8_t DIA_getASharp(asharp *param, ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_asharpWindow dialog(qtLastRegisteredDialog(),param,in);
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


