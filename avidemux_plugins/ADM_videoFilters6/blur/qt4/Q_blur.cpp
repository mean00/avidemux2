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
#include "Q_blur.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidBlur.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_blurWindow::Ui_blurWindow(QWidget *parent, blur *param,ADM_coreVideoFilter *in) : QDialog(parent)
{
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyBlur( this,width, height,in,canvas,ui.horizontalSlider);
        ADMVideoBlur::BlurCreateBuffers(width,height, &(myFly->rgbBufStride), &(myFly->rgbBufRaw), &(myFly->rgbBufImage), &(myFly->convertYuvToRgb), &(myFly->convertRgbToYuv));
        memcpy(&(myFly->param),param,sizeof(blur));
        myFly->left=param->left;
        myFly->right=param->right;
        myFly->top=param->top;
        myFly->bottom=param->bottom;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout, true);
        myFly->setTabOrder();
        myFly->upload();
        myFly->sliderChanged();

        myFly->rubber->nestedIgnore=1;
        myFly->rubber_is_hidden=param->rubber_is_hidden;
        ui.checkBoxRubber->setChecked(myFly->rubber_is_hidden);
        myFly->rubber->rubberband->setVisible(!myFly->rubber_is_hidden);

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect( ui.checkBoxRubber,SIGNAL(stateChanged(int)),this,SLOT(toggleRubber(int)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSpinBox(int))); ui.spinBox##x->setKeyboardTracking(true);
        SPINNER(Left);
        SPINNER(Right);
        SPINNER(Top);
        SPINNER(Bottom);
#undef SPINNER
        connect(ui.comboBoxAlgorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(valueChanged(int)));
#define SPINNER(x) \
        connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); \
        connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSpinBox(int)));
        SPINNER(Radius)

        QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        setModal(true);
}
void Ui_blurWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
void Ui_blurWindow::gather(blur *param)
{
    myFly->download();
    memcpy(param,&(myFly->param),sizeof(blur));
    param->left=myFly->left;
    param->right=myFly->right;
    param->top=myFly->top;
    param->bottom=myFly->bottom;
    param->rubber_is_hidden=myFly->rubber_is_hidden;
}
Ui_blurWindow::~Ui_blurWindow()
{
    if(myFly) {
        ADMVideoBlur::BlurDestroyBuffers(myFly->rgbBufRaw, myFly->rgbBufImage, myFly->convertYuvToRgb, myFly->convertRgbToYuv);
        delete myFly;
    }
    myFly=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
#define COPY_VALUE_TO_SPINBOX(x) \
        ui.spinBox##x->blockSignals(true); \
        ui.spinBox##x->setValue(ui.horizontalSlider##x->value()); \
        ui.spinBox##x->blockSignals(false);
void Ui_blurWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SPINBOX(Radius);
    myFly->download();
    myFly->sameImage();
    lock--;
}
#define COPY_VALUE_TO_SLIDER(x) \
        ui.horizontalSlider##x->blockSignals(true); \
        ui.horizontalSlider##x->setValue(ui.spinBox##x->value()); \
        ui.horizontalSlider##x->blockSignals(false);
void Ui_blurWindow::valueChangedSpinBox(int foo)
{
    if(lock) return;
    lock++;
    COPY_VALUE_TO_SLIDER(Radius);
    myFly->download();
    myFly->sameImage();
    lock--;
}
/**
 * \fn reset
 */
void Ui_blurWindow::reset( bool f )
{
    myFly->left=0;
    myFly->right=0;
    myFly->bottom=0;
    myFly->top=0;
    lock++;
    myFly->upload();
    myFly->sameImage();
    lock--;
}
/**
 * \fn toggleRubber
 */
void Ui_blurWindow::toggleRubber(int checkState)
{
    bool visible=true;
    if(checkState)
        visible=false;
    myFly->rubber->rubberband->setVisible(visible);
    myFly->rubber_is_hidden=!visible;
}
void Ui_blurWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();

    int x=(int)((double)myFly->left*myFly->_zoom);
    int y=(int)((double)myFly->top*myFly->_zoom);
    int w=(int)((double)(myFly->_w-(myFly->left+myFly->right))*myFly->_zoom);
    int h=(int)((double)(myFly->_h-(myFly->top+myFly->bottom))*myFly->_zoom);

    myFly->blockChanges(true);
    myFly->rubber->nestedIgnore++;
    myFly->rubber->move(x,y);
    myFly->rubber->resize(w,h);
    myFly->rubber->nestedIgnore--;
    myFly->blockChanges(false);
}

void Ui_blurWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
    myFly->rubber->nestedIgnore=0;
    if(myFly->rubber_is_hidden)
        myFly->rubber->rubberband->hide();
}

#define MYCOMBOX(x) w->comboBox##x
#define MYSLIDER(x) w->horizontalSlider##x
#define MYSPIN(x) w->spinBox##x
#define MYCHECK(x) w->checkBox##x
#define UPLOADSPIN(x, value) \
        w->spinBox##x->blockSignals(true); \
        w->spinBox##x->setValue(value); \
        w->spinBox##x->blockSignals(false);
//************************
uint8_t flyBlur::upload(bool redraw, bool toRubber)
{
    if(!redraw)
    {
        blockChanges(true);
    }

    Ui_blurDialog *w=(Ui_blurDialog *)_cookie;
    MYCOMBOX(Algorithm)->setCurrentIndex(param.algorithm);
    MYSLIDER(Radius)->setValue((int)param.radius);
    UPLOADSPIN(Radius, param.radius);

    w->spinBoxLeft->setValue(left);
    w->spinBoxRight->setValue(right);
    w->spinBoxTop->setValue(top);
    w->spinBoxBottom->setValue(bottom);

    if(toRubber)
    {
        rubber->nestedIgnore++;
        rubber->move(_zoom*(float)left,_zoom*(float)top);
        rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
        rubber->nestedIgnore--;
    }

    if(!redraw)
    {
        blockChanges(false);
    }

    return 1;
}
uint8_t flyBlur::download(void)
{
    int reject=0;
    Ui_blurDialog *w=(Ui_blurDialog *)_cookie;
    param.algorithm=MYCOMBOX(Algorithm)->currentIndex();
    param.radius=(int)MYSLIDER(Radius)->value();

#define SPIN_GET(x,y) x=w->spinBox##y->value();
    SPIN_GET(left,Left);
    SPIN_GET(right,Right);
    SPIN_GET(top,Top);
    SPIN_GET(bottom,Bottom);

    if((top+bottom)>_h)
        {
                top=bottom=0;
                reject=1;
        }
    if((left+right)>_w)
        {
                left=right=0;
                reject=1;
        }
    if(reject)
        upload();
    else
   {
       blockChanges(true);
       rubber->nestedIgnore++;
       rubber->move(_zoom*(float)left,_zoom*(float)top);
       rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
       rubber->nestedIgnore--;
       blockChanges(false);
   }
    return 1;
}
void flyBlur::setTabOrder(void)
{
    Ui_blurDialog *w=(Ui_blurDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSHCOMBOX(x) controls.push_back(MYCOMBOX(x));
#define PUSHSLIDER(x) controls.push_back(MYSLIDER(x));
#define PUSH_SPIN(x) controls.push_back(MYSPIN(x));
#define PUSH_TOG(x) controls.push_back(w->checkBox##x);
    PUSH_SPIN(Left)
    PUSH_SPIN(Right)
    PUSH_SPIN(Top)
    PUSH_SPIN(Bottom)
    PUSH_TOG(Rubber)

    PUSHCOMBOX(Algorithm)
    PUSHSLIDER(Radius)
    PUSH_SPIN(Radius)

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
 * \fn blockChanges
 * @param block
 * @return 
 */
#define APPLY_TO_ALL(x) {w->spinBoxLeft->x;w->spinBoxRight->x;w->spinBoxTop->x;w->spinBoxBottom->x;rubber->x;}
bool flyBlur::blockChanges(bool block)
{
    Ui_blurDialog *w=(Ui_blurDialog *)_cookie;
    APPLY_TO_ALL(blockSignals(block));
    return true;
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_getBlur(blur *param,ADM_coreVideoFilter *in)
{
    uint8_t ret=0;
    Ui_blurWindow dialog(qtLastRegisteredDialog(), param,in);

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


