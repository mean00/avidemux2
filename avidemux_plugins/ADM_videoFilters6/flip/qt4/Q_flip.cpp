/***************************************************************************
                          Q_flip.cpp  -  description
                          --------------------------

    GUI for flip

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

#include "Q_flip.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidFlip.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_flipWindow::Ui_flipWindow(QWidget *parent, flip *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyFlip( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(flip));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect(ui.comboBoxFlipdir, SIGNAL(currentIndexChanged(int)), this, SLOT(flipdirChanged(int)));

        setModal(true);
}
void Ui_flipWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_flipWindow::gather(flip *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(flip));
}
Ui_flipWindow::~Ui_flipWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_flipWindow::flipdirChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_flipWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_flipWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYCOMBOX(x) w->comboBox##x
//************************
uint8_t flyFlip::upload(void)
{
    Ui_flipDialog *w=(Ui_flipDialog *)_cookie;
    MYCOMBOX(Flipdir)->setCurrentIndex(param.flipdir);
    return 1;
}
uint8_t flyFlip::download(void)
{
    Ui_flipDialog *w=(Ui_flipDialog *)_cookie;
    param.flipdir=MYCOMBOX(Flipdir)->currentIndex();
    return 1;
}
void flyFlip::setTabOrder(void)
{
    Ui_flipDialog *w=(Ui_flipDialog *)_cookie;
    std::vector<QWidget *> controls;

    controls.push_back(MYCOMBOX(Flipdir));

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
uint8_t DIA_getFlip(flip *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_flipWindow dialog(qtLastRegisteredDialog(), param,in);

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


