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
#include "Q_quadTrans.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidQuadTrans.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_quadTransWindow::Ui_quadTransWindow(QWidget *parent, quadTrans *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyQuadTrans( this,width, height,in,canvas,ui.horizontalSlider);
        ADMVideoQuadTrans::QuadTransCreateBuffers(width,height, &(myFly->buffers));
        memcpy(&(myFly->param),param,sizeof(quadTrans));
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.doubleSpinBox##x,SIGNAL(valueChanged(double)),this,SLOT(valueChangedSpinBox(double))); ui.doubleSpinBox##x->setKeyboardTracking(true);
        SPINNER(Dx1);
        SPINNER(Dy1);
        SPINNER(Dx2);
        SPINNER(Dy2);
        SPINNER(Dx3);
        SPINNER(Dy3);
        SPINNER(Dx4);
        SPINNER(Dy4);
        SPINNER(Zoom);
#undef SPINNER
        connect(ui.comboBoxAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));

        QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        setModal(true);
}
void Ui_quadTransWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_quadTransWindow::gather(quadTrans *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(quadTrans));
}
Ui_quadTransWindow::~Ui_quadTransWindow()
{
    if(myFly) {
        ADMVideoQuadTrans::QuadTransDestroyBuffers(&(myFly->buffers));
        delete myFly;
    }
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}

void Ui_quadTransWindow::valueChanged( int f )
{
    valueChangedSpinBox(0.0);
}

void Ui_quadTransWindow::valueChangedSpinBox(double foo)
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
void Ui_quadTransWindow::reset( bool f )
{
    myFly->param.dx1=0;
    myFly->param.dy1=0;
    myFly->param.dx2=0;
    myFly->param.dy2=0;
    myFly->param.dx3=0;
    myFly->param.dy3=0;
    myFly->param.dx4=0;
    myFly->param.dy4=0;
    myFly->param.zoom=1;
    myFly->param.algo=0;
    lock++;
    myFly->upload();
    myFly->sameImage();
    lock--;
}
void Ui_quadTransWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

void Ui_quadTransWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSPIN(x) w->doubleSpinBox##x
#define UPLOADSPIN(x, value) \
        w->doubleSpinBox##x->blockSignals(true); \
        w->doubleSpinBox##x->setValue(value); \
        w->doubleSpinBox##x->blockSignals(false);
//************************
uint8_t flyQuadTrans::upload()
{
    Ui_quadTransDialog *w=(Ui_quadTransDialog *)_cookie;
    MYCOMBOX(Algo)->setCurrentIndex(param.algo);
    UPLOADSPIN(Dx1, param.dx1);
    UPLOADSPIN(Dy1, param.dy1);
    UPLOADSPIN(Dx2, param.dx2);
    UPLOADSPIN(Dy2, param.dy2);
    UPLOADSPIN(Dx3, param.dx3);
    UPLOADSPIN(Dy3, param.dy3);
    UPLOADSPIN(Dx4, param.dx4);
    UPLOADSPIN(Dy4, param.dy4);
    UPLOADSPIN(Zoom, param.zoom);

    return 1;
}
uint8_t flyQuadTrans::download(void)
{
    int reject=0;
    Ui_quadTransDialog *w=(Ui_quadTransDialog *)_cookie;
    param.algo=MYCOMBOX(Algo)->currentIndex();

#define SPIN_GET(x,y) x=w->doubleSpinBox##y->value();
    SPIN_GET(param.dx1, Dx1);
    SPIN_GET(param.dy1, Dy1);
    SPIN_GET(param.dx2, Dx2);
    SPIN_GET(param.dy2, Dy2);
    SPIN_GET(param.dx3, Dx3);
    SPIN_GET(param.dy3, Dy3);
    SPIN_GET(param.dx4, Dx4);
    SPIN_GET(param.dy4, Dy4);
    SPIN_GET(param.zoom, Zoom);

    upload();
    return 1;
}
void flyQuadTrans::setTabOrder(void)
{
    Ui_quadTransDialog *w=(Ui_quadTransDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHSLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(w->checkBox##x);
    PUSHCOMBOX(Algo)
    PUSH_SPIN(Zoom)
    
    PUSH_SPIN(Dx1)
    PUSH_SPIN(Dy1)
    PUSH_SPIN(Dx2)
    PUSH_SPIN(Dy2)
    PUSH_SPIN(Dx3)
    PUSH_SPIN(Dy3)
    PUSH_SPIN(Dx4)
    PUSH_SPIN(Dy4)

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
uint8_t DIA_getQuadTrans(quadTrans *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_quadTransWindow dialog(qtLastRegisteredDialog(), param,in);

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


