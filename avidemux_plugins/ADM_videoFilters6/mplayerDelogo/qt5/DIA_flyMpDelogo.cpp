/**/
/***************************************************************************
                          DIA_flyMpDelogo
                             -------------------

                           Ui for MPlayer DeLogo filter

    begin                : 08 Apr 2005
    copyright            : (C) 2004/5 by mean
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
#include "QRubberBand"
#include "QSizeGrip"
#include "QHBoxLayout"
#include "ADM_default.h"
#include "ADM_image.h"


#include "delogo.h"
#include "DIA_flyMpDelogo.h"
#include "ADM_vidMPdelogo.h"
#include "Q_mpdelogo.h"
/**
 * 
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
 flyMpDelogo::flyMpDelogo (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, QSlider *slider) : 
                ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) 
 {
    rubber=new Resizable_rubber_band(this,canvas);
    
 }
 /**
  * 
  */
flyMpDelogo::~flyMpDelogo()
{
    if(rubber)
    {
        delete rubber;
        rubber=NULL;
    }
}
/**
 * \fn bandResized
 * @param w
 * @param h
 * @return 
 */
bool    flyMpDelogo::bandResized(int x,int y,int w, int h)
{
    param.lw=(double)w/_zoom;
    param.lh=(double)h/_zoom;
    param.xoff=(double)x/_zoom;
    param.yoff=(double)y/_zoom;
    upload();
    return true;
}


/************* COMMON PART *********************/
/**
    \fn process
*/
uint8_t    flyMpDelogo::processYuv(ADMImage* in, ADMImage *out)
{
    out->duplicate(in);
    if(preview)
        MPDelogo::doDelogo(out, param.xoff, param.yoff,
                             param.lw,  param.lh,param.band,param.show);        
    else
    {
#if 0              
        uint8_t *y=out->GetWritePtr(PLANAR_Y);
        int stride=out->GetPitch(PLANAR_Y);
        int mx=param.lw+param.xoff;
        int my=param.lh+param.yoff;
        if(mx>=out->GetWidth(PLANAR_Y)) mx=out->GetWidth(PLANAR_Y)-1;
        if(my>=out->GetHeight(PLANAR_Y)) my=out->GetHeight(PLANAR_Y)-1;
        
        
        // hz
        uint8_t *p=y+stride*param.yoff;
        uint8_t *p2=y+stride*(my);
        
        uint8_t toggle=0;
        for(int x=param.xoff;x<mx;x++)
        {
            *(p+x)=toggle;
            toggle=0xff^toggle;
            *(p2+x)=toggle;
        }
        // vz
         p=y+stride*(param.yoff)+param.xoff;
         p2=y+stride*(param.yoff)+mx;
         
         for(int yy=param.yoff;yy<my;yy++)
         {
            *(p)=toggle;
            toggle=0xff^toggle;
            *(p2)=toggle;
            p+=stride;   p2+=stride;
         }
#endif         
         rubber->move(param.xoff,param.yoff);
         rubber->resize(param.lw,param.lh);
    }
    return 1;
}
/**
        \fn Ctor
*/
Resizable_rubber_band::Resizable_rubber_band(flyMpDelogo *fly,QWidget *parent) : QWidget(parent) 
{
  flyParent=fly;
  //tell QSizeGrip to resize this widget instead of top-level window
  setWindowFlags(Qt::SubWindow);
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  QSizeGrip* grip1 = new QSizeGrip(this);
  QSizeGrip* grip2 = new QSizeGrip(this);
  layout->addWidget(grip1, 0, Qt::AlignLeft | Qt::AlignTop);
  layout->addWidget(grip2, 0, Qt::AlignRight | Qt::AlignBottom);
  rubberband = new QRubberBand(QRubberBand::Rectangle, this);
  rubberband->move(0, 0);
  rubberband->show();
  show();
}
/**
        \fn resizeEvent
*/
void Resizable_rubber_band::resizeEvent(QResizeEvent *) 
{
  rubberband->resize(size());
  flyParent->bandResized(pos().x(),pos().y(),size().width(),size().height());
}

//EOF
