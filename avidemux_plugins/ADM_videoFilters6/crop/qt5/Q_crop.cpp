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