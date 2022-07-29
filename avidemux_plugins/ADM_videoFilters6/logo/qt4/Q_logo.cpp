/***************************************************************************
                        Q_mpdelogo.cpp  -  description
                             -------------------

			   flyDialog for MPlayer DeLogo filter

    copyright            : (C) 2004/2007 by mean
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

#include "Q_logo.h"
#include "ADM_toolkitQt.h"
#include "ADM_imageLoader.h"
#include "DIA_fileSel.h"
#include "ADM_last.h"
#include "ADM_vidLogo.h"

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

/**
 * \fn mouseReleaseEvent
 * @param event
 */
void ADM_LogoCanvas::mouseReleaseEvent(QMouseEvent * event)
{
    QPoint p=event->pos();
    int x=p.x();
    int y=p.y();

    if(x<0) x=0;
    if(y<0) y=0;

    aprintf("Released %d %d\n",x,y);
    emit movedSignal(x,y);

}

/**
 * \fn enableLowPart
 * @return 
 */
bool Ui_logoWindow::enableLowPart(void)
{
    bool enable = !imageName.empty();
#if 0
#define DOIT(x) ui.label##x->setEnabled(enable); ui.spin##x->setEnabled(enable);
    DOIT(X)
    DOIT(Y)
    DOIT(Alpha)
    DOIT(FadeInOut)
#endif
    if(enable)
    {
        std::string desc = QT_TRANSLATE_NOOP("logo","Image:");
        desc += " ";
        desc += imageName;
        ui.labelImage->setText(desc.c_str());
        //ui.spinX->setFocus();
        return true;
    }
    ui.labelImage->setText(QT_TRANSLATE_NOOP("logo","No image selected"));
    ui.pushButtonSelect->setFocus();
    return false;
}
/**
 * 
 * @return 
 */
void                   Ui_logoWindow::imageSelect()
{
     
    char buffer[2048];
    std::string source;
    if(imageName.size())
        source=imageName;
    else
        source=lastFolder;
    if(FileSel_SelectRead(QT_TRANSLATE_NOOP("logo","Select Logo Image"),buffer,2048,source.c_str()))
    {
        admCoreUtils::setLastReadFolder(std::string(buffer));
        if(tryToLoadimage(buffer))
            myLogo->sameImage();
    }
}
/**
 * \fn tryToLoadimage
 * @param image
 * @return 
 */
bool                Ui_logoWindow::tryToLoadimage(const char *imageName)
{
    bool status=false;
    if(strlen(imageName))
    {
        ADMImage *im2=createImageFromFile(imageName);
        if(im2)
        {
            if(image) delete image;
            if (scaledImage) delete scaledImage;
            image=im2;
            ADM_assert(myLogo);
            myLogo->imageWidth = image->GetWidth(PLANAR_Y);
            myLogo->imageHeight = image->GetHeight(PLANAR_Y);
            this->imageName=imageName;
            if(image->GetReadPtr(PLANAR_ALPHA))
                ADM_info("We have alpha\n");
            scaledImage = addLogopFilter::scaleImage(image, imageScale);
            if(scaledImage)
            {
                myLogo->imageWidth = scaledImage->GetWidth(PLANAR_Y);
                myLogo->imageHeight = scaledImage->GetHeight(PLANAR_Y);
                myLogo->adjustFrame(scaledImage);
                myLogo->updateFrameOpacity();
                status = true;
            }
        }
    }
    enableLowPart();
    return status;
}

/**
    \fn ctor
*/

  Ui_logoWindow::Ui_logoWindow(QWidget *parent,  logo *param, ADM_coreVideoFilter *in) 
            : QDialog(parent)
  {
        uint32_t width,height;
        ui.setupUi(this);
        image=NULL;
        scaledImage=NULL;
        myLogo=NULL;
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_LogoCanvas(ui.graphicsView,width,height);
        myLogo=new flyLogo(this, width, height,in,canvas,ui.horizontalSlider);

#define SPINENTRY(x) ui.spin##x
        SPINENTRY(X)->setMaximum(width);
        SPINENTRY(Y)->setMaximum(height);
        SPINENTRY(Alpha)->setMaximum(255);
        SPINENTRY(Alpha)->setMinimum(0);
        SPINENTRY(FadeInOut)->setDecimals(1);
        SPINENTRY(FadeInOut)->setSuffix(QT_TRANSLATE_NOOP("logo"," s"));
        SPINENTRY(FadeInOut)->setSingleStep(.1);
        SPINENTRY(FadeInOut)->setMaximum(10.);
        SPINENTRY(FadeInOut)->setMinimum(0.);

        myLogo->param.x=param->x;
        myLogo->param.y=param->y;
        myLogo->param.scale=param->scale;
        myLogo->param.alpha=param->alpha;
        myLogo->param.logoImageFile=param->logoImageFile;
        myLogo->param.fade=param->fade;
        myLogo->_cookie=&ui;

        admCoreUtils::getLastReadFolder(lastFolder);
        imageScale = param->scale;
        if(param->logoImageFile.size())
            tryToLoadimage(param->logoImageFile.c_str());
        else
            enableLowPart();

        myLogo->upload();
        myLogo->addControl(ui.toolboxLayout);
        myLogo->setTabOrder();
        myLogo->refreshImage();

        connect( ui.pushButtonSelect,SIGNAL(pressed()),this,SLOT(imageSelect()));
        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));

#define SPINNER(x,y) connect(ui.x,SIGNAL(valueChanged(y)),this,SLOT(valueChanged(y)));
        SPINNER(spinX,int);
        SPINNER(spinY,int);
        SPINNER(spinAlpha,int);
        SPINNER(spinFadeInOut,double);
#undef SPINNER

        connect(ui.spinScale,SIGNAL(valueChanged(double)),this,SLOT(scaleChanged(double)));
        connect(canvas,SIGNAL(movedSignal(int,int)),this,SLOT(moved(int,int)));

        setModal(true);
  }
/**
    \fn sliderUpdate
*/
void Ui_logoWindow::sliderUpdate(int foo)
{
    myLogo->sliderChanged();
}
/**
    \fn gather
*/
void Ui_logoWindow::gather(logo *param)
{
    myLogo->download();
#define DUPE(x)     param->x=myLogo->param.x;
    DUPE(x)
    DUPE(y)
    DUPE(scale)
    DUPE(alpha)
    DUPE(fade)
    param->logoImageFile=imageName;
}
/**
    \fn dtor
*/
Ui_logoWindow::~Ui_logoWindow()
{
    admCoreUtils::setLastReadFolder(lastFolder);
    if(myLogo) delete myLogo;
    myLogo=NULL; 
    if(canvas) delete canvas;
    canvas=NULL;
}
/**
    \fn valueChanged
*/

void Ui_logoWindow::valueChanged( int f )
{
    if(lock) return;
    lock++;
    myLogo->download();
    myLogo->updateFrameOpacity();
    myLogo->adjustFrame();
    myLogo->sameImage();
    lock--;
}

/**
    \fn valueChanged
*/
void Ui_logoWindow::valueChanged(double f)
{
    if(lock) return;
    lock++;
    myLogo->download();
    myLogo->sameImage();
    lock--;
}

/**
 * \fn moved
 * @param x
 * @param y
 */
void Ui_logoWindow::moved(int x, int y)
{
    if(lock) return;
    lock++;
    myLogo->setXy(x,y);
    myLogo->sameImage();
    lock--;
}

/**
    \fn scaleChanged
*/
void Ui_logoWindow::scaleChanged(double f)
{
    if(lock) return;
    lock++;
    myLogo->download();
    imageScale = ui.spinScale->value();
    if (image)
    {
        if (scaledImage) delete scaledImage;
        scaledImage=NULL;
        scaledImage = addLogopFilter::scaleImage(image, imageScale);
        if (scaledImage)
        {
            myLogo->imageWidth = scaledImage->GetWidth(PLANAR_Y);
            myLogo->imageHeight = scaledImage->GetHeight(PLANAR_Y);
            myLogo->adjustFrame(scaledImage);
        }
    }
    myLogo->sameImage();
    lock--;
}

/**
    \fn resizeEvent
*/
void Ui_logoWindow::resizeEvent(QResizeEvent *event)
{
    if(lock) return;
    if(!canvas->height())
        return;
    lock++;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myLogo->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myLogo->adjustCanvasPosition();
    myLogo->adjustFrame();

    lock--;
}

/**
    \fn showEvent
    \brief set canvas initial size and position
*/
void Ui_logoWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myLogo->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
}

/*********************************************************************/

/**
    \fn ctor
*/
draggableFrame::draggableFrame(ADM_flyDialog *fly, QWidget *parent) : QWidget(parent)
{
    flyParent = fly;
    drag = false;
    pitch = 0;
    opacity = 1;
    rgbdata = NULL;
}

/**
    \fn dtor
*/
draggableFrame::~draggableFrame()
{
    ADM_dealloc(rgbdata);
    rgbdata = NULL;
}

/**
    \fn setImage
*/
bool draggableFrame::setImage(ADMImage *pic)
{
    if (!pic) return false;
    ADM_dealloc(rgbdata);
    rgbdata = NULL;
    pitch = width();
    pitch = ADM_IMAGE_ALIGN(pitch * 4);
    rgbdata = (uint8_t *)ADM_alloc(pitch * height());
    if (!rgbdata) return false;
    ADMColorScalerFull scaler(ADM_CS_BICUBIC, pic->GetWidth(PLANAR_Y), pic->GetHeight(PLANAR_Y),
        width(), height(), ADM_PIXFRMT_YV12, ADM_PIXFRMT_BGR32A);

    return scaler.convertImage(pic,rgbdata);
}

/**
    \fn paintEvent
*/
void draggableFrame::paintEvent(QPaintEvent *event)
{
    if (!drag) return;

    QPainter painter(this);
    if (rgbdata)
    {
        painter.setOpacity(opacity);
        QImage replica(rgbdata, width(), height(), pitch, QImage::Format_RGB32);
        painter.drawImage(rect(),replica);
        painter.setOpacity(1.0);
    }
    QPen pen;
    QColor color(Qt::red);
    pen.setColor(color);
    pen.setWidth(4);
    painter.setPen(pen);
    painter.drawRect(rect());
    painter.end();
}

/**
    \fn enterEvent
*/
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
void draggableFrame::enterEvent(QEvent *event)
#else
void draggableFrame::enterEvent(QEnterEvent *event)
#endif
{
    setCursor(Qt::SizeAllCursor);
}

/**
    \fn leaveEvent
*/
void draggableFrame::leaveEvent(QEvent *event)
{
    setCursor(Qt::ArrowCursor);
}

/**
    \fn mousePressEvent
*/
void draggableFrame::mousePressEvent(QMouseEvent *event)
{
    dragOffset =
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    event->globalPos()
#else
    event->globalPosition().toPoint()
#endif
    - pos();
    dragGeometry = rect();
    drag = true;
    update();
}

/**
    \fn calculatePosition
*/
void draggableFrame::calculatePosition(QMouseEvent *event, int &xpos, int &ypos)
{
    int w, h, pw, ph;
    QPoint delta =
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    event->globalPos()
#else
    event->globalPosition().toPoint()
#endif
    - dragOffset;
    xpos = delta.x();
    ypos = delta.y();
    w = dragGeometry.width();
    h = dragGeometry.height();
    QSize sz = parentWidget()->size();
    pw = sz.width();
    ph = sz.height();
    if (xpos < 0) xpos = 0;
    if (ypos < 0) ypos = 0;
    if (xpos > pw) xpos = pw;
    if (ypos > ph) ypos = ph;
}

/**
    \fn mouseReleaseEvent
*/
void draggableFrame::mouseReleaseEvent(QMouseEvent *event)
{
    drag = false;

    int x, y;
    calculatePosition(event,x,y);

    flyParent->bandMoved(x, y, width(), height());
    update();
}

/**
    \fn mouseMoveEvent
*/
void draggableFrame::mouseMoveEvent(QMouseEvent *event)
{
    if (!drag) return;

    int x, y;
    calculatePosition(event,x,y);

    move(x,y);
}

/*********************************************************************/

/**
 *  \fn flyLogo ctor
 */
flyLogo::flyLogo (QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in, ADM_QCanvas *canvas, ADM_flyNavSlider *slider)
    : ADM_flyDialogYuv(parent,width,height,in,canvas,slider,RESIZE_AUTO)
{
    frame = NULL;
    imageWidth = imageHeight = 0;
    in->getTimeRange(&startOffset,&endOffset);
}

/**
 *  \fn flyLogo dtor
 */
flyLogo::~flyLogo()
{
    if (frame) delete frame;
    frame = NULL;
}

/**
 *  \fn bandMoved
 */
bool flyLogo::bandMoved(int x, int y, int w, int h)
{
    UNUSED_ARG(w);
    UNUSED_ARG(h);

    int normX,normY;
    normX = ((double)x / _zoom) + 0.49;
    normY = ((double)y / _zoom) + 0.49;
    if (normX < 0) normX = 0;
    if (normY < 0) normY = 0;
    param.x = normX;
    param.y = normY;

    upload(false);
    sameImage();

    return true;
}

/**
 *  \fn setXy
 */
bool flyLogo::setXy(int x, int y)
{
    if(x<0) x=0;
    if(y<0) y=0;
    x = ((double)x / _zoom) + 0.49;
    y = ((double)y / _zoom) + 0.49;
    param.x = (x > _w)? _w : x;
    param.y = (y > _h)? _h : y;
    upload();

    return true;
}

/**
 *  \fn setTabOrder
 */
void flyLogo::setTabOrder(void)
{
    Ui_logoDialog *w=(Ui_logoDialog *)_cookie;
    std::vector<QWidget *> controls;

    controls.push_back(w->pushButtonSelect);
#define SPINNER(x) controls.push_back(w->spin##x);
    SPINNER(X)
    SPINNER(Y)
    SPINNER(Scale)
    SPINNER(Alpha)
    SPINNER(FadeInOut)
#undef SPINNER
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

/**
 *  \fn adjustFrame
 */
void flyLogo::adjustFrame(ADMImage *pic)
{
    if(imageWidth < 1 | imageHeight < 1)
        return;

    if(!frame)
    {
        frame = new draggableFrame(this, _canvas);
        frame->show();
    }

    Ui_logoDialog *w = (Ui_logoDialog *) _cookie;
#define APPLY_TO_ALL(c) { w->spinX->c; w->spinY->c; w->spinScale->c; w->spinAlpha->c; w->spinFadeInOut->c; if(frame) frame->c; }
    APPLY_TO_ALL(blockSignals(true))

    int a = _zoom * param.x + 0.49;
    int b = _zoom * param.y + 0.49;

    frame->move(a,b);

    a = _zoom * imageWidth + 0.49;
    b = _zoom * imageHeight + 0.49;

    frame->resize(a,b);

    if(pic && frame->setImage(pic))
        frame->update();

    APPLY_TO_ALL(blockSignals(false))
}

/**
 *  \fn updateFrameOpacity
 */
void flyLogo::updateFrameOpacity(void)
{
    if (!frame) return;
    float f = param.alpha;
    f /= 256 * 2; // only half as opaque as the logo
    if (f > 1.0) f = 1.0;
    frame->opacity = f;
}

/**
    \fn upload
*/
uint8_t flyLogo::upload(bool toDraggableFrame)
{
    Ui_logoDialog *w = (Ui_logoDialog *) _cookie;
    APPLY_TO_ALL(blockSignals(true))
#define SETSPIN(u,v) w->spin##u->setValue(v);
    SETSPIN(X,param.x)
    SETSPIN(Y,param.y)
    SETSPIN(Scale,param.scale)
    SETSPIN(Alpha,param.alpha)
    SETSPIN(FadeInOut,(double)(param.fade)/1000)

    if(toDraggableFrame)
        adjustFrame();

    APPLY_TO_ALL(blockSignals(false))

    return 1;
}
/**
        \fn download
*/
uint8_t flyLogo::download(void)
{
    Ui_logoDialog *w = (Ui_logoDialog *) _cookie;
#define GETSPIN(u,v) param.v=w->spin##u->value();
    GETSPIN(X,x)
    GETSPIN(Y,y)
    GETSPIN(Scale,scale)
    GETSPIN(Alpha,alpha)
#define ROUNDUP 100
    param.fade=(((uint32_t)(w->spinFadeInOut->value() * 1000.) + ROUNDUP/2)/ROUNDUP)*ROUNDUP;
    return 1;
}

/**
    \fn process
*/
uint8_t    flyLogo::processYuv(ADMImage* in, ADMImage *out)
{
    out->duplicate(in);
    uint64_t pts=in->Pts;
    Ui_logoWindow *parent=(Ui_logoWindow *)_parent;
    if(!parent->scaledImage)
        return true;
    
    int targetHeight=out->GetHeight(PLANAR_Y);
    int targetWidth =out->GetWidth(PLANAR_Y);
    
    
    if(param.y>targetHeight) 
        return true;
    if(param.x>targetWidth) 
        return true;

    ADMImage *myImage=parent->scaledImage;
    double a=(double)param.alpha;
    uint64_t transition=param.fade*1000LL;
    uint64_t duration=endOffset-startOffset;

    if(transition && duration)
    {
        if(transition*2 > duration)
            transition=duration/2;
        if(pts < startOffset || pts >= endOffset)
        {
            a = 0.;
        }else
        {
            pts -= startOffset;
            if(pts < transition)
            {
                a /= (double)transition;
                a *= pts;
            }
            if(pts > duration-transition)
            {
                a /= (double)transition;
                a *= duration-pts;
            }
        }
        if(a > 255.)
            a = 255.;
    }

    if(myImage->GetReadPtr(PLANAR_ALPHA))
        myImage->copyWithAlphaChannel(out,param.x,param.y,(uint32_t)a);
    else
        myImage->copyToAlpha(out,param.x,param.y,(uint32_t)a);
    return true;
}

/**
      \fn     DIA_getMpDelogo
      \brief  Handle delogo dialog
*/

bool DIA_getLogo(logo *param, ADM_coreVideoFilter *in)
{
    bool ret=false;

    Ui_logoWindow dialog(qtLastRegisteredDialog(), param,in);
    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=true;
    }

    qtUnregisterDialog(&dialog);
    return ret;
}
//____________________________________
// EOF


