/***************************************************************************
                        Q_mpdelogo.cpp  -  description
                             -------------------

			   flyDialog for MPlayer DeLogo filter

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

#include "Q_mpdelogo.h"
#include "ADM_toolkitQt.h"

#if 1
#define aprintf printf
#else
#define aprintf(...) {}
#endif
/**
 * 
 * @param z
 * @param w
 * @param h
 */
 ADM_LogoCanvas::ADM_LogoCanvas(QWidget *z, uint32_t w, uint32_t h) : ADM_QCanvas(z,w,h)
 {
     
 }
 /**
  * 
  */
 ADM_LogoCanvas::~ADM_LogoCanvas()
 {
     
 }

/**
 * 
 * @param event
 */
void ADM_LogoCanvas::mousePressEvent(QMouseEvent * event)
{
    aprintf("Pressed\n");
}
/**
 * 
 * @param event
 */
void ADM_LogoCanvas::mouseReleaseEvent(QMouseEvent * event)
{
   
    int x,y;
    QPoint p=event->pos();
    QPoint pp=pos();
    aprintf("Evt %d %d, %d %d\n",p.x(),p.y(), pp.x(),pp.y());
#if 1 
     x=p.x()-pp.x();
     y=p.y()-pp.y();
#else
     x=p.x();
     y=p.y();
#endif     
    
    if(x<0) x=0;
    if(y<0) y=0;    
    
    aprintf("Released %d %d\n",x,y);
    emit movedSignal(x,y);

}
/**
 * 
 * @param event
 */
void ADM_LogoCanvas::moveEvent(QMoveEvent * event)
{
    aprintf("Move\n");
}
/**
    \fn ctor
*/

  Ui_mpdelogoWindow::Ui_mpdelogoWindow(QWidget *parent,  delogo *param, ADM_coreVideoFilter *in) 
            : QDialog(parent)
  {
        uint32_t width,height;
        ui.setupUi(this);
        _in=in;
        
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_LogoCanvas(ui.graphicsView,width,height);
        
        myCrop=new flyMpDelogo( width, height,in,canvas,ui.horizontalSlider);
        myCrop->param=*param;
        myCrop->_cookie=&ui;
        myCrop->setPreview(false);
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
        
        
        connect(canvas, SIGNAL(movedSignal(int,int)),this, SLOT(moved(int,int)));
        connect(ui.checkBoxPreview, SIGNAL(stateChanged(int )),this, SLOT(preview(int)));
          
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

/**
 * 
 * @param x
 * @param y
 */
void Ui_mpdelogoWindow::moved(int x,int y)
{
      aprintf("Moved %d %d\n",x,y);
      aprintf("Change (lock=%d)\n",lock);
      if(lock) return;
      lock++;
      
      int max_x=_in->getInfo()->width;
      if(x>(max_x-myCrop->param.lw)) x=max_x-myCrop->param.lw;
      
      int max_y=_in->getInfo()->height;
      if(y>(max_y-myCrop->param.lh)) y=max_y-myCrop->param.lh;
      
      myCrop->setXy(x,y);
      myCrop->sameImage();
      
      lock--;
}
/**
 * 
 * @param x
 */
 void Ui_mpdelogoWindow::preview(int x)
 {
     aprintf("Preview = %d\n",x);
     if(x==Qt::Checked)
     {
         myCrop->setPreview(true);
         myCrop->sameImage();
     }
     else
     {
         myCrop->setPreview(false);
         myCrop->sameImage();
     }
 }

/**
 * 
 * @param x
 * @param y
 * @return 
 */
bool flyMpDelogo::setXy(int x,int y)
{
      param.xoff= x;
      if(param.xoff<0) param.xoff=0;
      param.yoff= y;
      if(param.yoff<0) param.yoff=0;
      upload();
      return true;
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
        bool ret=false;
        
        Ui_mpdelogoWindow dialog(qtLastRegisteredDialog(), param,in);
		qtRegisterDialog(&dialog);

        if(dialog.exec()==QDialog::Accepted)
        {
            dialog.gather(param); 
            ret=true;
        }

        qtUnregisterDialog(&dialog);
        return ret;
}

//____________________________________
// EOF


