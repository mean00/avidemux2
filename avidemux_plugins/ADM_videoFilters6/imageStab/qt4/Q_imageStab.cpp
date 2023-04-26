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
#include <QPalette>
#include "ADM_default.h"
#include "Q_imageStab.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidImageStab.h"
#include <cmath>

//
//	Video is in YV12 Colorspace
//
//
Ui_imageStabWindow::Ui_imageStabWindow(QWidget *parent, imageStab *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyImageStab( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myFly->param),param,sizeof(imageStab));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->setTabOrder();
        myFly->upload();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x,y,z) ui.horizontalSlider##x->setScale(1,y,z); \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
        SPINNER(Smoothing,100,2)
        SPINNER(Gravity,100,2)
        SPINNER(Zoom,100,2)
        SPINNER(SceneThreshold,100,2)
        
        connect(ui.comboBoxAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));
        connect(ui.comboBoxMotionEstimation, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));

#define CHKBOX(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
        CHKBOX(AutoGravity);
        CHKBOX(Pad);

        QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        QT6_CRASH_WORKAROUND(imageStabWindow)

        setModal(true);
}
void Ui_imageStabWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_imageStabWindow::gather(imageStab *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(imageStab));
}
Ui_imageStabWindow::~Ui_imageStabWindow()
{
    if(myFly) delete myFly;
    myFly=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
}

void Ui_imageStabWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}
/**
 * \fn reset
 */
void Ui_imageStabWindow::reset( bool f )
{
    myFly->param.smoothing=0.5;
    myFly->param.gravity=0.5;
    myFly->param.autoGravity=true;
    myFly->param.sceneThreshold=0.5;
    myFly->param.zoom=1;
    myFly->param.algo = 0;
    myFly->param.motionEstimation = 0;
    myFly->param.pad=false;
    lock++;
    myFly->upload();
    myFly->sameImage();
    lock--;
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSLIDER(x) w->horizontalSlider##x
#define MYCHKBOX(x) w->checkBox##x
#define UPLOADSLIDER(x, value) \
        w->horizontalSlider##x->blockSignals(true); \
        w->horizontalSlider##x->setValue(value); \
        w->horizontalSlider##x->blockSignals(false);
//************************
uint8_t flyImageStab::upload()
{
    Ui_imageStabDialog *w=(Ui_imageStabDialog *)_cookie;
    MYCOMBOX(Algo)->setCurrentIndex(param.algo);
    MYCOMBOX(MotionEstimation)->setCurrentIndex(param.motionEstimation);
    UPLOADSLIDER(Smoothing, (int)round(param.smoothing*100.0));
    UPLOADSLIDER(Gravity, (int)round(param.gravity*100.0));
    UPLOADSLIDER(Zoom, (int)round(param.zoom*100.0));
    UPLOADSLIDER(SceneThreshold, (int)round(param.sceneThreshold*100.0));
    MYCHKBOX(AutoGravity)->setChecked(param.autoGravity);
    MYSLIDER(Gravity)->setEnabled(!param.autoGravity);
    MYCHKBOX(Pad)->setChecked(param.pad);

    return 1;
}
uint8_t flyImageStab::download(void)
{
    int reject=0;
    Ui_imageStabDialog *w=(Ui_imageStabDialog *)_cookie;
    param.algo=MYCOMBOX(Algo)->currentIndex();
    param.motionEstimation=MYCOMBOX(MotionEstimation)->currentIndex();
    param.smoothing = ((float)MYSLIDER(Smoothing)->value()) / 100.0;
    param.gravity = ((float)MYSLIDER(Gravity)->value()) / 100.0;
    param.zoom = ((float)MYSLIDER(Zoom)->value()) / 100.0;
    param.sceneThreshold = ((float)MYSLIDER(SceneThreshold)->value()) / 100.0;
    param.autoGravity = MYCHKBOX(AutoGravity)->isChecked();
    MYSLIDER(Gravity)->setEnabled(!param.autoGravity);
    param.pad = MYCHKBOX(Pad)->isChecked();

    upload();
    return 1;
}
void flyImageStab::setTabOrder(void)
{
    Ui_imageStabDialog *w=(Ui_imageStabDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHSLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSHCHKBOX(x) controls.push_back(MYCHKBOX(x));
    PUSHSLIDER(Smoothing)
    PUSHSLIDER(Gravity)
    PUSHCOMBOX(Algo)
    PUSHCOMBOX(MotionEstimation)
    PUSHCHKBOX(Pad)
    PUSHCHKBOX(AutoGravity)
    PUSHSLIDER(Zoom)
    PUSHSLIDER(SceneThreshold)

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
void flyImageStab::refreshIndicator(void)
{
    Ui_imageStabDialog *w=(Ui_imageStabDialog *)_cookie;
    QPalette indctrPalette(w->lineEditNewScene->palette());
    QColor color;

    color.setRgb(0,(newScene ? 255:64),0,255);
    indctrPalette.setColor(QPalette::Window,color);
    indctrPalette.setColor(QPalette::Base,color);
    indctrPalette.setColor(QPalette::AlternateBase,color);

    w->lineEditNewScene->setPalette(indctrPalette);
    w->progressBarScene->setValue(round(sceneDiff*100.0));
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_getImageStab(imageStab *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_imageStabWindow dialog(qtLastRegisteredDialog(), param,in);

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


