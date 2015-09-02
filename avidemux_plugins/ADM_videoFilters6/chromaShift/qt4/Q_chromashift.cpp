/***************************************************************************
                          DIA_chromashift.cpp  -  description
                             -------------------

			     GUI for chroma shifting
			     +Revisted the Gtk2 way

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

#include "Q_chromashift.h"
#include "ADM_toolkitQt.h"

Ui_chromaShiftWindow::Ui_chromaShiftWindow(QWidget* parent, chromashift *param,ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;
        
        int boundary=width/2;
        
        ui.spinBoxU->setMaximum(boundary/2);
        ui.spinBoxU->setMinimum(-boundary);
        ui.spinBoxV->setMaximum(boundary/2);
        ui.spinBoxV->setMinimum(-boundary);
        
        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myCrop=new flyChromaShift( width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(myCrop->param),param,sizeof(chromashift));
        myCrop->_cookie=&ui;
        myCrop->upload();
        myCrop->sliderChanged();


        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(U);
          SPINNER(V);
  }
  void Ui_chromaShiftWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
  void Ui_chromaShiftWindow::gather(chromashift *param)
  {
    
        myCrop->download();
        memcpy(param,&(myCrop->param),sizeof(chromashift));
  }
Ui_chromaShiftWindow::~Ui_chromaShiftWindow()
{
  if(myCrop) delete myCrop;
  myCrop=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_chromaShiftWindow::valueChanged( int f )
{
  if(lock) return;
  //printf("Chroma : value changed\n");
  lock++;
  myCrop->download();
  myCrop->sameImage();
  lock--;
}

#define MYSPIN(x) w->spinBox##x
//************************
uint8_t flyChromaShift::upload(void)
{
      Ui_chromashiftDialog *w=(Ui_chromashiftDialog *)_cookie;

        MYSPIN(U)->setValue(param.u);
        MYSPIN(V)->setValue(param.v);
        return 1;
}
uint8_t flyChromaShift::download(void)
{
       Ui_chromashiftDialog *w=(Ui_chromashiftDialog *)_cookie;
       param.u= MYSPIN(U)->value();
       param.v= MYSPIN(V)->value();
       return true;
}

/**
      \fn     DIA_getChromaShift
      \brief  Handle crop dialog
*/
uint8_t DIA_getChromaShift( ADM_coreVideoFilter *in,chromashift    *param )
{
        uint8_t ret=0;

        Ui_chromaShiftWindow dialog(qtLastRegisteredDialog(), param,in);
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


