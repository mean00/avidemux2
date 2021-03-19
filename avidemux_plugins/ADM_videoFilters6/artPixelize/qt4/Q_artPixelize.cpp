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

#include "Q_artPixelize.h"
#include "ADM_toolkitQt.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_artPixelizeWindow::Ui_artPixelizeWindow(QWidget *parent, artPixelize *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyArtPixelize( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(artPixelize));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
        SPINNER(PW);
        SPINNER(PH);

        ui.spinBoxPW->setFocus();
        setModal(true);
}
void Ui_artPixelizeWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artPixelizeWindow::gather(artPixelize *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artPixelize));
}
Ui_artPixelizeWindow::~Ui_artPixelizeWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_artPixelizeWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

void Ui_artPixelizeWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artPixelizeWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->spinBox##x
//************************
uint8_t flyArtPixelize::upload(void)
{
    Ui_artPixelizeDialog *w=(Ui_artPixelizeDialog *)_cookie;

    MYSPIN(PW)->setValue((int)param.pw);
    MYSPIN(PH)->setValue((int)param.ph);
    return 1;
}
uint8_t flyArtPixelize::download(void)
{
    Ui_artPixelizeDialog *w=(Ui_artPixelizeDialog *)_cookie;
    param.pw=MYSPIN(PW)->value();
    param.ph=MYSPIN(PH)->value();
    return 1;
}
void flyArtPixelize::setTabOrder(void)
{
    Ui_artPixelizeDialog *w=(Ui_artPixelizeDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHME(x) controls.push_back(MYSPIN(x));
    PUSHME(PW)
    PUSHME(PH)

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
uint8_t DIA_getArtPixelize(artPixelize *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artPixelizeWindow dialog(qtLastRegisteredDialog(), param,in);

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


