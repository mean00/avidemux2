/***************************************************************************
                         Q_Asharp.cpp  -  description
                             -------------------

			     flyDialog for Asharp
			     +Revisted the Gtk2 way

    copyright            : (C) 2004/2007 by mean
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

#include "Q_asharp.h"
#include "ADM_toolkitQt.h"

//
//	Video is in YV12 Colorspace
//
//
  Ui_asharpWindow::Ui_asharpWindow(QWidget *parent,  asharp *param, ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myCrop=new flyASharp( width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myCrop->param),param,sizeof(asharp));
        myCrop->_cookie=&ui;
        myCrop->upload();
        myCrop->sliderChanged();


        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.doubleSpinBox##x,SIGNAL(valueChanged(double)),this,SLOT(valueChanged(double))); 
          SPINNER(Treshold);
          SPINNER(Strength);
          SPINNER(Block);
          connect( ui.checkBox,SIGNAL(stateChanged(int)),this,SLOT(valueChanged2(int))); 

  }
  void Ui_asharpWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
  void Ui_asharpWindow::gather(asharp *param)
  {
    
        myCrop->download();
        memcpy(param,&(myCrop->param),sizeof(asharp));
  }
Ui_asharpWindow::~Ui_asharpWindow()
{
  if(myCrop) delete myCrop;
  myCrop=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_asharpWindow::valueChanged2( int f )
{
    valueChanged(0);
}
void Ui_asharpWindow::valueChanged( double f )
{
  printf("Update \n");
  if(lock) return;
  lock++;
  myCrop->download();
  myCrop->sameImage();
  lock--;
}

#define MYSPIN(x) w->doubleSpinBox##x
//************************
uint8_t flyASharp::upload(void)
{
      Ui_asharpDialog *w=(Ui_asharpDialog *)_cookie;

        MYSPIN(Treshold)->setValue(param.t);
        MYSPIN(Strength)->setValue(param.d);
        MYSPIN(Block)->setValue(param.b);
        
        //w->bf->w->checkBox->isChecked();
        w->checkBox->setChecked(param.bf);

        return 1;
}
uint8_t flyASharp::download(void)
{
       Ui_asharpDialog *w=(Ui_asharpDialog *)_cookie;
       param.t= MYSPIN(Treshold)->value();
       param.d= MYSPIN(Strength)->value();
       param.b= MYSPIN(Block)->value();
       
       //w->spinBoxBottom->setValue(bottom);
       param.bf=w->checkBox->isChecked();
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_getASharp(asharp *param, ADM_coreVideoFilter *in)
{
        uint8_t ret=0;
        
        Ui_asharpWindow dialog(qtLastRegisteredDialog(), param,in);
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


