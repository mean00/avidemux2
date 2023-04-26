/***************************************************************************
 * \file GUI for zoom filter
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
#include <QGroupBox>

#include <cmath>
#include "DIA_flyDialogQt4.h"
#include "ADM_default.h"
#include "ADM_image.h"
#include "./DIA_flyZoom.h"
#include "./Q_zoom.h"
#include "ADM_toolkitQt.h"
#include "ADM_QSettings.h"
#include "DIA_factory.h"

/**
 * 
 * @param parent
 * @param width
 * @param height
 * @param in
 * @param canvas
 * @param slider
 */
flyZoom::flyZoom (QDialog *parent,uint32_t width,uint32_t height,ADM_coreVideoFilter *in,
                                    ADM_QCanvas *canvas, ADM_flyNavSlider *slider)
                : ADM_flyDialogRgb(parent,width, height,in,canvas, slider,RESIZE_LAST)
{
    rubber=new ADM_rubberControl(this,canvas);
    left=right=top=bottom=0;
    _ox=0;
    _oy=0;
    _lw=_ow=width;
    _lh=_oh=height;
    ar = (double)width / height;
    // Remove event filter, we need to handle show and resize events in a different way.
    clearEventFilter();
}
flyZoom::~flyZoom()
{
    delete rubber;
    rubber=NULL;
}
/**
 * \fn setAspectRatioIndex
 */
void flyZoom::setAspectRatioIndex(int index)
{
    ar_select = index;
    switch(index) {
        case 1: ar = ((double)_lw / _lh);    // current selection
            break;
        case 2: ar = ((double)_w / _h);    // source
            break;
        case 3: ar = (64.0/27.0);    // 21:9
            break;
        case 4: ar = (2.0);    // 18:9
            break;
        case 5: ar = (16.0/9.0);    // 16:9
            break;
        case 6: ar = (4.0/3.0);    // 4:3
            break;
        case 7: ar = (1.0);    // 1:1
            break;
        case 8: ar = (9.0/16.0);    // 9:16
            break;
        default:ar_select = 0;
            break;
    }
}
/**
 * \fn getZoomMargins
 */
bool flyZoom::getZoomMargins(int *lf, int *rt, int *tp, int *bt)
{
    if(lf) *lf=left;
    if(rt) *rt=right;
    if(tp) *tp=top;
    if(bt) *bt=bottom;
    return true;
}
/**
 * \fn setZoomMargins
 * \brief Negative input values will be ignored
 */
void flyZoom::setZoomMargins(int lf, int rt, int tp, int bt)
{
    if(lf>=0) left=lf;
    if(rt>=0) right=rt;
    if(tp>=0) top=tp;
    if(bt>=0) bottom=bt;
}
/**
 * \fn hideRubber
 */
void flyZoom::hideRubber(bool hide)
{
    rubber_is_hidden = hide;
    rubber->rubberband->setVisible(!hide);
}
/**
 * \fn hideRubberGrips
 */
void flyZoom::hideRubberGrips(bool hideTopLeft, bool hideBottomRight)
{
    rubber->sizeGripEnable(!hideTopLeft, !hideBottomRight);
}
/**
 * \fn adjustRubber
 */
void flyZoom::adjustRubber(int x, int y, int w, int h)
{
    rubber->move(x,y);
    rubber->resize(w,h);
}
/**
 * \fn lockRubber
 */
int flyZoom::lockRubber(bool lock)
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
    uint32_t * tmp;
    for(int y=0;y<h;y++)
    {
        tmp = (uint32_t*)in;
        for(int x=0;x<w;x++)
        {
            *tmp >>= 2;
            *tmp &= 0xFF3F3F3F;
            *tmp |= 0xFF000000;
            *tmp += (192 << 8);
            tmp += 1;
        }
        in+=stride;
    }
}

/**
 * \fn processRgb
 * @param imageIn
 * @param imageOut
 * @return 
 */
uint8_t flyZoom::processRgb(uint8_t *imageIn, uint8_t *imageOut)
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
          and top-left corner position, bound-check left and top zoom values.
 * @param ar  : Aspect ratio
 * @param inw : Input width
 * @param inh : Input height
 * @param left: Left zoom value
 * @param top : Top zoom value
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
bool    flyZoom::bandResized(int x,int y,int w, int h)
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
    if (ar_select > 0 && !ignore && rightHandleMoved)
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
 * \fn bandMoved
 * @param x
 * @param y
 * @param w
 * @param h
 * @return 
 */
bool    flyZoom::bandMoved(int x,int y,int w, int h)
{
    double halfzoom=_zoom/2-0.01;

    int normX, normY, normW, normH;
    normX=(int)(((double)x+halfzoom)/_zoom);
    normY=(int)(((double)y+halfzoom)/_zoom);
    normW=(int)(((double)w+halfzoom)/_zoom);
    normH=(int)(((double)h+halfzoom)/_zoom);

    // bound checks are done in rubber control

    right=bound(normX&0xfffe,normW,_w)&0xfffe;
    bottom=bound(normY&0xfffe,normH,_h)&0xfffe;

    if(normX<0) normX=0;
    if(normY<0) normY=0;

    top=normY&0xfffe;
    left=normX&0xfffe;

    upload(false,false);
    sameImage();
    return true; 
}
/**
 * \fn blockChanges
 * @param block
 * @return 
 */
#define APPLY_TO_ALL(x) {w->spinBoxLeft->x;w->spinBoxRight->x;w->spinBoxTop->x;w->spinBoxBottom->x; \
                         rubber->x;w->checkBoxRubber->x;w->comboBoxAspectRatio->x;}
bool flyZoom::blockChanges(bool block)
{
    Ui_zoomDialog *w=(Ui_zoomDialog *)_cookie;
    APPLY_TO_ALL(blockSignals(block));
    return true;
}
/**
 * 
 * @param redraw
 * @return 
 */
uint8_t flyZoom::upload(bool redraw, bool toRubber)
{
    aprintf("left=%d, right=%d, top=%d, bottom=%d\n",left,right,top,bottom);
    Ui_zoomDialog *w=(Ui_zoomDialog *)_cookie;
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
 * Read zoom values from UI
 * @param even Fixup zoom values to make resulting width and height even
 * @return 
 */
uint8_t flyZoom::download(bool even)
{
int reject=0;
Ui_zoomDialog *w=(Ui_zoomDialog *)_cookie;
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
void flyZoom::dimensions(void)
{
    Ui_zoomDialog *w=(Ui_zoomDialog *)_cookie;
    QString dim=QString(QT_TRANSLATE_NOOP("zoom","Selection: "));
    dim+=QString::number(_w-left-right);
    dim+=QString(" x ");
    dim+=QString::number(_h-top-bottom);
    w->labelSize->setText(dim);
}

/**
 * \fn setTabOrder
 * \brief Move navigation / playback buttons up the tab order list
 */
void flyZoom::setTabOrder(void)
{
    Ui_zoomDialog *w=(Ui_zoomDialog *)_cookie;
    std::vector<QWidget *> controls;

#define PUSHME(x) controls.push_back(w->spinBox##x);
    PUSHME(Left)
    PUSHME(Right)
    PUSHME(Top)
    PUSHME(Bottom)

    controls.push_back(w->checkBoxRubber);
    controls.push_back(w->comboBoxAspectRatio);
    controls.insert(controls.end(), buttonList.begin(), buttonList.end());
    controls.push_back(w->horizontalSlider);

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

//
//	Video is in YV12 Colorspace
//
//
Ui_zoomWindow::Ui_zoomWindow(QWidget* parent, zoom *param, bool firstRun, ADM_coreVideoFilter *in) : QDialog(parent)
{
    ui.setupUi(this);
    lock=0;
    shown = false;
    // Allocate space for green-ised video
    inputWidth = in->getInfo()->width;
    inputHeight = in->getInfo()->height;

    canvas = new ADM_QCanvas(ui.graphicsView, inputWidth, inputHeight);

    myFly = new flyZoom(this, inputWidth, inputHeight, in, canvas, ui.horizontalSlider);
    myFly->setZoomMargins(param->left, param->right, param->top, param->bottom);

    bool rubberIsHidden = false;
    QSettings *qset = qtSettingsCreate();
    if(qset)
    {
        qset->beginGroup("zoom");
        rubberIsHidden = qset->value("rubberIsHidden", false).toBool();
        if (firstRun)
        {
            param->algo = qset->value("defaultAlgo", 1).toInt();
            param->pad = qset->value("defaultPadding", 0).toInt();
            // sanitize
            if(param->algo >= ui.comboBoxAlgo->count())
                param->algo = 1;
            if(param->pad >= ui.comboBoxPad->count())
                param->pad = 0;
        }
        qset->endGroup();
        delete qset;
        qset = NULL;
    }

    myFly->hideRubber(rubberIsHidden);
    myFly->_cookie=&ui;
    myFly->addControl(ui.toolboxLayout);
    myFly->setTabOrder();

    ui.checkBoxRubber->setChecked(rubberIsHidden);
    ui.comboBoxAspectRatio->setCurrentIndex(param->ar_select);
    if(!param->ar_select)
        myFly->upload(false,true);

    connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
    connect( ui.checkBoxRubber,SIGNAL(stateChanged(int)),this,SLOT(toggleRubber(int)));
    connect( ui.comboBoxAspectRatio,SIGNAL(currentIndexChanged(int)),this,SLOT(changeARSelect(int)));

    ui.comboBoxAlgo->setCurrentIndex(param->algo);
    ui.comboBoxPad->setCurrentIndex(param->pad);

    QPushButton *pushButtonReset = ui.buttonBox->button(QDialogButtonBox::Reset);
    connect(pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

    preferencesButton = ui.buttonBox->addButton(QT_TRANSLATE_NOOP("zoom","Preferences"),QDialogButtonBox::ResetRole);
    preferencesButton->setCheckable(true);
    connect(preferencesButton,SIGNAL(clicked(bool)),this,SLOT(setPreferences(bool)));

    changeARSelect(param->ar_select);

#define SPINNER(x) connect(ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(widthChanged(int)));
    SPINNER(Left)
    SPINNER(Right)
#undef SPINNER
#define SPINNER(x) connect(ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(heightChanged(int)));
    SPINNER(Top)
    SPINNER(Bottom)

    QT6_CRASH_WORKAROUND(zoomWindow)

    setModal(true);
}
/**
 * 
 * @param foo
 */
void Ui_zoomWindow::sliderUpdate(int foo)
{
    myFly->sliderChanged();
}
/**
 * 
 * @param param
 */
void Ui_zoomWindow::gather(zoom *param)
{
    myFly->download(true);
    int left,right,top,bottom;
    myFly->getZoomMargins(&left,&right,&top,&bottom);
    param->left = left;
    param->right = right;
    param->top = top;
    param->bottom = bottom;
    param->ar_select = myFly->getAspectRatioIndex();
    param->algo=ui.comboBoxAlgo->currentIndex();
    param->pad=ui.comboBoxPad->currentIndex();
}
/**
 * \fn dtor
 */
Ui_zoomWindow::~Ui_zoomWindow()
{
    if(myFly) delete myFly;
    myFly=NULL;
    if(canvas) delete canvas;
    canvas=NULL;
}
/**
 * \fn widthChanged
 */
void Ui_zoomWindow::widthChanged(int val)
{
    if(lock) return;
    lock++;
    myFly->lockRubber(true);
    if(myFly->getKeepAspect())
        updateRightBottomSpinners(val,false);
    myFly->download();
    myFly->sameImage();
    myFly->lockRubber(false);
    lock--;
}
/**
 * \fn heightChanged
 */
void Ui_zoomWindow::heightChanged(int val)
{
    if(lock) return;
    lock++;
    myFly->lockRubber(true);
    if(myFly->getKeepAspect())
        updateRightBottomSpinners(val,true);
    myFly->download();
    myFly->sameImage();
    myFly->lockRubber(false);
    lock--;
}
/**
 * \fn updateRightBottomSpinners
 */
void Ui_zoomWindow::updateRightBottomSpinners(int val, bool useHeightAsRef)
{
    const double ar = myFly->getAspectRatio();
    int left,top;
    myFly->getZoomMargins(&left,NULL,&top,NULL);
    myFly->blockChanges(true);

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
    myFly->blockChanges(false);
}
/**
 * \fn toggleRubber
 */
void Ui_zoomWindow::toggleRubber(int checkState)
{
    bool visible=true;
    if(checkState)
        visible=false;
    myFly->hideRubber(!visible);
}
/**
 * \fn rubberIsVisible
 */
bool Ui_zoomWindow::rubberIsHidden(void)
{
    return myFly->stateOfRubber();
}
/**
 * \fn applyAspectRatio
 */
void Ui_zoomWindow::applyAspectRatio(void) {
    if(!lock)
    {
        lock++;
        int left,right,top,bottom;
        myFly->getZoomMargins(&left,&right,&top,&bottom);
        int wout = inputWidth - left - right;
        int hout = inputHeight - top - bottom;
        recomputeDimensions(myFly->getAspectRatio(),inputWidth,inputHeight,left,top,wout,hout);

        right = boundChecked(inputWidth - wout - left, inputWidth);
        bottom = boundChecked(inputHeight - hout - top, inputHeight);
        myFly->setZoomMargins(left,right,top,bottom);
        myFly->upload(true,true);

        myFly->lockRubber(true);
        myFly->download();
        myFly->sameImage();
        myFly->lockRubber(false);
        lock--;
    }
}
/**
 * \fn changeARSelect
 */
void Ui_zoomWindow::changeARSelect(int f)
{
    myFly->lockDimensions();
    myFly->setAspectRatioIndex(f);

    bool keep_aspect = myFly->getKeepAspect();
    if(keep_aspect)
        applyAspectRatio();
    ui.spinBoxLeft->setEnabled(!keep_aspect);
    ui.spinBoxTop->setEnabled(!keep_aspect);
    myFly->hideRubberGrips(keep_aspect,false);
}

/**
 * \fn setPreferences
 */
void Ui_zoomWindow::setPreferences(bool f)
{
    UNUSED_ARG(f);

    QSettings *qset = qtSettingsCreate();
    if(!qset)
    {
        preferencesButton->setChecked(false);
        return;
    }
    myFly->play(false); // stop playback

    qset->beginGroup("zoom");

    QDialog dialog(preferencesButton);
    dialog.setWindowTitle(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Preferences")));

    QGroupBox *frameDefaults = new QGroupBox(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Defaults for new filter instances")));

    QLabel *textAlgo = new QLabel(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Resize method:")));

    QComboBox *saveAlgoComboBox = new QComboBox();
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Most recently accepted")),-1);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Bilinear")),0);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Bicubic")),1);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Lanczos")),2);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Spline")),3);

    int userData = (qset->value("saveAlgo", 0).toInt() > 0)? -1 : qset->value("defaultAlgo", 1).toInt();

    for(int i = 0; i < saveAlgoComboBox->count(); i++)
    {
        if(userData != saveAlgoComboBox->itemData(i).toInt()) continue;
        saveAlgoComboBox->setCurrentIndex(i);
        break;
    }

    QLabel *textPaddingType = new QLabel(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Padding type:")));

    QComboBox *savePadComboBox = new QComboBox();
    savePadComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Most recently accepted")),-1);
    savePadComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Black Bars")),0);
    savePadComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","Echo")),1);
    savePadComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("zoom","None")),2);

    userData = (qset->value("savePad", 0).toInt() > 0)? -1 : qset->value("defaultPadding", 0).toInt();

    for(int i = 0; i < savePadComboBox->count(); i++)
    {
        if(userData != savePadComboBox->itemData(i).toInt()) continue;
        savePadComboBox->setCurrentIndex(i);
        break;
    }

    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    QGridLayout *grid = new QGridLayout();

    grid->addWidget(textAlgo,0,0);
    grid->addWidget(saveAlgoComboBox,0,1);
    grid->addWidget(textPaddingType,1,0);
    grid->addWidget(savePadComboBox,1,1);
    grid->setColumnStretch(1,1);

    frameDefaults->setLayout(grid);

    QVBoxLayout *vboxLayout = new QVBoxLayout();

    vboxLayout->addWidget(frameDefaults);
    vboxLayout->addSpacerItem(spacer);
    vboxLayout->addWidget(buttonBox);
    dialog.setLayout(vboxLayout);

    if(QDialog::Accepted == dialog.exec())
    {
        int idx = saveAlgoComboBox->currentIndex();
        qset->setValue("saveAlgo", saveAlgoComboBox->itemData(idx).toInt() == -1);
        if(idx > 0)
            qset->setValue("defaultAlgo", saveAlgoComboBox->itemData(idx));
        idx = savePadComboBox->currentIndex();
        qset->setValue("savePad", savePadComboBox->itemData(idx).toInt() == -1);
        if(idx > 0)
            qset->setValue("defaultPadding", savePadComboBox->itemData(idx));
    }
    qset->endGroup();
    delete qset;
    qset = NULL;

    preferencesButton->setChecked(false);
}

/**
 * 
 * @param f
 */
void Ui_zoomWindow::reset( bool f )
{
    lock++;
    myFly->blockChanges(true);
    ui.comboBoxAspectRatio->setCurrentIndex(0);
    myFly->setAspectRatioIndex(0);
    changeARSelect(0);
    myFly->setZoomMargins(0,0,0,0);
    myFly->lockDimensions();
    myFly->blockChanges(false);
    ui.comboBoxAlgo->setCurrentIndex(1);
    ui.comboBoxPad->setCurrentIndex(0);
    myFly->upload();
    myFly->sameImage();
    lock--;
}
/**
 * 
 * @param event
 */
void Ui_zoomWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myFly->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myFly->adjustCanvasPosition();

    int left,right,top,bottom;
    myFly->getZoomMargins(&left,&right,&top,&bottom);

    float z = myFly->getZoomValue();
    int x = (double)left * z + 0.49;
    int y = (double)top * z + 0.49;
    int w = (double)(inputWidth - (left + right)) * z + 0.49;
    int h = (double)(inputHeight - (top + bottom)) * z + 0.49;

    myFly->blockChanges(true);
    myFly->lockRubber(true);
    myFly->adjustRubber(x,y,w,h);
    myFly->lockRubber(false);
    myFly->blockChanges(false);
}

/**
 * \fn showEvent
 * \brief set canvas size and position
 */
void Ui_zoomWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if(shown) return;
    shown = true;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    /* Avoid shifting the layout displaying output aspect ratio
    by setting a sufficient minimum width based on font metrics. */
    QFontMetrics fm = ui.labelSize->fontMetrics(); // we may assume that both labels use the same font
    QString text = QString(QT_TRANSLATE_NOOP("zoom","Selection: "));

    int w = inputWidth;
    int h = inputHeight;
    const char *str[5] = {"0","00","000","0000","00000"};

    int pos = 0;
    while((w = w / 10) && pos < 4)
        pos++;
    text += str[pos];
    text += " x ";
    pos = 0;
    while((h = h / 10) && pos < 4)
        pos++;
    text += str[pos];

    ui.labelSize->setMinimumWidth(1.05 * fm.boundingRect(text).width());

    myFly->refreshImage();
    myFly->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled

    QApplication::restoreOverrideCursor();
}

//EOF

