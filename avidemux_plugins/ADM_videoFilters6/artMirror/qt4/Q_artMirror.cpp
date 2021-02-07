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

#include "Q_artMirror.h"
#include "ADM_toolkitQt.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_artMirrorWindow::Ui_artMirrorWindow(QWidget *parent, artMirror *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyArtMirror( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(artMirror));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect(ui.comboBoxMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(Displacement,100,2)

        setModal(true);
}
void Ui_artMirrorWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artMirrorWindow::gather(artMirror *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artMirror));
}
Ui_artMirrorWindow::~Ui_artMirrorWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_artMirrorWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_artMirrorWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artMirrorWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyArtMirror::upload(void)
{
    Ui_artMirrorDialog *w=(Ui_artMirrorDialog *)_cookie;
    MYCOMBOX(Method)->setCurrentIndex(param.method);
    MYSPIN(Displacement)->setValue((int)round(param.displacement*100.0));
    return 1;
}
uint8_t flyArtMirror::download(void)
{
    Ui_artMirrorDialog *w=(Ui_artMirrorDialog *)_cookie;
    param.method=MYCOMBOX(Method)->currentIndex();
    param.displacement=((float)MYSPIN(Displacement)->value()) / 100.0;
    return 1;
}

/**
    \fn     DIA_getCropParams
    \brief  Handle crop dialog
*/
uint8_t DIA_getArtMirror(artMirror *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artMirrorWindow dialog(qtLastRegisteredDialog(), param,in);

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


