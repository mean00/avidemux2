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
 * \fn ctor
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
flyMpDelogo::flyMpDelogo(
    QDialog *parent,
    uint32_t width,
    uint32_t height,
    ADM_coreVideoFilter *in,
    ADM_QCanvas *canvas,
    ADM_QSlider *slider) : ADM_flyDialogYuv(parent,width,height,in,canvas,slider,RESIZE_AUTO)
{
    rubber=new ADM_rubberControl(this,canvas);
    rubber->resize(width,height);
    _ox=0;
    _oy=0;
    _ow=width/2;
    _oh=height/2;
}
/**
 * \fn dtor
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
 * \fn setParam
 */
void flyMpDelogo::setParam(delogo *ps)
{
    if(!ps) return;
#define CPY(x) param.x = ps->x;
    CPY(xoff)
    CPY(yoff)
    CPY(lw)
    CPY(lh)
    CPY(band)
    CPY(show)
#undef CPY
}
/**
 * \fn initRubber
 * \brief To be called on show event
 */
void flyMpDelogo::initRubber(void)
{
    rubber->rubberband->show(); // must be called first
    rubber->nestedIgnore = 0;
}
/**
 * \fn adjustRubber
 */
void flyMpDelogo::adjustRubber(void)
{
    rubber->nestedIgnore++;
    blockChanges(true);
    rubber->move(_zoom * param.xoff + 0.49, _zoom * param.yoff + 0.49);
    rubber->resize(_zoom * param.lw + 0.49, _zoom * param.lh + 0.49);
    blockChanges(false);
    rubber->nestedIgnore--;
}
/**
 * \fn      lockRubber
 * \brief   Inhibit resize event callback
 */
int flyMpDelogo::lockRubber(bool lock)
{
    int old = rubber->nestedIgnore;
    if(lock)
        rubber->nestedIgnore++;
    else
        rubber->nestedIgnore--;
    return old;
}
/**
 * \fn      bandResized
 * \brief   Handle resize event
 *
 * \details Identify which grip was used to resize the rubber band
 *          by comparing to its previous position and size, resize
 *          the rubber band back into bounds if necessary, calculate
 *          delogo params from its bound-checked positon and size,
 *          update spinboxes.
 *
 * @param x The new X axis offset relative to parent
 * @param y The new Y axis offset
 * @param w The new width of the rubber band
 * @param h The new height
 */
bool flyMpDelogo::bandResized(int x, int y, int w, int h)
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

    bool resizeRubber=false;
    if(x<0)
    {
        w+=x;
        x=0;
        resizeRubber=true;
    }
    if(y<0)
    {
        h+=y;
        y=0;
        resizeRubber=true;
    }
    if(resizeRubber)
    {
        rubber->nestedIgnore++;
        rubber->move(x,y);
        rubber->resize(w,h);
        rubber->nestedIgnore--;
    }

    int nw = (double)w/_zoom + 0.49;
    int nh = (double)h/_zoom + 0.49;
    int nx = (double)x/_zoom + 0.49;
    int ny = (double)y/_zoom + 0.49;

    aprintf("%d x %d => %d x %d, normalized offsets nx=%d, ny=%d, zoom=%f\n",param.lw,param.lh,nw,nh,nx,ny,_zoom);

    resizeRubber=false;
    if(nx<0 || ny<0 || nx+nw>_w || ny+nh>_h)
        resizeRubber=true;

    uint32_t right=param.xoff+param.lw;
    uint32_t bottom=param.yoff+param.lh;

    if(nx<0)
    {
        nw+=nx;
        nx=0;
    }
    if(ny<0)
    {
        nh+=ny;
        ny=0;
    }
    if(nx+nw>_w)
        nw=_w-nx;
    if(ny+nh>_h)
        nh=_h-ny;

    if(leftGripMoved)
    {
        param.xoff=nx;
        param.yoff=ny;
        param.lw=right-nx;
        param.lh=bottom-ny;
    }else if(rightGripMoved)
    {
        param.lw=nw;
        param.lh=nh;
    }else
    {
        param.xoff=nx;
        param.yoff=ny;
        param.lw=right-nx;
        param.lh=bottom-ny;
    }

    upload(false,resizeRubber);
    //
    return true;
}
/**
 * \fn bandMoved
 * @param x
 * @param y
 * @param w
 * @param h
 * @return 
 */
bool    flyMpDelogo::bandMoved(int x,int y,int w, int h)
{
    _ox = x;
    _oy = y;
    _ow = w;
    _oh = h;

    int nx = (double)x/_zoom + 0.49;
    int ny = (double)y/_zoom + 0.49;

    // bound checks are done in rubber control    bool resizeRubber=false;

    if(nx<0) nx=0;
    if(ny<0) ny=0;

    if(nx+param.lw>_w)
        nx=_w-param.lw;
    if(ny+param.lh>_h)
        ny=_h-param.lh;

    param.xoff=nx;
    param.yoff=ny;

    upload(false,false);
    //
    return true;
}
/**
 *  \fn blockChanges
 *  \brief Break signal/slot loops
 */
bool flyMpDelogo::blockChanges(bool block)
{
     Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
#define APPLY_TO_ALL(x) {w->spinX->x;w->spinY->x;w->spinW->x;w->spinH->x;w->spinBand->x;rubber->x;}
     APPLY_TO_ALL(blockSignals(block));
     return true;
}
/**
 *  \fn boundCheck
 */
bool flyMpDelogo::boundCheck(bool sizeHasPriority)
{
    Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
    bool passed = true;
    if(param.lw > _w)
    {
        param.lw = _w;
        passed = false;
    }
    if(param.xoff > _w)
    {
        param.xoff = _w;
        passed = false;
    }
    if(param.xoff + param.lw > _w)
    {
        if(sizeHasPriority)
            param.xoff = _w - param.lw;
        else
            param.lw = _w - param.xoff;
        passed = false;
    }
    if(param.lh > _h)
    {
        param.lh = _h;
        passed = false;
    }
    if(param.yoff > _h)
    {
        param.yoff = _h;
        passed = false;
    }
    if(param.yoff + param.lh > _h)
    {
        if(sizeHasPriority)
            param.yoff = _h - param.lh;
        else
            param.lh = _h - param.yoff;
        passed = false;
    }
    /* Setting maximum dynamically breaks auto-repeat of
       spinbox buttons. Do not uncomment the lines below. */
    //w->spinX->setMaximum(_w - param.lw);
    //w->spinY->setMaximum(_h - param.lh);

    return passed;
}
/**
 *  \fn upload
 */
uint8_t flyMpDelogo::upload(bool redraw, bool toRubber)
{
    Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
    if(!redraw)
    {
        blockChanges(true);
    }
    //printf(">>>Upload event : x = %d y = %d w = %d h = %d\n",param.xoff,param.yoff,param.lw,param.lh);
#define SETSPIN(x,y) w->spin##x->setValue(param.y);
    SETSPIN(X,xoff)
    SETSPIN(Y,yoff)
    SETSPIN(W,lw)
    SETSPIN(H,lh)
    SETSPIN(Band,band)

    if(toRubber)
        adjustRubber();
    if(!redraw)
    {
         blockChanges(false);
    }

    return 1;
}
/**
 *  \fn download
 */
uint8_t flyMpDelogo::download(bool sizeHasPriority)
{
    Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
#define GETSPIN(x,y) param.y=w->spin##x->value();
    GETSPIN(X,xoff)
    GETSPIN(Y,yoff)
    GETSPIN(W,lw)
    GETSPIN(H,lh)
    GETSPIN(Band,band)

    if(!boundCheck(sizeHasPriority))
        upload(false,true);
    //printf(">>>Download event : x = %d y = %d w = %d h = %d\n",param.xoff,param.yoff,param.lw,param.lh);
    return 1;
}
/**
 *  \fn setTabOrder
 */
void flyMpDelogo::setTabOrder(void)
{
    Ui_mpdelogoDialog *w=(Ui_mpdelogoDialog *)_cookie;
    std::vector<QWidget *> controls;

#define SPINNER(x) controls.push_back(w->spin##x);
    SPINNER(X)
    SPINNER(Y)
    SPINNER(W)
    SPINNER(H)
    SPINNER(Band)
#undef SPINNER
    controls.insert(controls.end(), buttonList.begin(), buttonList.end());
    controls.push_back(w->horizontalSlider);
    controls.push_back(w->checkBoxPreview);

    QWidget *first, *second;

    for(std::vector<QWidget *>::iterator tor = controls.begin(); tor != controls.end(); ++tor)
    {
        if(tor+1 == controls.end()) break;
        first = *tor;
        second = *(tor+1);
        _parent->setTabOrder(first,second);
        //ADM_info("Tab order: %p (%s) --> %p (%s)\n",first,first->objectName().toUtf8().constData(),second,second->objectName().toUtf8().constData());
    }
}

/************* COMMON PART *********************/
/**
 * \fn      processYuv
 * \brief   Update preview
 */
uint8_t    flyMpDelogo::processYuv(ADMImage* in, ADMImage *out)
{
    out->duplicate(in);
    if(preview)
        MPDelogo::doDelogo(out, param.xoff, param.yoff, param.lw, param.lh, param.band, param.show);
    else
        adjustRubber();
    return 1;
}

/************* Dialog window *******************/
/**
 *  \fn ctor
 */
Ui_mpdelogoWindow::Ui_mpdelogoWindow(QWidget *parent, delogo *param, ADM_coreVideoFilter *in) : QDialog(parent)
{
#ifdef USAGE_HINT
    static bool doOnce=false;
#endif
    uint32_t width,height;

    aprintf("Ctor @ %d: %d, %d x %d\n",param->xoff, param->yoff, param->lw,param->lh);
    ui.setupUi(this);
    _in=in;
    lock=0;
    // Allocate space for green-ised video
    width=in->getInfo()->width;
    height=in->getInfo()->height;

    canvas = new ADM_QCanvas(ui.graphicsView,width,height);
    myCrop = new flyMpDelogo(this,width,height,in,canvas,ui.horizontalSlider);
    myCrop->setParam(param);
    myCrop->_cookie=&ui;
    myCrop->addControl(ui.toolboxLayout);
    myCrop->setTabOrder();
    myCrop->setPreview(false);
#define SPINENTRY(x,y) ui.spin##x->setMaximum(y); ui.spin##x->setKeyboardTracking(false);
    SPINENTRY(X,width)
    SPINENTRY(W,width)
    SPINENTRY(Y,height)
    SPINENTRY(H,height)

    setSpinWidth(width,height);

    aprintf("Uploading\n");
    myCrop->upload();
    myCrop->sliderChanged();
    myCrop->lockRubber(true);

    connect(ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect(ui.spin##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));
    SPINNER(Band)
    SPINNER(X)
    SPINNER(Y)
#undef SPINNER
#define SPINNER(x) connect(ui.spin##x,SIGNAL(valueChanged(int)),this,SLOT(sizeChanged(int)));
    SPINNER(W)
    SPINNER(H)

    connect(ui.checkBoxPreview, SIGNAL(stateChanged(int )),this, SLOT(preview(int)));

    helpLayout=NULL;
#ifdef USAGE_HINT
    if(!doOnce)
    {
        Q_INIT_RESOURCE(delogo);
        doOnce=true;
    }
    helpLayout = new QHBoxLayout(); // not a mistake, we cannot use the dialog as parent
    ui.main->insertLayout(1,helpLayout);
    QLabel *labelHelpText = new QLabel(QT_TRANSLATE_NOOP("mpdelogoDialog","You can resize the red rectangle using the grips "),this);
    QLabel *labelHelpPic = new QLabel(NULL,this);
    labelHelpPic->setPixmap(QPixmap(":/images/grips.png"));
    helpLayout->addWidget(labelHelpText);
    helpLayout->addWidget(labelHelpPic);
    helpLayout->addStretch();
#endif
    setModal(true);
}
/**
 *  \fn dtor
 */
Ui_mpdelogoWindow::~Ui_mpdelogoWindow()
{
    if(myCrop) delete myCrop;
    myCrop=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
    if(helpLayout) delete helpLayout;
    helpLayout=NULL;
}
/**
 *  \fn resizeEvent
 */
void Ui_mpdelogoWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myCrop->lockRubber(true);
    myCrop->blockChanges(true);
    myCrop->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myCrop->adjustCanvasPosition();
    myCrop->blockChanges(false);
    myCrop->lockRubber(false);
}

/**
 *  \fn setSpinWidth
 *  \brief Try to avoid that spinboxes change width on setMaximum()
 */
void Ui_mpdelogoWindow::setSpinWidth(int inputWidth, int inputHeight)
{ // not needed when we don't set maximum dynamically
#if 0
    QString text;
    int higher = (inputWidth > inputHeight)? inputWidth : inputHeight;
    int maxpos = 1; // an extra one to account for buttons, not really correct
    while(higher)
    {
        higher/=10;
        maxpos++;
    }
    if(maxpos < 3) maxpos = 3;
    while(maxpos-- > 0)
    {
        text+=QString("4"); // usually the widest digit with variable-width fonts
    }
    QFontMetrics fm = ui.spinX->fontMetrics();
    int required = fm.boundingRect(text).width() + 20; // add some padding out of precaution
#define SETME(x) ui.spin##x->setMinimumWidth(required);
    SETME(X)
    SETME(Y)
    SETME(W)
    SETME(H)
#endif
}
/**
 *  \fn showEvent
 */
void Ui_mpdelogoWindow::showEvent(QShowEvent *event)
{
    myCrop->initRubber();
    QDialog::showEvent(event);
    myCrop->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
}
/**
 *  \fn sliderUpdate
 */
void Ui_mpdelogoWindow::sliderUpdate(int foo)
{
    myCrop->sliderChanged();
}
/**
 *  \fn gather
 */
void Ui_mpdelogoWindow::gather(delogo *param)
{
    myCrop->download();
    if(param)
        memcpy(param, myCrop->getParam(), sizeof(delogo));
}
/**
 *  \fn sizeChanged
 */
void Ui_mpdelogoWindow::sizeChanged(int f)
{
    if(lock) return;
    lock++;
    myCrop->download(true);
    myCrop->sameImage();
    lock--;
}
/**
 *  \fn valueChanged
 */
void Ui_mpdelogoWindow::valueChanged(int f)
{
    if(lock) return;
    lock++;
    myCrop->download(false);
    myCrop->sameImage();
    lock--;
}
/**
 * \fn      preview
 * \brief   Toggle preview
 * @param x
 */
void Ui_mpdelogoWindow::preview(int x)
{
    aprintf("Preview = %d\n",x);
    if(x==Qt::Checked)
    {
        myCrop->setPreview(true);
        myCrop->sameImage();
    }else
    {
        myCrop->setPreview(false);
        myCrop->sameImage();
    }
}

/**
 * \fn      DIA_getMpDelogo
 * \brief   Handle delogo dialog
 */
bool DIA_getMpDelogo(delogo *param, ADM_coreVideoFilter *in)
{
    bool ret=false;
    Ui_mpdelogoWindow dialog(qtLastRegisteredDialog(),param,in);
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
