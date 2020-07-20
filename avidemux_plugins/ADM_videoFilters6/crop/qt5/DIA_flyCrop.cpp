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
                                    ADM_QCanvas *canvas, ADM_QSlider *slider)
                : ADM_flyDialogRgb(parent,width, height,in,canvas, slider,RESIZE_LAST)
{
    rubber=new ADM_rubberControl(this,canvas);
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
 * \fn blank
 * \brief Green bars
 */

static void blank(uint8_t *in, int w, int h, int stride)
{
    for(int y=0;y<h;y++)
    {
        memset(in,0,4*w);
        uint8_t *green=in+1;
        for(int x=0;x<w;x++)
            green[x<<2]=0xff;
        uint8_t *alpha=in+3;
        for(int x=0;x<w;x++)
            alpha[x<<2]=0xff;
        in+=stride;
    }
}

/**
 * \fn processRgb
 * @param imageIn
 * @param imageOut
 * @return 
 */
uint8_t flyCrop::processRgb(uint8_t *imageIn, uint8_t *imageOut)
{
    int stride=ADM_IMAGE_ALIGN(_w*4);
    memcpy(imageOut,imageIn,stride*_h);

    blank(imageOut,_w,top,stride);
    blank(imageOut+stride*(_h-bottom),_w,bottom,stride);
    blank(imageOut,left,_h,stride);
    blank(imageOut+(_w-right)*4,right,_h,stride);
    return true;
}
/**
 * 
 * @param imageIn
 * @param imageOut
 * @return 
 */
static int bound(int val, int other, int maxx)
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
    aprintf("Rubber resized: x=%d, y=%d, w=%d, h=%d\n",x,y,w,h);
    aprintf("Debug: old values: x=%d, y=%d, w=%d, h=%d\n",_ox,_oy,_ow,_oh);

    double halfzoom=_zoom/2-0.01;
    // try to recalculate values only if these values were actually modified by moving the handles
    bool leftHandleMoved=false;
    bool rightHandleMoved=false;
    if((x+w)==(_ox+_ow) && (y+h)==(_oy+_oh))
        leftHandleMoved=true;
    if(x==_ox && y==_oy)
        rightHandleMoved=true;

    _ox=x;
    _oy=y;
    _ow=w;
    _oh=h;

    bool ignore=false;
    if(leftHandleMoved && rightHandleMoved) // bogus event
        ignore=true;

    int normX, normY, normW, normH;
    normX=(int)(((double)x+halfzoom)/_zoom);
    normY=(int)(((double)y+halfzoom)/_zoom);
    normW=(int)(((double)w+halfzoom)/_zoom);
    normH=(int)(((double)h+halfzoom)/_zoom);

    // resize the rubberband back into bounds once the user tries to drag handles out of sight
    bool resizeRubber=false;
    if(normX<0 || normY<0 || normX+normW>_w || normY+normH>_h)
    {
        resizeRubber=true;
        aprintf("rubberband out of bounds, will be resized back\n");
    }

    if(ignore)
    {
        upload(false,resizeRubber);
        return false;
    }

    if(rightHandleMoved)
    {
        right=bound(normX,normW,_w)&0xfffe;
        bottom=bound(normY,normH,_h)&0xfffe;
    }

    if(normX<0) normX=0;
    if(normY<0) normY=0;

    if(leftHandleMoved)
    {
        top=normY&0xfffe;
        left=normX&0xfffe;
    }

    upload(false,resizeRubber);
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
int flyCrop::autoRunV(uint8_t *in, int stride, int w, int increment)
{
    uint32_t avg,eqt;
    int y;
    for(y=0;y<w;y++)
    {
        MetricsV(in,stride,_h,&avg,&eqt);
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
    in=_yuvBuffer->GetReadPtr(PLANAR_Y);
    int stride=_yuvBuffer->GetPitch(PLANAR_Y);

    top=autoRun(in,_w,((_h>>1)-2),stride);
    bottom=autoRun(in+stride*(_h-1),_w,((_h>>1)-2),-stride);
    left=autoRunV(in,stride,((_w>>1)-2),1);
    right=autoRunV(in+_w-1,stride,((_w>>1)-2),-1);
    upload(false,true);
    sameImage();
    return 1;
}
/**
 * 
 * @param redraw
 * @return 
 */
uint8_t flyCrop::upload(bool redraw, bool toRubber)
{
    aprintf("left=%d, right=%d, top=%d, bottom=%d\n",left,right,top,bottom);
    Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
    if(!redraw)
    {
        blockChanges(true);
    }
    w->spinBoxLeft->setValue(left);
    w->spinBoxRight->setValue(right);
    w->spinBoxTop->setValue(top);
    w->spinBoxBottom->setValue(bottom);
    dimensions();

    if(toRubber)
    {
        rubber->nestedIgnore++;
        rubber->move(_zoom*(float)left,_zoom*(float)top);
        rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
        rubber->nestedIgnore--;
    }

    if(!redraw)
    {
         blockChanges(false);
    }
    return 1;
}
/**
 * Read crop values from UI
 * @param even Fixup crop values to make resulting width and height even
 * @return 
 */
uint8_t flyCrop::download(bool even)
{
int reject=0;
Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
#define SPIN_GET(x,y) x=w->spinBox##y->value();
    SPIN_GET(left,Left);
    SPIN_GET(right,Right);
    SPIN_GET(top,Top);
    SPIN_GET(bottom,Bottom);

    aprintf("%d %d %d %d\n",left,right,top,bottom);

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
            upload(false,true);
    else
    {
        blockChanges(true);
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
        rubber->nestedIgnore++;
        rubber->move(_zoom*(float)left,_zoom*(float)top);
        rubber->resize(_zoom*(float)(_w-left-right),_zoom*(float)(_h-top-bottom));
        rubber->nestedIgnore--;
        blockChanges(false);
    }
    dimensions();
    return true;
}

/**
 * \fn dimensions
 * \brief Fill in label displaying video size
 */
void flyCrop::dimensions(void)
{
    Ui_cropDialog *w=(Ui_cropDialog *)_cookie;
    QString dim=QString(QT_TRANSLATE_NOOP("crop","Size: "));
    dim+=QString::number(_w-left-right);
    dim+=QString(" x ");
    dim+=QString::number(_h-top-bottom);
    w->label_5->setText(dim);
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
    myCrop->rubber_is_hidden=param->rubber_is_hidden;
    myCrop->_cookie=&ui;
    myCrop->addControl(ui.toolboxLayout);
    myCrop->upload(false,true);
    myCrop->sliderChanged();
    myCrop->rubber->nestedIgnore=1;

    ui.checkBoxRubber->setChecked(myCrop->rubber_is_hidden);

    connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
    connect( ui.checkBoxRubber,SIGNAL(stateChanged(int)),this,SLOT(toggleRubber(int)));
    connect( ui.pushButtonAutoCrop,SIGNAL(clicked(bool)),this,SLOT(autoCrop(bool)));
    connect( ui.pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
      SPINNER(Left);
      SPINNER(Right);
      SPINNER(Top);
      SPINNER(Bottom);

    setModal(true);
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
    myCrop->download(true);
    param->left=myCrop->left;
    param->right=myCrop->right;
    param->top=myCrop->top;
    param->bottom=myCrop->bottom;
    param->rubber_is_hidden=myCrop->rubber_is_hidden;
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
 * \fn toggleRubber
 */
void Ui_cropWindow::toggleRubber(int checkState)
{
    bool visible=true;
    if(checkState)
        visible=false;
    myCrop->rubber->rubberband->setVisible(visible);
    myCrop->rubber_is_hidden=!visible;
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

    int x=(int)((double)myCrop->left*myCrop->_zoom);
    int y=(int)((double)myCrop->top*myCrop->_zoom);
    int w=(int)((double)(myCrop->_w-(myCrop->left+myCrop->right))*myCrop->_zoom);
    int h=(int)((double)(myCrop->_h-(myCrop->top+myCrop->bottom))*myCrop->_zoom);

    myCrop->blockChanges(true);
    myCrop->rubber->nestedIgnore++;
    myCrop->rubber->move(x,y);
    myCrop->rubber->resize(w,h);
    myCrop->rubber->nestedIgnore--;
    myCrop->blockChanges(false);
}

/**
 * \fn showEvent
 * \brief set canvas size and position
 */
void Ui_cropWindow::showEvent(QShowEvent *event)
{
    myCrop->rubber->rubberband->show(); // must be called first
    myCrop->rubber->rubberband->setVisible(!(myCrop->rubber_is_hidden));
    QDialog::showEvent(event);
    myCrop->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
    myCrop->rubber->nestedIgnore=0;
}

//EOF

