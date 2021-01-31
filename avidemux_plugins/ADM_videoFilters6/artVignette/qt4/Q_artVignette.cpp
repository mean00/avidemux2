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

#include "Q_artVignette.h"
#include "ADM_vidArtVignette.h"
#include "ADM_toolkitQt.h"
#include "math.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_artVignetteWindow::Ui_artVignetteWindow(QWidget *parent, artVignette *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyArtVignette( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(artVignette));
        myFly->filterW = width;
        myFly->filterH = height;
        myFly->filterMask = new float[width*height];
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
        SPINNER(Aspect);
        SPINNER(Center);
        SPINNER(Soft);

        setModal(true);
}
void Ui_artVignetteWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artVignetteWindow::gather(artVignette *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artVignette));
}
Ui_artVignetteWindow::~Ui_artVignetteWindow()
{
    if(myFly) {
        if (myFly->filterMask) delete myFly->filterMask;
        delete myFly;
    }
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_artVignetteWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_artVignetteWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artVignetteWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************

uint8_t flyArtVignette::upload(void)
{
    Ui_artVignetteDialog *w=(Ui_artVignetteDialog *)_cookie;
    MYSPIN(Aspect)->setValue((int)round(param.aspect * 100.0));
    MYSPIN(Center)->setValue((int)round(param.center * 100.0));
    MYSPIN(Soft)->setValue((int)round(param.soft * 100.0));
    ADMVideoArtVignette::ArtVignetteCreateMask(filterMask, filterW, filterH, param.aspect, param.center, param.soft);
    return 1;
}
uint8_t flyArtVignette::download(void)
{
    Ui_artVignetteDialog *w=(Ui_artVignetteDialog *)_cookie;
    param.aspect=(float)MYSPIN(Aspect)->value() / 100.0;
    param.center=(float)MYSPIN(Center)->value() / 100.0;
    param.soft=(float)MYSPIN(Soft)->value() / 100.0;
    ADMVideoArtVignette::ArtVignetteCreateMask(filterMask, filterW, filterH, param.aspect, param.center, param.soft);
    return 1;
}

/**
    \fn     DIA_getCropParams
    \brief  Handle crop dialog
*/
uint8_t DIA_getArtVignette(artVignette *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artVignetteWindow dialog(qtLastRegisteredDialog(), param,in);

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


