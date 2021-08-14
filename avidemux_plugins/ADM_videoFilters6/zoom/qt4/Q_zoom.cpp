/***************************************************************************
                          DIA_zoom.cpp  -  description
                             -------------------

			    GUI for zooming

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
#include "Q_zoom.h"
#include "ADM_toolkitQt.h"

//
//	Video is in YV12 Colorspace
//
//
Ui_zoomWindow::Ui_zoomWindow(QWidget* parent, zoom *param,ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        myFly=new flyZoom( this,width, height,in,canvas,ui.horizontalSlider);
        myFly->left=param->left;
        myFly->right=param->right;
        myFly->top=param->top;
        myFly->bottom=param->bottom;
        myFly->_cookie=&ui;
        myFly->addControl(ui.toolboxLayout);
        myFly->upload();
        myFly->sliderChanged();


        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect( ui.pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(Left);
          SPINNER(Right);
          SPINNER(Top);
          SPINNER(Bottom);
  }
  void Ui_zoomWindow::sliderUpdate(int foo)
  {
    myFly->sliderChanged();
  }
  void Ui_zoomWindow::gather(zoom *param)
  {
        myFly->download(true);
        param->left=myFly->left;
        param->right=myFly->right;
        param->top=myFly->top;
        param->bottom=myFly->bottom;
  }
Ui_zoomWindow::~Ui_zoomWindow()
{
  if(myFly) delete myFly;
  myFly=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_zoomWindow::valueChanged( int f )
{
  if(lock) return;
  lock++;
  myFly->download();
  myFly->sameImage();
  lock--;
}

void Ui_zoomWindow::reset( bool f )
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

void Ui_zoomWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
}

void Ui_zoomWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();
}

//************************
uint8_t flyZoom::upload(void)
{
      Ui_zoomDialog *w=(Ui_zoomDialog *)_cookie;
        
        w->spinBoxLeft->setValue(left);
        w->spinBoxRight->setValue(right);
        w->spinBoxTop->setValue(top);
        w->spinBoxBottom->setValue(bottom);
        
        return 1;
}
uint8_t flyZoom::download(bool even)
{
    int reject=0;
    Ui_zoomDialog *w=(Ui_zoomDialog *)_cookie;
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
      \fn     DIA_getZoomParams
      \brief  Handle zoom dialog
*/
int DIA_getZoomParams(	const char *name,zoom *param,ADM_coreVideoFilter *in)
{
        uint8_t ret=0;
        
        Ui_zoomWindow dialog(qtLastRegisteredDialog(), param,in);
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
