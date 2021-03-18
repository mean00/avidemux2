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
#include "Q_artCharcoal.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidArtCharcoal.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_artCharcoalWindow::Ui_artCharcoalWindow(QWidget *parent, artCharcoal *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        myFly=new flyArtCharcoal( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(artCharcoal));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(ScatterX,1,0)
        SPINNER(ScatterY,1,0)
        SPINNER(Intensity,100,2)
        SPINNER(Color,100,2)

#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        CHKBOX(Invert);

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
}
void Ui_artCharcoalWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_artCharcoalWindow::gather(artCharcoal *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(artCharcoal));
}
Ui_artCharcoalWindow::~Ui_artCharcoalWindow()
{
    if(myFly) delete myFly;
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
void Ui_artCharcoalWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
void Ui_artCharcoalWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoArtCharcoal::reset(&myFly->param);
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_artCharcoalWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_artCharcoalWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyArtCharcoal::upload(void)
{
    Ui_artCharcoalDialog *w=(Ui_artCharcoalDialog *)_cookie;

    MYSPIN(ScatterX)->setValue((int)param.scatterX);
    MYSPIN(ScatterY)->setValue((int)param.scatterY);
    MYSPIN(Intensity)->setValue((int)round(param.intensity*100.0));
    MYSPIN(Color)->setValue((int)round(param.color*100.0));
    MYCHECK(Invert)->setChecked(param.invert);
    return 1;
}
uint8_t flyArtCharcoal::download(void)
{
    Ui_artCharcoalDialog *w=(Ui_artCharcoalDialog *)_cookie;
    param.scatterX=(int)MYSPIN(ScatterX)->value();
    param.scatterY=(int)MYSPIN(ScatterY)->value();
    param.intensity=((float)MYSPIN(Intensity)->value()) / 100.0;
    param.color=((float)MYSPIN(Color)->value()) / 100.0;
    param.invert=MYCHECK(Invert)->isChecked();
    return 1;
}
void flyArtCharcoal::setTabOrder(void)
{
    Ui_artCharcoalDialog *w=(Ui_artCharcoalDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(MYCHECK(x));
    PUSH_SPIN(ScatterX)
    PUSH_SPIN(ScatterY)
    PUSH_SPIN(Intensity)
    PUSH_SPIN(Color)
    PUSH_TOG(Invert)

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
uint8_t DIA_getArtCharcoal(artCharcoal *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_artCharcoalWindow dialog(qtLastRegisteredDialog(), param,in);

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


