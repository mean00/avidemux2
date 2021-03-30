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
#include "Q_artGrid.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidArtGrid.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_artGridWindow::Ui_artGridWindow(QWidget *parent, artGrid *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyArtGrid( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(artGrid));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \
        connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSpinBox(int)));
        SPINNER(Size)

#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        CHKBOX(Roll);

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_artGridWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artGridWindow::gather(artGrid *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artGrid));
}
Ui_artGridWindow::~Ui_artGridWindow()
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
void Ui_artGridWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SPINBOX(Size);
    myFly->download();
    myFly->blacken();
    myFly->sameImage();
    lock--;
}
#define COPY_VALUE_TO_SLIDER(x) \
        ui.horizontalSlider##x->blockSignals(true); \
        ui.horizontalSlider##x->setValue(ui.spinBox##x->value()); \
        ui.horizontalSlider##x->blockSignals(false);
void Ui_artGridWindow::valueChangedSpinBox(int foo)
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SLIDER(Size);
    myFly->download();
    myFly->blacken();
    myFly->sameImage();
    lock--;
}
void Ui_artGridWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoArtGrid::reset(&myFly->param);
    myFly->upload();
    myFly->blacken();
    myFly->sameImage();
    lock--;
}
void Ui_artGridWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artGridWindow::showEvent(QShowEvent *event)
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
uint8_t flyArtGrid::upload(void)
{
    Ui_artGridDialog *w=(Ui_artGridDialog *)_cookie;

    MYSLIDER(Size)->setValue((int)param.size);
    UPLOADSPIN(Size, param.size);
    MYCHECK(Roll)->setChecked(param.roll);
    return 1;
}
uint8_t flyArtGrid::download(void)
{
    Ui_artGridDialog *w=(Ui_artGridDialog *)_cookie;
    param.size=(int)MYSLIDER(Size)->value();
    param.roll=MYCHECK(Roll)->isChecked();
    return 1;
}
void flyArtGrid::setTabOrder(void)
{
    Ui_artGridDialog *w=(Ui_artGridDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    PUSH_SLIDER(Size)
    PUSH_SPIN(Size)
    PUSH_TOG(Roll)

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
uint8_t DIA_getArtGrid(artGrid *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artGridWindow dialog(qtLastRegisteredDialog(), param,in);

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


