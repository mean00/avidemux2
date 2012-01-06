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

#include "Q_mpdelogo.h"
#include "ADM_toolkitQt.h"

/**
    \fn ctor
*/

  Ui_mpdelogoWindow::Ui_mpdelogoWindow(QWidget *parent,  delogo *param, ADM_coreVideoFilter *in) 
            : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myCrop=new flyMpDelogo( width, height,in,canvas,ui.horizontalSlider);
        myCrop->param=*param;
        myCrop->_cookie=&ui;
#define SPINENTRY(x) ui.x
        SPINENTRY(spinX)->setMaximum(width);
        SPINENTRY(spinW)->setMaximum(width);
        SPINENTRY(spinY)->setMaximum(height);
        SPINENTRY(spinH)->setMaximum(height);

        SPINENTRY(spinX)->setSingleStep(5);
        SPINENTRY(spinY)->setSingleStep(5);
        SPINENTRY(spinW)->setSingleStep(5);
        SPINENTRY(spinH)->setSingleStep(5);
        
        myCrop->upload();
        myCrop->sliderChanged();
        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(spinX);
          SPINNER(spinY);
          SPINNER(spinW);
          SPINNER(spinH);
          SPINNER(spinBand);
  }
/**
    \fn sliderUpdate
*/

  void Ui_mpdelogoWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
/**
    \fn gather
*/

  void Ui_mpdelogoWindow::gather(delogo *param)
  {
    
        myCrop->download();
        memcpy(param,&(myCrop->param),sizeof(delogo));
  }
/**
    \fn dtor
*/
Ui_mpdelogoWindow::~Ui_mpdelogoWindow()
{
  if(myCrop) delete myCrop;
  myCrop=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
/**
    \fn valueChanged
*/

void Ui_mpdelogoWindow::valueChanged( int f )
{
    printf("Change (lock=%d)\n",lock);
  if(lock) return;
  lock++;
  myCrop->download();
  myCrop->sameImage();
  lock--;
}

#define MYSPIN(x) w->x
//************************
/**
    \fn upload
*/
uint8_t flyMpDelogo::upload(void)
{

        Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;

        MYSPIN(spinX)->setValue(param.xoff);
        MYSPIN(spinY)->setValue(param.yoff);
        MYSPIN(spinW)->setValue(param.lw);
        MYSPIN(spinH)->setValue(param.lh);   
        MYSPIN(spinBand)->setValue(param.band);   
        
        printf("Upload\n");
        return 1;
}
/**
        \fn download
*/
uint8_t flyMpDelogo::download(void)
{

        Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
        param.xoff= MYSPIN(spinX)->value();
        param.yoff= MYSPIN(spinY)->value();
        param.lw= MYSPIN(spinW)->value();
        param.lh= MYSPIN(spinH)->value();
        param.band= MYSPIN(spinBand)->value();
       
        printf("Download\n");
        return true;
}

/**
      \fn     DIA_getMpDelogo
      \brief  Handle delogo dialog
*/
bool DIA_getMpDelogo(delogo *param, ADM_coreVideoFilter *in)
{
        uint8_t ret=0;
        
        Ui_mpdelogoWindow dialog(qtLastRegisteredDialog(), param,in);
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


