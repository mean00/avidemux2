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

#include "Q_logo.h"
#include "ADM_toolkitQt.h"
#include "ADM_imageLoader.h"
#include "DIA_fileSel.h"

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
 * 
 * @param enabled
 * @return 
 */
bool                Ui_logoWindow::enableLowPart(bool enabled)
{
    ui.spinX->setEnabled(enabled);
    ui.spinY->setEnabled(enabled);
    ui.spinAlpha->setEnabled(enabled);
    ui.horizontalSlider->setEnabled(enabled);
    return true;
}
/**
 * 
 * @return 
 */
void                   Ui_logoWindow::imageSelect()
{
     
    char buffer[2048];
    if(FileSel_SelectWrite("Select Logo Image",buffer,2048,""))
    {
        if(tryToLoadimage(buffer))
        {
        }
    }
}
/**
 * \fn tryToLoadimage
 * @param image
 * @return 
 */
bool                Ui_logoWindow::tryToLoadimage(const char *imageName)
{
    bool status=false;
    if(strlen(imageName))
    {
        ADMImage *im2=createImageFromFile(imageName);
        if(im2)
        {
            if(image) delete image;
            image=im2;
            imageWidth=image->GetWidth(PLANAR_Y);
            imageHeight=image->GetHeight(PLANAR_Y);            
            this->imageName=std::string(imageName);
            ui.labelImage->setText(this->imageName.c_str());
            status=true;
        }
    }
    enableLowPart(status);
    return status;
}

/**
    \fn ctor
*/

  Ui_logoWindow::Ui_logoWindow(QWidget *parent,  logo *param, ADM_coreVideoFilter *in) 
            : QDialog(parent)
  {
        uint32_t width,height;
        ui.setupUi(this);
        _in=in;
        
        image=NULL;;
        alpha=param->alpha;
        if(param->logo)
        {
            if(tryToLoadimage(param->logo))
            {
                imageName=std::string(param->logo);
            }
        }
        
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_LogoCanvas(ui.graphicsView,width,height);
        
        myLogo=new flyLogo( width, height,in,canvas,ui.horizontalSlider);
        myLogo->setParent(this);
        myLogo->param=*param;
        myLogo->_cookie=&ui;
        myLogo->setPreview(false);
#define SPINENTRY(x) ui.x
        SPINENTRY(spinX)->setMaximum(width);
        
        SPINENTRY(spinY)->setMaximum(height);
        
        SPINENTRY(spinX)->setSingleStep(5);
        SPINENTRY(spinY)->setSingleStep(5);
        
        myLogo->upload();
        myLogo->sliderChanged();
        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect( ui.pushButtonSelect,SIGNAL(pressed()),this,SLOT(imageSelect()));
#define SPINNER(x) connect( ui.x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
        SPINNER(spinX);
        SPINNER(spinY);                
        
        connect(canvas, SIGNAL(movedSignal(int,int)),this, SLOT(moved(int,int)));
        show();
          
  }
/**
    \fn sliderUpdate
*/
void Ui_logoWindow::sliderUpdate(int foo)
{
    myLogo->sliderChanged();
}
/**
    \fn gather
*/
void Ui_logoWindow::gather(logo *param)
{
    myLogo->download();
#define DUPE(x)     param->x=myLogo->param.x;
    DUPE(x)
    DUPE(y)
    DUPE(alpha)
    param->logo=ADM_strdup(imageName.c_str());
    
}
/**
    \fn dtor
*/
Ui_logoWindow::~Ui_logoWindow()
{
    if(myLogo) delete myLogo;
    myLogo=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
/**
    \fn valueChanged
*/

void Ui_logoWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myLogo->download();
    myLogo->sameImage();
    lock--;
}

/**
 * 
 * @param x
 * @param y
 */
void Ui_logoWindow::moved(int x,int y)
{
      if(lock) return;
      lock++;
#if 0      
      int max_x=_in->getInfo()->width;
      if(x>(max_x-myLogo->param.lw)) x=max_x-myLogo->param.lw;
      
      int max_y=_in->getInfo()->height;
      if(y>(max_y-myLogo->param.lh)) y=max_y-myLogo->param.lh;
#endif      
      myLogo->setXy(x,y);
      myLogo->sameImage();
      
      lock--;
}
/**
 * 
 * @param x
 */
 void Ui_logoWindow::preview(int x)
 {
     if(x==Qt::Checked)
     {
         myLogo->setPreview(true);
         myLogo->sameImage();
     }
     else
     {
         myLogo->setPreview(false);
         myLogo->sameImage();
     }
 }

/**
 * 
 * @param x
 * @param y
 * @return 
 */
bool flyLogo::setXy(int x,int y)
{
#if 0
      param.xoff= x;
      if(param.xoff<0) param.xoff=0;
      param.yoff= y;
      if(param.yoff<0) param.yoff=0;
      upload();
#endif
      return true;
}

//************************
/**
    \fn upload
*/
uint8_t flyLogo::upload(void)
{

    
#define MYSPIN(x) parent->ui.x
    MYSPIN(spinX)->setValue(param.x);
    MYSPIN(spinY)->setValue(param.y);
    MYSPIN(spinAlpha)->setValue(param.alpha);   
    return 1;
}
/**
        \fn download
*/
uint8_t flyLogo::download(void)
{
    param.x= MYSPIN(spinX)->value();
    param.y= MYSPIN(spinY)->value();
    param.alpha= MYSPIN(spinAlpha)->value();
    return true;
}

/**
      \fn     DIA_getMpDelogo
      \brief  Handle delogo dialog
*/

bool DIA_getLogo(logo *param, ADM_coreVideoFilter *in)
{
    bool ret=false;

    Ui_logoWindow dialog(qtLastRegisteredDialog(), param,in);
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


