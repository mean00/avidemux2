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

#include "Q_msharpen.h"
#include "ADM_toolkitQt.h"

//
//	Video is in YV12 Colorspace
//
//
  Ui_msharpenWindow::Ui_msharpenWindow(QWidget *parent,  msharpen *param, ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        flymsharpen=new flyMSharpen( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(flymsharpen->param),param,sizeof(*param));
        flymsharpen->_cookie=&ui;
        flymsharpen->addControl(ui.toolboxLayout);
        flymsharpen->upload();
        flymsharpen->sliderChanged();


        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.doubleSpinBox##x,SIGNAL(valueChanged(double)),this,SLOT(valueChanged(double))); 
          SPINNER(Treshold);
          SPINNER(Strength);
          SPINNER(Block);
          //connect( ui.checkBox,SIGNAL(stateChanged(int)),this,SLOT(valueChanged2(int))); 

  }
  void Ui_msharpenWindow::sliderUpdate(int foo)
  {
    flymsharpen->sliderChanged();
  }
  void Ui_msharpenWindow::gather(msharpen *param)
  {
    
        flymsharpen->download();
        memcpy(param,&(flymsharpen->param),sizeof(*param));
  }
Ui_msharpenWindow::~Ui_msharpenWindow()
{
  if(flymsharpen) delete flymsharpen;
  flymsharpen=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_msharpenWindow::valueChanged( double f )
{
  printf("Update \n");
  if(lock) return;
  lock++;
  flymsharpen->download();
  flymsharpen->sameImage();
  lock--;
}

#define MYSPIN(x) w->doubleSpinBox##x

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_getASharp(msharpen *param, ADM_coreVideoFilter *in)
{
        uint8_t ret=0;
        
        Ui_msharpenWindow dialog(qtLastRegisteredDialog(), param,in);
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


