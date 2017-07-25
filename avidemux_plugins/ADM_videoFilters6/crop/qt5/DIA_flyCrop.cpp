/***************************************************************************
 * \file GUI for crop filter
 * \author mean 2002/2017 fixounet@free.fr
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
#include "./DIA_flyCrop.h"
#include "./Q_crop.h"
#include "ADM_toolkitQt.h"

uint8_t Metrics( uint8_t *in, uint32_t width,uint32_t *avg, uint32_t *eqt);
uint8_t MetricsV( uint8_t *in, uint32_t width,uint32_t height,uint32_t *avg, uint32_t *eqt);

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
    _ox=0;
    _oy=0;
    _ow=width;
    _oh=height;
}
flyCrop::~flyCrop()
{
    delete rubber;
    rubber=NULL;
}

/**
    \fn blank
    \brief 
*/

static void blank(uint8_t *in, int w,int h,int stride)
{
    for(int y=0;y<h;y++)
    {
        memset(in,0,4*w);
        uint8_t *p=in+1;
        for(int x=0;x<w;x++)        
            p[x<<2]=0xff;        
        in+=stride;
    }  
}

/**
 * \fn processRgb
 * @param imageIn
 * @param imageOut
 * @return 
 */
uint8_t    flyCrop::processRgb(uint8_t *imageIn, uint8_t *imageOut)
{
    memcpy(imageOut,imageIn,_w*_h*4);
    
    blank(imageOut,_w,top,4*_w);
    blank(imageOut+(_w*4)*(_h-bottom),_w,bottom,4*_w);
    blank(imageOut,left,_h,4*_w);
    blank(imageOut+(_w-right-1)*4,right,_h,4*_w);
    return true;
}
/**
 * 
 * @param imageIn
 * @param imageOut
 * @return 
 */
int bound(int val, int other, int maxx)
{
   int r=(int)maxx-(int)(val+other);
   if(r<0) 
        r=0;
   return r;
}
/**
 * \fn bandResized
 * @param x
 * @param y
 * @param w
 * @param h
 * @return 
 */
bool    flyCrop::bandResized(int x,int y,int w, int h)
{
    aprintf("Rubber resize %d x %d, w=%d h=%d\n",x,y,w,h);
    if(x<0) x=0;
    if(y<0) y=0;
    if(x>_w) x=_w;
    if(y>_h) y=_h;

    double halfzoom=_zoom/2-0.01;
    // try to recalculate values only if these values were actually modified by moving the handles
    if(x!=_ox)
        x=(int)(((double)x+halfzoom)/_zoom)&0xfffe;
    if(y!=_oy)
        y=(int)(((double)y+halfzoom)/_zoom)&0xfffe;
    if(w!=_ow)
        w=(int)(((double)w+halfzoom)/_zoom)&0xfffe;
    if(h!=_oh)
        h=(int)(((double)h+halfzoom)/_zoom)&0xfffe;

    _ox=x;
    _oy=y;
    _ow=w;
    _oh=h;

    top=y;
    left=x;
    bottom=bound(y,h,_h);
    right=bound(x,w,_w);

    upload(false);
    sameImage();
    return true; 
}
/**
 * \fn blockChanges
 * @param block
 * @return 
 */
#define APPLY_TO_ALL(x) {w->spinBoxLeft->x;w->spinBoxRight->x;w->spinBoxTop->x;w->spinBoxBottom->x;rubber->x;}
bool flyCrop::blockChanges(bool block)
{
    Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
    APPLY_TO_ALL(blockSignals(block));
    return true;
}
/**
     \fn autoRun
    \brief 
*/
#define THRESH_AVG   30
#define THRESH_EQT   50

int flyCrop::autoRun(uint8_t *in,int w,int h, int increment)
{
    uint32_t avg,eqt;
    int y;
    for(y=0;y<h;y++)	
    {
        Metrics(in,w,&avg,&eqt);
        in+=increment;
        if(avg> THRESH_AVG || eqt > THRESH_EQT)
                break;
    }
    if(y)
        y=y-1;
    return y&0xfffe;
}
int flyCrop::autoRunV(uint8_t *in,int w,int h, int increment)
{
    uint32_t avg,eqt;
    int y;
    for(y=0;y<h;y++)	
    {
        MetricsV(in,w,h,&avg,&eqt);
        in+=increment;
        if(avg> THRESH_AVG || eqt > THRESH_EQT)
                break;
    }
    if(y)
        y=y-1;
    return y&0xfffe;
}
/**
 * 
 * @return 
 */
uint8_t  flyCrop::autocrop(void)
{
uint8_t *in;
uint32_t y,avg,eqt;
    // Top


    in=_yuvBuffer->GetReadPtr(PLANAR_Y);
    int stride=_yuvBuffer->GetPitch(PLANAR_Y);
    
    top=autoRun(in,_w,((_h>>1)-2),stride);
    bottom=autoRun(in+stride*(_h-1),_w,((_h>>1)-2),-stride);
    left=autoRunV(in,_w,((_w>>1)-2),1);
    right=autoRunV(in+_w-1,_w,((_w>>1)-2),-1);
    upload(false);
    sameImage();
    return 1;
}
/**
 * 
 * @param redraw
 * @return 
 */
uint8_t flyCrop::upload(bool redraw)
{
    Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
    if(!redraw)
    {
        blockChanges(true);
    }
    w->spinBoxLeft->setValue(left);
    w->spinBoxRight->setValue(right);
    w->spinBoxTop->setValue(top);
    w->spinBoxBottom->setValue(bottom);

    rubber->nestedIgnore++;
    rubber->move(_zoom*(float)left,_zoom*(float)top);
    rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
    rubber->nestedIgnore--;

    if(!redraw)
    {
         blockChanges(false);
    }
    return 1;
}
/**
 * 
 * @return 
 */
uint8_t flyCrop::download(void)
{
int reject=0;
Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
#define SPIN_GET(x,y) x=w->spinBox##y->value();
    SPIN_GET(left,Left);
    SPIN_GET(right,Right);
    SPIN_GET(top,Top);
    SPIN_GET(bottom,Bottom);

    aprintf("%d %d %d %d\n",left,right,top,bottom);

    left&=0xffffe;
    right&=0xffffe;
    top&=0xffffe;
    bottom&=0xffffe;

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
            upload(false);
    else
    {
        blockChanges(true);
        rubber->nestedIgnore++;
        rubber->move(_zoom*(float)left,_zoom*(float)top);
        rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
        rubber->nestedIgnore--;
        blockChanges(false);
    }
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
    myCrop->rubber->nestedIgnore++; // don't modify crop parameters simply by opening the dialog
    myCrop->left=param->left;
    myCrop->right=param->right;
    myCrop->top=param->top;
    myCrop->bottom=param->bottom;
    myCrop->_cookie=&ui;
    myCrop->addControl(ui.toolboxLayout);
    myCrop->upload(false);
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
    myCrop->rubber->nestedIgnore--;
}
/**
 * 
 * @param foo
 */
void Ui_cropWindow::sliderUpdate(int foo)
{
    myCrop->sliderChanged();
}
/**
 * 
 * @param param
 */
void Ui_cropWindow::gather(crop *param)
{
    myCrop->download();
    param->left=myCrop->left;
    param->right=myCrop->right;
    param->top=myCrop->top;
    param->bottom=myCrop->bottom;
    }
/**
 * 
 */
Ui_cropWindow::~Ui_cropWindow()
{
    if(myCrop) delete myCrop;
    myCrop=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
/**
 * 
 * @param f
 */
void Ui_cropWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myCrop->rubber->nestedIgnore++;
    myCrop->download();
    myCrop->sameImage();
    myCrop->rubber->nestedIgnore--;
    lock--;
}
/**
 * 
 * @param f
 */
void Ui_cropWindow::autoCrop( bool f )
{
    lock++;
    myCrop->autocrop();
    lock--;
}
/**
 * 
 * @param f
 */
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
/**
 * 
 * @param event
 */
void Ui_cropWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myCrop->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myCrop->adjustCanvasPosition();
#if 0
    myCrop->blockChanges(true);
    myCrop->rubber->nestedIgnore++;
    myCrop->rubber->resize(
        (int)((double)(myCrop->_w-myCrop->left-myCrop->right)*myCrop->_zoom/(double)graphicsViewWidth),
        (int)((double)(myCrop->_h-myCrop->top-myCrop->bottom)*myCrop->_zoom/(double)graphicsViewHeight)
    );
    myCrop->rubber->nestedIgnore--;
    myCrop->blockChanges(false);
#endif
}

//EOF

