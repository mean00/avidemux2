/***************************************************************************
                          DIA_flyCrop.cpp  -  description
                             -------------------

        Common part of the crop dialog
    
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
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "DIA_flyCrop.h"

#if 1
    #define aprintf ADM_info
#else
    #define aprintf(...) {}
#endif

static uint8_t 		Metrics( uint8_t *in, uint32_t width,uint32_t *avg, uint32_t *eqt);
static uint8_t 		MetricsV( uint8_t *in, uint32_t width,uint32_t height,uint32_t *avg, uint32_t *eqt);

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
#define APPLY_TO_ALL(x) {w->spinX->x;w->spinY->x;w->spinW->x;w->spinH->x;w->spinBand->x;}
bool flyCrop::blockChanges(bool block)
{
    // Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
    // APPLY_TO_ALL(blockSignals(block));
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


/**
        \fn Ctor
*/
cropRubber::cropRubber(flyCrop *fly,QWidget *parent) : QWidget(parent) 
{
  nestedIgnore=0;
  flyParent=fly;
  //tell QSizeGrip to resize this widget instead of top-level window
  setWindowFlags(Qt::SubWindow);
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  QSizeGrip* grip1 = new QSizeGrip(this);
  QSizeGrip* grip2 = new QSizeGrip(this);
#ifdef __APPLE__
  // work around grips not shown on macOS
  grip1->setFixedSize(10,10);
  grip2->setFixedSize(10,10);
#endif
  grip1->setVisible(true);
  grip2->setVisible(true);
  layout->addWidget(grip1, 0, Qt::AlignLeft | Qt::AlignTop);
  layout->addWidget(grip2, 0, Qt::AlignRight | Qt::AlignBottom);
  rubberband = new QRubberBand(QRubberBand::Rectangle, this);
  QPalette pal;
  pal.setBrush(QPalette::Highlight, QBrush(Qt::red,Qt::DiagCrossPattern));
  rubberband->setPalette(pal);
  rubberband->setForegroundRole(QPalette::Highlight);
  rubberband->move(0, 0);
  rubberband->show();
  show();
}
/**
        \fn resizeEvent
*/
void cropRubber::resizeEvent(QResizeEvent *) 
{
  int x,y,w,h;
  x=pos().x();
  y=pos().y();
  w=size().width();
  h=size().height();
  aprintf("Resize event : %d x %d , %d x %d\n",x,y,w,h);
  rubberband->resize(size());
  if(!nestedIgnore)
    flyParent->bandResized(pos().x(),pos().y(),size().width(),size().height());
}


/**
     \fn Metrics
	\brief Compute the average value of pixels	and eqt is the "ecart type"
*/

uint8_t Metrics( uint8_t *in, uint32_t width,uint32_t *avg, uint32_t *eqt)
{

uint32_t x;
uint32_t sum=0,eq=0;
uint8_t v;
              for(x=0;x<width;x++)
              {
                      sum+=*(in+x);
              }
              sum=sum/width;
              *avg=sum;
              for(x=0;x<width;x++)
              {
                      v=*(in+x)-sum;
                      eq+=v*v;
              }
              eq=eq/(width*width);
              *eqt=eq;
              return 1;
}
/**
     \fn MetricsV
	\brief Compute the average value of pixels	and eqt is the "ecart type"
*/
uint8_t MetricsV( uint8_t *in,uint32_t width, uint32_t height,uint32_t *avg, uint32_t *eqt)
{

uint32_t x;
uint32_t sum=0,eq=0;
uint8_t v;
              for(x=0;x<height;x++)
              {
                      sum+=*(in+x*width);
              }
              sum=sum/height;
              *avg=sum;
              for(x=0;x<height;x++)
              {
                      v=*(in+x*width)-sum;
                      eq+=v*v;
              }
              eq=eq/(height*height);
              *eqt=eq;
              return 1;
}
//EOF

