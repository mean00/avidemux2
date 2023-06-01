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
#include "Q_grain.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidGrain.h"
#include "math.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_grainWindow::Ui_grainWindow(QWidget *parent, grain *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyGrain( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(grain));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, ControlOption::PeekOriginalBtn);
        myFly->setTabOrder();
        myFly->upload();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y) ui.horizontalSlider##x->setScale(1,y,2); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(Noise,100)

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        QT6_CRASH_WORKAROUND(grainWindow)

        ui.horizontalSliderNoise->setFocus();
        setModal(true);
}
void Ui_grainWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_grainWindow::gather(grain *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(grain));
}
Ui_grainWindow::~Ui_grainWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_grainWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_grainWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoGrain::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyGrain::upload(void)
{
    Ui_grainDialog *w=(Ui_grainDialog *)_cookie;

    MYSPIN(Noise)->setValue((int)round(param.noise*100.0));
    return 1;
}
uint8_t flyGrain::download(void)
{
    Ui_grainDialog *w=(Ui_grainDialog *)_cookie;
    param.noise=((float)MYSPIN(Noise)->value()) / 100.0;
    return 1;
}
void flyGrain::setTabOrder(void)
{
    Ui_grainDialog *w=(Ui_grainDialog *)_cookie;
    std::vector<QWidget *> controls;
#define SPUSH(x) controls.push_back(MYSPIN(x));
    SPUSH(Noise)

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
uint8_t DIA_getGrain(grain *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_grainWindow dialog(qtLastRegisteredDialog(), param,in);

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


