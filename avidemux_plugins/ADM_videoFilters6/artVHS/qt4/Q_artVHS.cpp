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
#include "Q_artVHS.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidArtVHS.h"
#include "math.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_artVHSWindow::Ui_artVHSWindow(QWidget *parent, artVHS *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyArtVHS( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(artVHS));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y) ui.horizontalSlider##x->setScale(1,y,2); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(LumaBW,100)
        SPINNER(ChromaBW,100)
        SPINNER(UnSync,10)
        SPINNER(UnSyncFilter,100)

#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        CHKBOX(LumaNoDelay);
        CHKBOX(ChromaNoDelay);

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        ui.horizontalSliderLumaBW->setFocus();
        setModal(true);
}
void Ui_artVHSWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artVHSWindow::gather(artVHS *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artVHS));
}
Ui_artVHSWindow::~Ui_artVHSWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_artVHSWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_artVHSWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoArtVHS::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_artVHSWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artVHSWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyArtVHS::upload(void)
{
    Ui_artVHSDialog *w=(Ui_artVHSDialog *)_cookie;

    MYSPIN(LumaBW)->setValue((int)round(param.lumaBW*100.0));
    MYSPIN(ChromaBW)->setValue((int)round(param.chromaBW*100.0));
    MYSPIN(UnSync)->setValue((int)round(param.unSync*10.0));
    MYSPIN(UnSyncFilter)->setValue((int)round(param.unSyncFilter*100.0));
    MYCHECK(LumaNoDelay)->setChecked(param.lumaNoDelay);
    MYCHECK(ChromaNoDelay)->setChecked(param.chromaNoDelay);
    return 1;
}
uint8_t flyArtVHS::download(void)
{
    Ui_artVHSDialog *w=(Ui_artVHSDialog *)_cookie;
    param.lumaBW=((float)MYSPIN(LumaBW)->value()) / 100.0;
    param.chromaBW=((float)MYSPIN(ChromaBW)->value()) / 100.0;
    param.unSync=((float)MYSPIN(UnSync)->value()) / 10.0;
    param.unSyncFilter=((float)MYSPIN(UnSyncFilter)->value()) / 100.0;
    param.lumaNoDelay=MYCHECK(LumaNoDelay)->isChecked();
    param.chromaNoDelay=MYCHECK(ChromaNoDelay)->isChecked();
    return 1;
}
void flyArtVHS::setTabOrder(void)
{
    Ui_artVHSDialog *w=(Ui_artVHSDialog *)_cookie;
    std::vector<QWidget *> controls;
#define SPUSH(x) controls.push_back(MYSPIN(x));
#define TPUSH(x) controls.push_back(MYCHECK(x));
    SPUSH(LumaBW)
    TPUSH(LumaNoDelay)
    SPUSH(ChromaBW)
    TPUSH(ChromaNoDelay)
    SPUSH(UnSync)
    SPUSH(UnSyncFilter)

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
uint8_t DIA_getArtVHS(artVHS *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artVHSWindow dialog(qtLastRegisteredDialog(), param,in);

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


