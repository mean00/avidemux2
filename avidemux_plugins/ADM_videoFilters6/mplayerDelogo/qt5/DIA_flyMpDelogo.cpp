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
#include "ADM_toolkitQt.h"


#if 0
#define aprintf ADM_info
#else
#define aprintf(...) {}
#endif


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
    rubber->resize(width,height);
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
    aprintf("BandResized\n");        
    int nw,nh,nx,ny;
    
    delogo newParam;
    
    double halfzoom=_zoom/2-0.01;
    
    newParam.lw=((double)w+halfzoom)/_zoom;
    newParam.lh=((double)h+halfzoom)/_zoom;
    newParam.xoff=((double)x+halfzoom)/_zoom;
    newParam.yoff=((double)y+halfzoom)/_zoom;
    
    aprintf("%d x %d => %d x %d, %f\n",param.lw,param.lh,newParam.lw,newParam.lh,_zoom);
    
    param.lw=newParam.lw;
    param.lh=newParam.lh;
    param.xoff=newParam.xoff;
    param.yoff=newParam.yoff;
    
    upload(false);
    //
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
        rubber->move(_zoom*(float)param.xoff,_zoom*(float)param.yoff);
        rubber->resize(_zoom*(float)param.lw,_zoom*(float)param.lh);
    }
    return 1;
}
/**
        \fn Ctor
*/
Resizable_rubber_band::Resizable_rubber_band(flyMpDelogo *fly,QWidget *parent) : QWidget(parent) 
{
  nestedIgnore=0;
  flyParent=fly;
  //tell QSizeGrip to resize this widget instead of top-level window
  setWindowFlags(Qt::SubWindow);
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  QSizeGrip* grip1 = new QSizeGrip(this);
  QSizeGrip* grip2 = new QSizeGrip(this);
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
void Resizable_rubber_band::resizeEvent(QResizeEvent *) 
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
    \fn ctor
*/

  Ui_mpdelogoWindow::Ui_mpdelogoWindow(QWidget *parent,  delogo *param, ADM_coreVideoFilter *in) 
            : QDialog(parent)
  {
    static bool doOnce=false;
    uint32_t width,height;
        
        aprintf("Ctor @ %d: %d, %d x %d\n",param->xoff, param->yoff, param->lw,param->lh);
        
        ui.setupUi(this);
        _in=in;
        
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);        
        myCrop=new flyMpDelogo(this, width, height,in,canvas,ui.horizontalSlider);
        myCrop->param=*param;
        myCrop->_cookie=&ui;
        myCrop->addControl(ui.toolboxLayout);
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
        aprintf("Uploading\n");
        myCrop->upload();
        myCrop->sliderChanged();
        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
        SPINNER(spinX);
        SPINNER(spinY);
        SPINNER(spinW);
        SPINNER(spinH);
        SPINNER(spinBand);

        connect(ui.checkBoxPreview, SIGNAL(stateChanged(int )),this, SLOT(preview(int)));
        
        if(!doOnce)
        {
            Q_INIT_RESOURCE(delogo);
            doOnce=true;
        }
        ui.labelHelp->setPixmap(QPixmap(":/images/grips.png"));
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
      printf("setXy\n");
      param.xoff= x;
      param.yoff= y;
      upload(false);
      return true;
}

#define MYSPIN(x) w->x
//************************
/**
    \fn upload
*/

#define APPLY_TO_ALL(x) {w->spinX->x;w->spinY->x;w->spinW->x;w->spinH->x;w->spinBand->x;}

bool flyMpDelogo::blockChanges(bool block)
{
     Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
     APPLY_TO_ALL(blockSignals(block));
     rubber->blockSignals(block);
     return true;
}

uint8_t flyMpDelogo::upload(bool redraw)
{
    Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
    if(!redraw)
    {
        blockChanges(true);
    }
    printf(">>>Upload event : %d x %d , %d x %d\n",param.xoff,param.yoff,param.lw,param.lh);

    MYSPIN(spinX)->setValue(param.xoff);
    MYSPIN(spinY)->setValue(param.yoff);
    MYSPIN(spinW)->setValue(param.lw);
    MYSPIN(spinH)->setValue(param.lh);   
    MYSPIN(spinBand)->setValue(param.band);   

    if(!redraw)
    {
         blockChanges(false);
    }

        
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
        rubber->nestedIgnore++;
        blockChanges(true);
        rubber->resize(_zoom*(float)param.lw,_zoom*(float)param.lh);
        blockChanges(false);
        rubber->nestedIgnore--;
        printf(">>>Download event : %d x %d , %d x %d\n",param.xoff,param.yoff,param.lw,param.lh);
        printf("Download\n");
        return true;
}

/**
    \fn autoZoom
*/
void flyMpDelogo::autoZoom(bool state)
{
    rubber->nestedIgnore++;
    blockChanges(true);
    ADM_flyDialog::autoZoom(state);
    blockChanges(false);
    rubber->nestedIgnore--;
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

//EOF
