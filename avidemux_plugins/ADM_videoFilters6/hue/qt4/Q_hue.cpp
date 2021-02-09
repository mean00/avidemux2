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

#include "Q_hue.h"
#include "ADM_toolkitQt.h"
#include "../ADM_vidHue.h"

//
//	Video is in YV12 Colorspace
//
//
  Ui_hueWindow::Ui_hueWindow(QWidget *parent, hue *param,ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myCrop=new flyHue( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myCrop->param),param,sizeof(hue));
        myCrop->_cookie=&ui;
        myCrop->addControl(ui.toolboxLayout);
        myCrop->setTabOrder();
        myCrop->upload();
        myCrop->sliderChanged();

        ui.horizontalSliderSaturation->setScale(1,10,1);
        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(Hue);
          SPINNER(Saturation);

        connect(ui.checkBoxFullPreview,SIGNAL(stateChanged(int)),this,SLOT(toggleFullPreview(int)));

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked()),this,SLOT(reset()));

        setModal(true);
  }
  void Ui_hueWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
  void Ui_hueWindow::gather(hue *param)
  {
    
        myCrop->download();
        memcpy(param,&(myCrop->param),sizeof(hue));
  }
Ui_hueWindow::~Ui_hueWindow()
{
  if(myCrop) delete myCrop;
  myCrop=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_hueWindow::valueChanged( int f )
{
  if(lock) return;
  lock++;
   myCrop->download();
  myCrop->sameImage();
  lock--;
}
void Ui_hueWindow::toggleFullPreview(int state)
{
    if(lock) return;
    lock++;
    myCrop->fullpreview = state != Qt::Unchecked;
    myCrop->sameImage();
    lock--;
}

void Ui_hueWindow::reset(void)
{
    if(lock) return;
    lock++;
    ADMVideoHue::reset(&myCrop->param);
    ADMVideoHue::update(&myCrop->param);
    myCrop->upload();
    myCrop->sameImage();
    lock--;
}

void Ui_hueWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myCrop->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myCrop->adjustCanvasPosition();
}

void Ui_hueWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myCrop->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyHue::upload(void)
{
      Ui_hueDialog *w=(Ui_hueDialog *)_cookie;

        MYSPIN(Saturation)->blockSignals(true);
        MYSPIN(Hue)->blockSignals(true);
        MYCHECK(FullPreview)->blockSignals(true);

        MYSPIN(Saturation)->setValue((int)(param.saturation*10));
        MYSPIN(Hue)->setValue((int)param.hue);
        MYCHECK(FullPreview)->setChecked(fullpreview);

        MYSPIN(Saturation)->blockSignals(false);
        MYSPIN(Hue)->blockSignals(false);
        MYCHECK(FullPreview)->blockSignals(false);

        return 1;
}
uint8_t flyHue::download(void)
{
       Ui_hueDialog *w=(Ui_hueDialog *)_cookie;
         param.hue=MYSPIN(Hue)->value();
         param.saturation=MYSPIN(Saturation)->value()/10.;
         ADMVideoHue::update(&param);
return 1;
}

void flyHue::setTabOrder(void)
{
    Ui_hueDialog *w=(Ui_hueDialog *)_cookie;
    std::vector<QWidget *> controls;
    controls.push_back(MYSPIN(Hue));
    controls.push_back(MYSPIN(Saturation));
    for(std::vector<QWidget *>::iterator it = buttonList.begin(); it != buttonList.end(); ++it)
        controls.push_back(*it);
    controls.push_back(w->horizontalSlider);
    //controls.push_back(w->graphicsView);
    controls.push_back(MYCHECK(FullPreview));
    controls.push_back(w->buttonBox->button(QDialogButtonBox::Reset));
#if 0
    { // button box stuff
    QList<QAbstractButton *> buttonBoxList = w->buttonBox->buttons();
    QList<QAbstractButton *>::reverse_iterator bit;
    for (bit = buttonBoxList.rbegin(); bit != buttonBoxList.rend(); ++bit)
        controls.push_back(*bit);
    }
#endif
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
uint8_t DIA_getHue(hue *param,ADM_coreVideoFilter *in)
{
        uint8_t ret=0;
        Ui_hueWindow dialog(qtLastRegisteredDialog(), param,in);

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


