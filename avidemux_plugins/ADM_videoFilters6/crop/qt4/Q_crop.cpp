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
#include "Q_crop.h"
#include "ADM_toolkitQt.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_cropWindow::Ui_cropWindow(QWidget* parent, crop *param,ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myCrop=new flyCrop( this,width, height,in,canvas,ui.horizontalSlider);
        myCrop->left=param->left;
        myCrop->right=param->right;
        myCrop->top=param->top;
        myCrop->bottom=param->bottom;
        myCrop->_cookie=&ui;
        myCrop->addControl(ui.toolboxLayout);
        myCrop->upload();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect( ui.pushButtonAutoCrop,SIGNAL(clicked(bool)),this,SLOT(autoCrop(bool)));
        connect( ui.pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(Left);
          SPINNER(Right);
          SPINNER(Top);
          SPINNER(Bottom);
  }
  void Ui_cropWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
  void Ui_cropWindow::gather(crop *param)
  {
        myCrop->download(true);
        param->left=myCrop->left;
        param->right=myCrop->right;
        param->top=myCrop->top;
        param->bottom=myCrop->bottom;
  }
Ui_cropWindow::~Ui_cropWindow()
{
  if(myCrop) delete myCrop;
  myCrop=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_cropWindow::valueChanged( int f )
{
  if(lock) return;
  lock++;
  myCrop->download();
  myCrop->sameImage();
  lock--;
}

void Ui_cropWindow::autoCrop( bool f )
{
  lock++;
  myCrop->autocrop();
  lock--;
}
void Ui_cropWindow::reset( bool f )
{
         myCrop->left=0;
         myCrop->right=0;
         myCrop->bottom=0;
         myCrop->top=0;
         lock++;
         myCrop->upload();
         myCrop->sameImage();
         lock--;
}

//************************
uint8_t flyCrop::upload(void)
{
      Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
        
        w->spinBoxLeft->setValue(left);
        w->spinBoxRight->setValue(right);
        w->spinBoxTop->setValue(top);
        w->spinBoxBottom->setValue(bottom);
        
        return 1;
}
uint8_t flyCrop::download(bool even)
{
    int reject=0;
    Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
#define SPIN_GET(x,y) x=w->spinBox##y->value();
    SPIN_GET(left,Left);
    SPIN_GET(right,Right);
    SPIN_GET(top,Top);
    SPIN_GET(bottom,Bottom);

    printf("%d %d %d %d\n",left,right,top,bottom);

    if((top+bottom)>_h)
    {
        top=bottom=0;
        reject=1;
        ADM_warning(" ** Rejected top bottom **\n");
    }
    if((left+right)>_w)
    {
        left=right=0;
        reject=1;
        ADM_warning(" ** Rejected left right **\n");
    }
    if(reject)
        return upload();

    if(even)
    {
        if((_w-left-right)&1)
        {
            if(left&1)
                left&=0xfffe;
            else if(right)
                right--;
            else if(left)
                left--;
            else
                right++;
        }
        if((_h-top-bottom)&1)
        {
            if(top&1)
                top&=0xfffe;
            else if(bottom)
                bottom--;
            else if(top)
                top--;
            else
                bottom++;
        }
    }
    return 1;
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
int DIA_getCropParams(	const char *name,crop *param,ADM_coreVideoFilter *in)
{
        uint8_t ret=0;
        
        Ui_cropWindow dialog(qtLastRegisteredDialog(), param,in);
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
