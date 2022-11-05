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
#include "Q_aiEnhance.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidAiEnhance.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_aiEnhanceWindow::Ui_aiEnhanceWindow(QWidget *parent, aiEnhance *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width*2;
        height=in->getInfo()->height*2;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        peekOriginalBtn=new QPushButton();
        peekOriginalBtn->setObjectName(QString("peekOriginalBtn"));
        peekOriginalBtn->setAutoRepeat(false);
        peekOriginalBtn->setText(QT_TRANSLATE_NOOP("aiEnhance", "Peek Original"));
        
        myFly=new flyAiEnhance( this,width, height,in,canvas,ui.horizontalSlider);
        ADMVideoAiEnhance::AiEnhanceInitializeBuffers(in->getInfo()->width,in->getInfo()->height, &(myFly->buffers));
        memcpy(&(myFly->param),param,sizeof(aiEnhance));
        myFly->showOriginal = false;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, ControlOption::UserWidgetAfterPeekBtn, peekOriginalBtn);    //use local generation for Peek original functionality
        myFly->setTabOrder();
        myFly->upload();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect(ui.comboBoxAlgo, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));

        connect( peekOriginalBtn,SIGNAL(pressed()),this,SLOT(peekOriginalPressed()));
        connect( peekOriginalBtn,SIGNAL(released()),this,SLOT(peekOriginalReleased()));        
        
        setModal(true);
}
void Ui_aiEnhanceWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_aiEnhanceWindow::gather(aiEnhance *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(aiEnhance));
}

void Ui_aiEnhanceWindow::peekOriginalPressed(void)
{
    myFly->showOriginal = true;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;    
}

void Ui_aiEnhanceWindow::peekOriginalReleased(void)
{
    myFly->showOriginal = false;
    if(lock) return;
    lock++;
    myFly->sameImage();
    lock--;    
}

Ui_aiEnhanceWindow::~Ui_aiEnhanceWindow()
{
    if(myFly) {
        ADMVideoAiEnhance::AiEnhanceDestroyBuffers(&(myFly->buffers));
        delete myFly;
    }
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}

void Ui_aiEnhanceWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myFly->download();
    myFly->sameImage();
    lock--;
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSPIN(x) w->doubleSpinBox##x
#define UPLOADSPIN(x, value) \
        w->doubleSpinBox##x->blockSignals(true); \
        w->doubleSpinBox##x->setValue(value); \
        w->doubleSpinBox##x->blockSignals(false);
//************************
uint8_t flyAiEnhance::upload()
{
    Ui_aiEnhanceDialog *w=(Ui_aiEnhanceDialog *)_cookie;
    MYCOMBOX(Algo)->setCurrentIndex(param.algo);
    
    QString wt=QString(QT_TRANSLATE_NOOP("aiEnhance", "Warning: the preview scaled back to x2"));
    
    if (ADMVideoAiEnhance::getScaling(param.algo) > 2)
        w->labelWarning->setText(wt);
    else
        w->labelWarning->clear();

    return 1;
}
uint8_t flyAiEnhance::download(void)
{
    int reject=0;
    Ui_aiEnhanceDialog *w=(Ui_aiEnhanceDialog *)_cookie;
    param.algo=MYCOMBOX(Algo)->currentIndex();

    upload();
    return 1;
}
void flyAiEnhance::setTabOrder(void)
{
    Ui_aiEnhanceDialog *w=(Ui_aiEnhanceDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHSLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(w->checkBox##x);
    PUSHCOMBOX(Algo)

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
uint8_t DIA_getAiEnhance(aiEnhance *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_aiEnhanceWindow dialog(qtLastRegisteredDialog(), param,in);

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


