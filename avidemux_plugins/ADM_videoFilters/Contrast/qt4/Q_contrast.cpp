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

#include "Q_contrast.h"
#include "ADM_toolkitQt.h"

//
//	Video is in YV12 Colorspace
//
//
  Ui_contrastWindow::Ui_contrastWindow(QWidget* parent, CONTRAST_PARAM *param,AVDMGenericVideoStream *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myCrop=new flyContrast( width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myCrop->param),param,sizeof(CONTRAST_PARAM));
        myCrop->_cookie=&ui;
        myCrop->upload();
        myCrop->sliderChanged();


        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(Brightness);
          SPINNER(Contrast);
          connect( ui.checkBoxU,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
          connect( ui.checkBoxV,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int))); 
          connect( ui.checkBoxY,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));  

  }
  void Ui_contrastWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
  void Ui_contrastWindow::gather(CONTRAST_PARAM *param)
  {
    
        myCrop->download();
        memcpy(param,&(myCrop->param),sizeof(CONTRAST_PARAM));
  }
Ui_contrastWindow::~Ui_contrastWindow()
{
  if(myCrop) delete myCrop;
  myCrop=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_contrastWindow::valueChanged( int f )
{
  if(lock) return;
  lock++;
  myCrop->update();
  lock--;
}

#define MYSPIN(x) w->horizontalSlider##x
#define MYCHECK(x) w->checkBox##x
//************************
uint8_t flyContrast::upload(void)
{
      Ui_contrastDialog *w=(Ui_contrastDialog *)_cookie;

        MYSPIN(Contrast)->setValue((uint32_t)(param.coef*10));
        MYSPIN(Brightness)->setValue(param.offset);
#define CHECKSET(a,b) MYCHECK(a)->setChecked(param.b)
        
        CHECKSET(Y,doLuma);
        CHECKSET(U,doChromaU);
        CHECKSET(V,doChromaV);

        return 1;
}
uint8_t flyContrast::download(void)
{
       Ui_contrastDialog *w=(Ui_contrastDialog *)_cookie;
         param.coef=MYSPIN(Contrast)->value()/10.;
         param.offset=MYSPIN(Brightness)->value();
#define CHECKGET(a,b) param.b=MYCHECK(a)->isChecked()
        
        CHECKGET(Y,doLuma);
        CHECKGET(U,doChromaU);
        CHECKGET(V,doChromaV);
return 1;
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
uint8_t DIA_contrast(AVDMGenericVideoStream *in,CONTRAST_PARAM *param)
{
        uint8_t ret=0;
        Ui_contrastWindow dialog(qtLastRegisteredDialog(), param,in);
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


