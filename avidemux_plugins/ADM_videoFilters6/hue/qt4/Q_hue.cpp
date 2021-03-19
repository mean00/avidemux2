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
        myCrop->setParam(param);
        myCrop->_cookie=&ui;
        myCrop->addControl(ui.toolboxLayout, true);
        myCrop->setTabOrder();
        myCrop->upload();
        myCrop->sliderChanged();

        ui.horizontalSliderHue->setFocus();
        ui.horizontalSliderSaturation->setScale(1,10,1);
        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(Hue);
          SPINNER(Saturation);

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
        myCrop->getParam(param);
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

void Ui_hueWindow::reset(void)
{
    if(lock) return;
    lock++;
    myCrop->reset();
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

    MYSPIN(Saturation)->setValue((int)(flyset.param.saturation*10));
    MYSPIN(Hue)->setValue((int)flyset.param.hue);

    MYSPIN(Saturation)->blockSignals(false);
    MYSPIN(Hue)->blockSignals(false);

    update();

    return 1;
}
uint8_t flyHue::download(void)
{
    Ui_hueDialog *w=(Ui_hueDialog *)_cookie;

    flyset.param.hue=MYSPIN(Hue)->value();
    flyset.param.saturation=MYSPIN(Saturation)->value()/10.;

    update();

    return 1;
}

void flyHue::setTabOrder(void)
{
    Ui_hueDialog *w=(Ui_hueDialog *)_cookie;
    std::vector<QWidget *> controls;
    controls.push_back(MYSPIN(Hue));
    controls.push_back(MYSPIN(Saturation));
    controls.insert(controls.end(), buttonList.begin(), buttonList.end());
    controls.push_back(w->horizontalSlider);
    //controls.push_back(w->graphicsView);
#if 0 /* don't mess with button box */
    controls.push_back(w->buttonBox->button(QDialogButtonBox::Reset));
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


