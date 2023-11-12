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
#include <cmath>
#include "Q_flat360.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidFlat360.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_flat360Window::Ui_flat360Window(QWidget *parent, flat360 *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyFlat360( this,width, height,in,canvas,ui.horizontalSlider);
        ADMVideoFlat360::Flat360CreateBuffers(width,height, &(myFly->buffers));
        memcpy(&(myFly->param),param,sizeof(flat360));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, ControlOption::PeekOriginalBtn);
        myFly->setTabOrder();
        myFly->upload();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \
        connect( ui.doubleSpinBox##x,SIGNAL(valueChanged(double)),this,SLOT(valueChangedSpinBox(double)));
        SPINNER(Yaw);
        SPINNER(Pitch);
        SPINNER(Roll);
        SPINNER(Fov);
        SPINNER(Distortion);
#undef SPINNER
        connect(ui.comboBoxMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));
        connect(ui.comboBoxAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));
        connect(ui.spinBoxPad,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \

        QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        QT6_CRASH_WORKAROUND(flat360Window)

        setModal(true);
}
void Ui_flat360Window::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_flat360Window::gather(flat360 *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(flat360));
}
Ui_flat360Window::~Ui_flat360Window()
{
    if(myFly) {
        ADMVideoFlat360::Flat360DestroyBuffers(&(myFly->buffers));
        delete myFly;
    }
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}

#define COPY_VALUE_TO_SPINBOX(x) \
        ui.doubleSpinBox##x->blockSignals(true); \
        ui.doubleSpinBox##x->setValue((float)ui.horizontalSlider##x->value() / 10.0); \
        ui.doubleSpinBox##x->blockSignals(false);
void Ui_flat360Window::valueChanged( int f )
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SPINBOX(Yaw);
    COPY_VALUE_TO_SPINBOX(Pitch);
    COPY_VALUE_TO_SPINBOX(Roll);
    COPY_VALUE_TO_SPINBOX(Fov);
    COPY_VALUE_TO_SPINBOX(Distortion);
    myFly->download();
    myFly->sameImage();
    lock--;
}

#define COPY_VALUE_TO_SLIDER(x) \
        ui.horizontalSlider##x->blockSignals(true); \
        ui.horizontalSlider##x->setValue((int)std::round(ui.doubleSpinBox##x->value() * 10.0)); \
        ui.horizontalSlider##x->blockSignals(false);
void Ui_flat360Window::valueChangedSpinBox(double foo)
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SLIDER(Yaw);
    COPY_VALUE_TO_SLIDER(Pitch);
    COPY_VALUE_TO_SLIDER(Roll);
    COPY_VALUE_TO_SLIDER(Fov);
    COPY_VALUE_TO_SLIDER(Distortion);
    myFly->download();
    myFly->sameImage();
    lock--;
}
/**
 * \fn reset
 */
void Ui_flat360Window::reset( bool f )
{
    myFly->param.yaw=0;
    myFly->param.pitch=0;
    myFly->param.roll=0;
    myFly->param.fov=90;
    myFly->param.distortion=0;
    lock++;
    myFly->upload();
    myFly->sameImage();
    lock--;
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSLIDER(x) w->horizontalSlider##x
#define MYSPIN(x) w->spinBox##x
#define MYDBLSPIN(x) w->doubleSpinBox##x
#define UPLOADSPINSLIDER(x, value) \
        w->doubleSpinBox##x->blockSignals(true); \
        w->doubleSpinBox##x->setValue(value); \
        w->doubleSpinBox##x->blockSignals(false); \
        w->horizontalSlider##x->blockSignals(true); \
        w->horizontalSlider##x->setValue((int)std::round(value * 10.0)); \
        w->horizontalSlider##x->blockSignals(false);
//************************
uint8_t flyFlat360::upload()
{
    Ui_flat360Dialog *w=(Ui_flat360Dialog *)_cookie;
    MYCOMBOX(Method)->setCurrentIndex(param.method);
    MYCOMBOX(Algo)->setCurrentIndex(param.algo);
    MYSPIN(Pad)->setValue(param.pad);
    MYSPIN(Pad)->setEnabled(param.method == 2);
    UPLOADSPINSLIDER(Yaw, param.yaw);
    UPLOADSPINSLIDER(Pitch, param.pitch);
    UPLOADSPINSLIDER(Roll, param.roll);
    UPLOADSPINSLIDER(Fov, param.fov);
    UPLOADSPINSLIDER(Distortion, param.distortion);
    return 1;
}
uint8_t flyFlat360::download(void)
{
    int reject=0;
    Ui_flat360Dialog *w=(Ui_flat360Dialog *)_cookie;
    param.method=MYCOMBOX(Method)->currentIndex();
    param.algo=MYCOMBOX(Algo)->currentIndex();
    param.pad=MYSPIN(Pad)->value();

#define SPIN_GET(x,y) x=w->doubleSpinBox##y->value();
    SPIN_GET(param.yaw, Yaw);
    SPIN_GET(param.pitch, Pitch);
    SPIN_GET(param.roll, Roll);
    SPIN_GET(param.fov, Fov);
    SPIN_GET(param.distortion, Distortion);

    upload();
    return 1;
}
void flyFlat360::setTabOrder(void)
{
    Ui_flat360Dialog *w=(Ui_flat360Dialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHSLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_DBLSPIN(x) controls.push_back(MYDBLSPIN(x));
#define PUSH_TOG(x) controls.push_back(w->checkBox##x);
    PUSHCOMBOX(Method)
    PUSH_SPIN(Pad);
    PUSHCOMBOX(Algo)
    
    PUSH_DBLSPIN(Yaw)
    PUSH_DBLSPIN(Pitch)
    PUSH_DBLSPIN(Roll)
    PUSH_DBLSPIN(Fov)
    PUSH_DBLSPIN(Distortion)
    PUSHSLIDER(Yaw)
    PUSHSLIDER(Pitch)
    PUSHSLIDER(Roll)
    PUSHSLIDER(Fov)
    PUSH_DBLSPIN(Distortion)

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
uint8_t DIA_getFlat360(flat360 *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_flat360Window dialog(qtLastRegisteredDialog(), param,in);

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


