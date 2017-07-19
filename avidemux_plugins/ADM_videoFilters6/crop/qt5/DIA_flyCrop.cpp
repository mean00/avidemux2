/***************************************************************************
                          DIA_flyCrop.cpp  -  description
                             -------------------

        Common part of the crop dialog
    
    copyright            : (C) 2002/2017 by mean
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
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyCrop.h"
#include "Q_crop.h"
#include "ADM_toolkitQt.h"



uint8_t 		Metrics( uint8_t *in, uint32_t width,uint32_t *avg, uint32_t *eqt);
uint8_t 		MetricsV( uint8_t *in, uint32_t width,uint32_t height,uint32_t *avg, uint32_t *eqt);

/**
 * 
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
flyCrop::flyCrop (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, QSlider *slider) 
                : ADM_flyDialogRgb(parent,width, height,in,canvas, slider,RESIZE_LAST)
{
    rubber=new cropRubber(this,canvas);
    rubber->resize(width,height);
}
flyCrop::~flyCrop()
{
    delete rubber;
    rubber=NULL;
}

/**
    \fn process
	\brief 
*/
uint8_t    flyCrop::processRgb(uint8_t *imageIn, uint8_t *imageOut)
{
        uint32_t x,y;
        uint8_t  *in;
        uint32_t w=_w,h=_h;
  
        
        memcpy(imageOut,imageIn,_w*_h*4);
        in=imageOut;
        for(y=0;y<top;y++)
        {
                for(x=0;x<w;x++)
                {
                        *in++=0;
                        
                        
                        *in++=0xff;
                        
                        *in++=0;
                        *in++=0;
                }
        }
        // bottom
        in=imageOut+(w*4)*(h-bottom);
        for(y=0;y<bottom;y++)
        {
                for(x=0;x<w;x++)
                {
                        *in++=0;
                        
                        
                        *in++=0xff;
                        *in++=0;
                        *in++=0;
                }
        }
        // left
        in=imageOut;
        uint32_t stride=4*w-4;
        for(y=0;y<h;y++)
        {
                for(x=0;x<left;x++)
                {
                        *(in+4*x)=0;
                        
                        
                        *(in+4*x+1)=0xff;
                        *(in+4*x+2)=0;
                        *(in+4*x+3)=0;
                }
                for(x=0;x<right;x++)
                {
                        *(in-4*x+stride-4)=0;
                        
                        
                        *(in-4*x+stride-3)=0xff;
                        *(in-4*x+stride-2)=0;
                        *(in-4*x+stride-1)=0;
                        
                }
                in+=4*w;
  
        }
        return true;

}

/**
 * 
 * @param x
 * @param y
 * @param w
 * @param h
 * @return 
 */


bool    flyCrop::bandResized(int x,int y,int w, int h)
{
    aprintf("Rubber resize %d x %d, w=%d h=%d\n",x,y,w,h);
    int nw,nh,nx,ny;
        
    double halfzoom=_zoom/2-0.01;
    
    top=((double)y+halfzoom)/_zoom;
    left=((double)x+halfzoom)/_zoom;
    int r=_w-(((double)(x+w)+halfzoom)/_zoom);
    if(r<0) 
        r=0;
    if(r+left>_w) 
        { r=_w-left;}
    right=r;
    int b=_h-(((double)(y+h)+halfzoom)/_zoom);
    if(b<0) 
        b=0;
    if(b+top>_h) 
        { b=_h-top;}
    bottom=b;
    upload();
    return true; 
}
/**
 * 
 * @param block
 * @return 
 */
#define APPLY_TO_ALL(x) {w->spinBoxLeft->x;w->spinBoxRight->x;w->spinBoxTop->x;w->spinBoxBottom->x;}
bool flyCrop::blockChanges(bool block)
{
//    Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
//    APPLY_TO_ALL(blockSignals(block));
    rubber->blockSignals(block);
    return true;
}
/**
     \fn autocrop
	\brief 
*/

uint8_t  flyCrop::autocrop(void)
{
uint8_t *in;
uint32_t y,avg,eqt;
	// Top

#define THRESH_AVG   30
#define THRESH_EQT   50
        
        in=_yuvBuffer->GetReadPtr(PLANAR_Y);
        for(y=0;y<((_h>>1)-2);y++)	
        {
                Metrics(in,_w,&avg,&eqt);
                in+=_w;
                //printf("LineT :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                        break;
        }
//gotcha_:	
        if(y)
                top=y-1;
        else 
                top=0;
                
        in=_yuvBuffer->GetReadPtr(PLANAR_Y)+_w*(_h-1);
        for(y=0;y<((_h>>1)-2);y++)	
        {
                Metrics(in,_w,&avg,&eqt);
                in-=_w;
                //printf("Line B :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                                break;
        }
//gotcha_:	
        if(y)
                bottom=y-1;
        else
                bottom=0;

                
// Left
        in=_yuvBuffer->GetReadPtr(PLANAR_Y);
        for(y=0;y<((_w>>1)-2);y++)	
        {
                MetricsV(in,_w,_h,&avg,&eqt);
                in++;
                //printf("Line L :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                                break;
        }
//gotcha_:	
        if(y)
                left=y-1;
        else
                left=0;		
// Right
        in=_yuvBuffer->GetReadPtr(PLANAR_Y)+_w-1;
        for(y=0;y<((_w>>1)-2);y++)	
        {
                MetricsV(in,_w,_h,&avg,&eqt);
                in--;
                //printf("Line R :%d avg: %d eqt: %d\n",y,avg,eqt);
                if(avg> THRESH_AVG || eqt > THRESH_EQT)
                                break;
        }
//gotcha_:	
        if(y)
                right=y-1;
        else
                right=0;
  
              
        // Update display
        top=top & 0xfffe;
        bottom=bottom & 0xfffe;
        upload();
        sameImage();
        return 1;
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
uint8_t flyCrop::download(void)
{
        int reject=0;
Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
#define SPIN_GET(x,y) x=w->spinBox##y->value();
                        SPIN_GET(left,Left);
                        SPIN_GET(right,Right);
                        SPIN_GET(top,Top);
                        SPIN_GET(bottom,Bottom);
                        
                        printf("%d %d %d %d\n",left,right,top,bottom);
                        
                        left&=0xffffe;
                        right&=0xffffe;
                        top&=0xffffe;
                        bottom&=0xffffe;
                        
                        if((top+bottom)>_h)
                                {
                                        top=bottom=0;
                                        reject=1;
                                }
                        if((left+right)>_w)
                                {
                                        left=right=0;
                                        reject=1;
                                }
                        if(reject)
                                upload();
                        return true;
}

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
        myCrop->sliderChanged();


        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect( ui.pushButtonAutoCrop,SIGNAL(clicked(bool)),this,SLOT(autoCrop(bool)));
        connect( ui.pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(Left);
          SPINNER(Right);
          SPINNER(Top);
          SPINNER(Bottom);

        show();
        myCrop->adjustCanvasPosition();
        canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
  }
  void Ui_cropWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
  void Ui_cropWindow::gather(crop *param)
  {
    
        myCrop->download();
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

void Ui_cropWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myCrop->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myCrop->adjustCanvasPosition();
}

//EOF

