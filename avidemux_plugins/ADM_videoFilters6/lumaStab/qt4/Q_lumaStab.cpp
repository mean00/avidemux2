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
#include "Q_lumaStab.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidLumaStab.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_lumaStabWindow::Ui_lumaStabWindow(QWidget *parent, lumaStab *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyLumaStab( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(lumaStab));
        myFly->indctr=ui.lineEditNewScene;
        myFly->indctrPB=ui.progressBarScene;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(FilterLength,1,0)
        SPINNER(SceneThreshold,100,2)
        SPINNER(CBRatio,100,2)

#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        CHKBOX(Chroma);


        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_lumaStabWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_lumaStabWindow::gather(lumaStab *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(lumaStab));
}
Ui_lumaStabWindow::~Ui_lumaStabWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_lumaStabWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    //myFly->sameImage();
    lock--;
}
void Ui_lumaStabWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoLumaStab::reset(&myFly->param);
    myFly->upload();
    //myFly->sameImage();
    lock--;
}

void Ui_lumaStabWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_lumaStabWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyLumaStab::upload(void)
{
    Ui_lumaStabDialog *w=(Ui_lumaStabDialog *)_cookie;
    MYSPIN(FilterLength)->setValue((int)param.filterLength);
    MYSPIN(CBRatio)->setValue((int)round(param.cbratio*100.0));
    MYSPIN(SceneThreshold)->setValue((int)round(param.sceneThreshold*100.0));
    MYCHECK(Chroma)->setChecked(param.chroma);
    return 1;
}
uint8_t flyLumaStab::download(void)
{
    Ui_lumaStabDialog *w=(Ui_lumaStabDialog *)_cookie;
    param.filterLength=MYSPIN(FilterLength)->value();
    param.cbratio=((float)MYSPIN(CBRatio)->value()) / 100.0;
    param.sceneThreshold=((float)MYSPIN(SceneThreshold)->value()) / 100.0;
    param.chroma=MYCHECK(Chroma)->isChecked();
    return 1;
}
void flyLumaStab::setTabOrder(void)
{
    Ui_lumaStabDialog *w=(Ui_lumaStabDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    PUSH_SPIN(FilterLength)
    PUSH_SPIN(CBRatio)
    PUSH_SPIN(SceneThreshold)
    PUSH_TOG(Chroma)

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
uint8_t DIA_getLumaStab(lumaStab *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_lumaStabWindow dialog(qtLastRegisteredDialog(), param,in);

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


