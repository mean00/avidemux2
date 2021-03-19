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

#include "Q_artCartoon.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidArtCartoon.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_artCartoonWindow::Ui_artCartoonWindow(QWidget *parent, artCartoon *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        myFly=new flyArtCartoon(this,width,height,in,canvas,ui.horizontalSlider);

        memcpy(&(myFly->param),param,sizeof(artCartoon));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(Threshold,100,2)
        SPINNER(Scatter,1,0)
        SPINNER(Color,1,0)

        setModal(true);
}
void Ui_artCartoonWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artCartoonWindow::gather(artCartoon *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artCartoon));
}
Ui_artCartoonWindow::~Ui_artCartoonWindow()
{
    if(myFly) delete myFly;
    myFly=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_artCartoonWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_artCartoonWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artCartoonWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyArtCartoon::upload(void)
{
    Ui_artCartoonDialog *w=(Ui_artCartoonDialog *)_cookie;
    MYSPIN(Threshold)->setValue((int)round(param.threshold*100.0));
    MYSPIN(Scatter)->setValue(param.scatter);
    MYSPIN(Color)->setValue(param.color);
    return 1;
}

uint8_t flyArtCartoon::download(void)
{
    Ui_artCartoonDialog *w=(Ui_artCartoonDialog *)_cookie;
    param.threshold=((float)MYSPIN(Threshold)->value()) / 100.0;
    param.scatter=MYSPIN(Scatter)->value();
    param.color=MYSPIN(Color)->value();
    return 1;
}

void flyArtCartoon::setTabOrder(void)
{
    Ui_artCartoonDialog *w=(Ui_artCartoonDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHME(x) controls.push_back(MYSPIN(x));
    PUSHME(Threshold)
    PUSHME(Scatter)
    PUSHME(Color)

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
uint8_t DIA_getArtCartoon(artCartoon *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artCartoonWindow dialog(qtLastRegisteredDialog(), param,in);

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


