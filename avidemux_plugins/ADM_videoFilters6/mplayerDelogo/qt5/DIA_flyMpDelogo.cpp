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
                                    ADM_QCanvas *canvas, ADM_QSlider *slider) :
                ADM_flyDialogYuv(parent,width, height,in,canvas, slider,RESIZE_AUTO) 
 {
    rubber=new ADM_rubberControl(this,canvas);
    rubber->resize(width,height);
    _ox=0;
    _oy=0;
    _ow=width/2;
    _oh=height/2;
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
    bool leftGripMoved=false;
    bool rightGripMoved=false;
    if((x+w)==(_ox+_ow) && (y+h)==(_oy+_oh))
        leftGripMoved=true;
    if(x==_ox && y==_oy)
        rightGripMoved=true;

    _ox=x;
    _oy=y;
    _ow=w;
    _oh=h;

    bool ignore=false;
    if(leftGripMoved && rightGripMoved) // bogus event
        ignore=true;

    int nw,nh,nx,ny;
    double halfzoom=_zoom/2-0.01;
    nw=(int)(((double)w-halfzoom)/_zoom);
    nh=(int)(((double)h-halfzoom)/_zoom);
    nx=(int)(((double)x+halfzoom)/_zoom);
    ny=(int)(((double)y+halfzoom)/_zoom);

    aprintf("%d x %d => %d x %d, normalized offsets nx=%d, ny=%d, zoom=%f\n",param.lw,param.lh,nw,nh,nx,ny,_zoom);
    bool resizeRubber=false;
    if(nx<0 || ny<0 || nx+nw>_w || ny+nh>_h)
        resizeRubber=true;

    if(ignore)
    {
        upload(false,resizeRubber);
        return false;
    }

    uint32_t right=param.xoff+param.lw;
    uint32_t bottom=param.yoff+param.lh;

    if(nx+nw>_w)
        nw=_w-nx;
    if(ny+nh>_h)
        nh=_h-ny;

    if(nx<0) nx=0;
    if(ny<0) ny=0;

    if(leftGripMoved)
    {
        param.xoff=nx;
        param.yoff=ny;
        param.lw=right-nx;
        param.lh=bottom-ny;
    }

    if(rightGripMoved)
    {
        param.lw=nw;
        param.lh=nh;
    }

    upload(false,resizeRubber);
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
        rubber->nestedIgnore++;
        blockChanges(true);
        rubber->move(_zoom*(float)param.xoff,_zoom*(float)param.yoff);
        rubber->resize(_zoom*(float)param.lw,_zoom*(float)param.lh);
        blockChanges(false);
        rubber->nestedIgnore--;
    }
    return 1;
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

        aprintf("Uploading\n");
        myCrop->upload();
        myCrop->sliderChanged();
        myCrop->rubber->nestedIgnore=1;

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

        setModal(true);
  }

/**
    \fn resizeEvent
*/
void Ui_mpdelogoWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myCrop->rubber->nestedIgnore++;
    myCrop->blockChanges(true);
    myCrop->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myCrop->adjustCanvasPosition();
    myCrop->blockChanges(false);
    myCrop->rubber->nestedIgnore--;
}

/**
    \fn showEvent
*/
void Ui_mpdelogoWindow::showEvent(QShowEvent *event)
{
    myCrop->rubber->rubberband->show(); // must be called first
    QDialog::showEvent(event);
    myCrop->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
    myCrop->rubber->nestedIgnore=0;
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

uint8_t flyMpDelogo::upload(bool redraw, bool toRubber)
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

    if(toRubber)
    {
        rubber->nestedIgnore++;
        rubber->resize((float)(param.lw)*_zoom,(float)(param.lh)*_zoom);
        rubber->move((float)(param.xoff)*_zoom,(float)(param.yoff)*_zoom);
        rubber->nestedIgnore--;
    }

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
        printf(">>>Download event : %d x %d , %d x %d\n",param.xoff,param.yoff,param.lw,param.lh);
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

//EOF
