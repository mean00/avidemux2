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
#include "Q_artDynThreshold.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidArtDynThreshold.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_artDynThresholdWindow::Ui_artDynThresholdWindow(QWidget *parent, artDynThreshold *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyArtDynThreshold( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(artDynThreshold));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(Levels,1,0)
        SPINNER(Offset,100,2)

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_artDynThresholdWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artDynThresholdWindow::gather(artDynThreshold *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artDynThreshold));
}
Ui_artDynThresholdWindow::~Ui_artDynThresholdWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_artDynThresholdWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_artDynThresholdWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoArtDynThreshold::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_artDynThresholdWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artDynThresholdWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyArtDynThreshold::upload(void)
{
    Ui_artDynThresholdDialog *w=(Ui_artDynThresholdDialog *)_cookie;

    MYSPIN(Levels)->setValue((int)param.levels);
    MYSPIN(Offset)->setValue((int)round(param.offset*100.0));
    return 1;
}
uint8_t flyArtDynThreshold::download(void)
{
    Ui_artDynThresholdDialog *w=(Ui_artDynThresholdDialog *)_cookie;
    param.levels=(int)MYSPIN(Levels)->value();
    param.offset=((float)MYSPIN(Offset)->value() / 100.0);
    return 1;
}
void flyArtDynThreshold::setTabOrder(void)
{
    Ui_artDynThresholdDialog *w=(Ui_artDynThresholdDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    PUSH_SPIN(Levels)
    PUSH_SPIN(Offset)

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
uint8_t DIA_getArtDynThreshold(artDynThreshold *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artDynThresholdWindow dialog(qtLastRegisteredDialog(), param,in);

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


