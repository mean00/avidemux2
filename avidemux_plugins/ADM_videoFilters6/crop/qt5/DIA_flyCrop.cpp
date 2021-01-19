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
    left=right=top=bottom=0;
    _ox=0;
    _oy=0;
    _lw=_ow=width;
    _lh=_oh=height;
    //ar = (double)_w / _h;
    setAspectRatioIndex(0);
}
flyCrop::~flyCrop()
{
    delete rubber;
    rubber=NULL;
}
/**
 * \fn setAspectRatioIndex
 */
void flyCrop::setAspectRatioIndex(int index)
{
    ar_select = index;
    switch(index) {
        case 1: ar = ((double)_w / _h);    // source
            break;
        case 2: ar = (64.0/27.0);    // 21:9
            break;
        case 3: ar = (2.0);    // 18:9
            break;
        case 4: ar = (16.0/9.0);    // 16:9
            break;
        case 5: ar = (4.0/3.0);    // 4:3
            break;
        case 6: ar = (1.0);    // 1:1
            break;
        case 7: ar = (9.0/16.0);    // 9:16
            break;
        default : ar = ((double)_lw / _lh);    // current selection
                ar_select = 0;
            break;
    }
}
/**
 * \fn getCropMargins
 */
bool flyCrop::getCropMargins(int *lf, int *rt, int *tp, int *bt)
{
    if(lf) *lf=left;
    if(rt) *rt=right;
    if(tp) *tp=top;
    if(bt) *bt=bottom;
    return true;
}
/**
 * \fn setCropMargins
 * \brief Negative input values will be ignored
 */
void flyCrop::setCropMargins(int lf, int rt, int tp, int bt)
{
    if(lf>=0) left=lf;
    if(rt>=0) right=rt;
    if(tp>=0) top=tp;
    if(bt>=0) bottom=bt;
}
/**
 * \fn initRubber
 * \brief To be called on show event
 */
void flyCrop::initRubber(void)
{
    rubber->rubberband->show(); // must be called first
    rubber->rubberband->setVisible(!rubber_is_hidden);
    rubber->nestedIgnore = 0;
}
/**
 * \fn hideRubber
 */
void flyCrop::hideRubber(bool hide)
{
    rubber_is_hidden = hide;
    rubber->rubberband->setVisible(!hide);
}
/**
 * \fn hideRubberGrips
 */
void flyCrop::hideRubberGrips(bool hideTopLeft, bool hideBottomRight)
{
    rubber->sizeGripEnable(!hideTopLeft, !hideBottomRight);
}
/**
 * \fn adjustRubber
 */
void flyCrop::adjustRubber(int x, int y, int w, int h)
{
    rubber->move(x,y);
    rubber->resize(w,h);
}
/**
 * \fn lockRubber
 */
int flyCrop::lockRubber(bool lock)
{
    int old = rubber->nestedIgnore;
    if(lock)
        rubber->nestedIgnore++;
    else
        rubber->nestedIgnore--;
    return old;
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
 * \fn boundChecked
 */
static int boundChecked(int val, int maxx)
{
    if(val<0) return 0;
    if(val>maxx) return maxx;
    return val;
}
/**
 * \fn recomputeDimensions
 * \brief Calculate width and height of output picture for given aspect ratio
          and top-left corner position, bound-check left and top crop values.
 * @param ar  : Aspect ratio
 * @param inw : Input width
 * @param inh : Input height
 * @param left: Left crop value
 * @param top : Top crop value
 * @param outw: Output width
 * @param outh: Output height
 */
static void recomputeDimensions(const double ar, const int inw, const int inh, int &left, int &top, int &outw, int &outh)
{
    int arW, arH;
    aprintf("Keep aspect ratio: %d/%d == %f\n", inw, inh, ar);

    left = boundChecked(left,inw);
    top  = boundChecked(top,inh);
    outw = boundChecked(outw,inw);
    outh = boundChecked(outh,inh);
    if(!outw || !outh)
        return;

    if((double)outw / outh > ar)
    {
        arW = outw;
        arH = (double)outw / ar + 0.49;
    }else
    {
        arW = (double)outh * ar + 0.49;
        arH = outh;
    }

    if(left + arW > inw)
    {
        arW = inw - left;
        arH = (double)arW / ar + 0.49;
    }
    if (top + arH > inh)
    {
        arH = inh - top;
        arW = (double)arH * ar + 0.49;
    }

    outw = boundChecked(arW,inw);
    outh = boundChecked(arH,inh);
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

    // keep aspect ratio only when dragged on the bottom-right corner
    if (keep_aspect && !ignore && rightHandleMoved)
    {
        recomputeDimensions(ar,_w,_h,normX,normY,normW,normH);
        resizeRubber=true;
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
#define APPLY_TO_ALL(x) {w->spinBoxLeft->x;w->spinBoxRight->x;w->spinBoxTop->x;w->spinBoxBottom->x;rubber->x;\
                         w->checkBoxRubber->x;w->checkBoxKeepAspect->x;w->comboBoxAspectRatio->x;}
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
        w->spinBoxLeft->setMaximum(_w-right-8);
        w->spinBoxRight->setMaximum(_w-left-8);
        w->spinBoxTop->setMaximum(_h-bottom-8);
        w->spinBoxBottom->setMaximum(_h-top-8);
        rubber->nestedIgnore++;
        rubber->move(_zoom*left+0.49,_zoom*top+0.49);
        rubber->resize(_zoom*bound(left,right,_w)+0.49,_zoom*bound(top,bottom,_h)+0.49);
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
    ui.setupUi(this);
    lock=0;
    // Allocate space for green-ised video
    inputWidth = in->getInfo()->width;
    inputHeight = in->getInfo()->height;

    canvas = new ADM_QCanvas(ui.graphicsView, inputWidth, inputHeight);

    myCrop = new flyCrop(this, inputWidth, inputHeight, in, canvas, ui.horizontalSlider);
    myCrop->setCropMargins(param->left, param->right, param->top, param->bottom);
    myCrop->hideRubber(param->rubber_is_hidden);
    myCrop->setKeepAspect(param->keep_aspect);
    myCrop->_cookie=&ui;
    myCrop->addControl(ui.toolboxLayout);

    ui.checkBoxRubber->setChecked(param->rubber_is_hidden);
    ui.checkBoxKeepAspect->setChecked(param->keep_aspect);
    ui.comboBoxAspectRatio->setEnabled(param->keep_aspect);
    ui.comboBoxAspectRatio->setCurrentIndex(param->ar_select);
    myCrop->setAspectRatioIndex(param->ar_select);
    if(param->keep_aspect)
        toggleKeepAspect(true);
    else
        myCrop->upload(false,true);
    myCrop->sliderChanged();
    myCrop->lockRubber(true);

    connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
    connect( ui.checkBoxRubber,SIGNAL(stateChanged(int)),this,SLOT(toggleRubber(int)));
    connect( ui.checkBoxKeepAspect,SIGNAL(stateChanged(int)),this,SLOT(toggleKeepAspect(int)));
    connect( ui.comboBoxAspectRatio,SIGNAL(currentIndexChanged(int)),this,SLOT(changeARSelect(int)));
    connect( ui.pushButtonAutoCrop,SIGNAL(clicked(bool)),this,SLOT(autoCrop(bool)));
    connect( ui.pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));
#define SPINNER(x) connect(ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(widthChanged(int)));
    SPINNER(Left)
    SPINNER(Right)
#undef SPINNER
#define SPINNER(x) connect(ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(heightChanged(int)));
    SPINNER(Top)
    SPINNER(Bottom)

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
    int left,right,top,bottom;
    myCrop->getCropMargins(&left,&right,&top,&bottom);
    param->left = left;
    param->right = right;
    param->top = top;
    param->bottom = bottom;
    param->rubber_is_hidden = myCrop->stateOfRubber();
    param->keep_aspect = myCrop->getKeepAspect();
    param->ar_select = myCrop->getAspectRatioIndex();
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
 * \fn widthChanged
 */
void Ui_cropWindow::widthChanged(int val)
{
    if(lock) return;
    lock++;
    myCrop->lockRubber(true);
    if(myCrop->getKeepAspect())
        updateRightBottomSpinners(val,false);
    myCrop->download();
    myCrop->sameImage();
    myCrop->lockRubber(false);
    lock--;
}
/**
 * \fn heightChanged
 */
void Ui_cropWindow::heightChanged(int val)
{
    if(lock) return;
    lock++;
    myCrop->lockRubber(true);
    if(myCrop->getKeepAspect())
        updateRightBottomSpinners(val,true);
    myCrop->download();
    myCrop->sameImage();
    myCrop->lockRubber(false);
    lock--;
}
/**
 * \fn updateRightBottomSpinners
 */
void Ui_cropWindow::updateRightBottomSpinners(int val, bool useHeightAsRef)
{
    const double ar = myCrop->getAspectRatio();
    int left,top;
    myCrop->getCropMargins(&left,NULL,&top,NULL);
    myCrop->blockChanges(true);

    if(useHeightAsRef)
    {
        int h = boundChecked(inputHeight - top - val, inputHeight);
        int w = (double)h * ar + 0.49;
        w = boundChecked(inputWidth - w - left, inputWidth);

        ui.spinBoxRight->setValue(w);
    }else
    {
        int w = boundChecked(inputWidth - left - val, inputWidth);
        int h = (double)w / ar + 0.49;
        h = boundChecked(inputHeight - h - top, inputHeight);

        ui.spinBoxBottom->setValue(h);
    }
    myCrop->blockChanges(false);
}
/**
 * \fn toggleRubber
 */
void Ui_cropWindow::toggleRubber(int checkState)
{
    bool visible=true;
    if(checkState)
        visible=false;
    myCrop->hideRubber(!visible);
}
/**
 * \fn applyAspectRatio
 */
void Ui_cropWindow::applyAspectRatio(void) {
    if(!lock)
    {
        lock++;
        int left,right,top,bottom;
        myCrop->getCropMargins(&left,&right,&top,&bottom);
        int wout = inputWidth - left - right;
        int hout = inputHeight - top - bottom;
        recomputeDimensions(myCrop->getAspectRatio(),inputWidth,inputHeight,left,top,wout,hout);

        right = boundChecked(inputWidth - wout - left, inputWidth);
        bottom = boundChecked(inputHeight - hout - top, inputHeight);
        myCrop->setCropMargins(left,right,top,bottom);
        myCrop->upload(true,true);

        myCrop->lockRubber(true);
        myCrop->download();
        myCrop->sameImage();
        myCrop->lockRubber(false);
        lock--;
    }
}
/**
 * \fn toggleKeepAspect
 */
void Ui_cropWindow::toggleKeepAspect(int checkState)
{
    bool keep_aspect=false;
    if(checkState)
    {
        keep_aspect=true;
        myCrop->lockDimensions();
        myCrop->setAspectRatioIndex(myCrop->getAspectRatioIndex());
        applyAspectRatio();
    }
    ui.spinBoxLeft->setEnabled(!keep_aspect);
    ui.spinBoxTop->setEnabled(!keep_aspect);
    ui.pushButtonAutoCrop->setEnabled(!keep_aspect);
    ui.comboBoxAspectRatio->setEnabled(keep_aspect);
    myCrop->hideRubberGrips(keep_aspect,false);
    myCrop->setKeepAspect(keep_aspect);
}
/**
 * \fn changeARSelect
 */
void Ui_cropWindow::changeARSelect(int f)
{
    myCrop->lockDimensions();
    myCrop->setAspectRatioIndex(f);
    applyAspectRatio();
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
    lock++;
    myCrop->blockChanges(true);
    ui.checkBoxKeepAspect->setChecked(false);
    ui.comboBoxAspectRatio->setCurrentIndex(0);
    toggleKeepAspect(false);
    myCrop->setCropMargins(0,0,0,0);
    myCrop->lockDimensions();
    myCrop->setAspectRatioIndex(0);
    myCrop->blockChanges(false);
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

    int left,right,top,bottom;
    myCrop->getCropMargins(&left,&right,&top,&bottom);

    float z = myCrop->getZoomValue();
    int x = (double)left * z + 0.49;
    int y = (double)top * z + 0.49;
    int w = (double)(inputWidth - (left + right)) * z + 0.49;
    int h = (double)(inputHeight - (top + bottom)) * z + 0.49;

    myCrop->blockChanges(true);
    myCrop->lockRubber(true);
    myCrop->adjustRubber(x,y,w,h);
    myCrop->lockRubber(false);
    myCrop->blockChanges(false);
}

/**
 * \fn showEvent
 * \brief set canvas size and position
 */
void Ui_cropWindow::showEvent(QShowEvent *event)
{
    myCrop->initRubber();
    QDialog::showEvent(event);
    myCrop->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
}

//EOF

