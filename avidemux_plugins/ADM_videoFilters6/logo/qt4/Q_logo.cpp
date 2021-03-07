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

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif
/**
 * 
 * @param z
 * @param w
 * @param h
 */
 ADM_LogoCanvas::ADM_LogoCanvas(QWidget *z, uint32_t w, uint32_t h) : ADM_QCanvas(z,w,h)
 {
     
 }
 /**
  * 
  */
 ADM_LogoCanvas::~ADM_LogoCanvas()
 {
     
 }

/**
 * 
 * @param event
 */
void ADM_LogoCanvas::mousePressEvent(QMouseEvent * event)
{
    aprintf("Pressed\n");
}
/**
 * 
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
 * 
 * @param event
 */
void ADM_LogoCanvas::moveEvent(QMoveEvent * event)
{
    aprintf("Move\n");
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
        ui.spinX->setFocus();
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
        {
            myLogo->sameImage();
        }
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
            image=im2;
            imageWidth=image->GetWidth(PLANAR_Y);
            imageHeight=image->GetHeight(PLANAR_Y);
            this->imageName=imageName;
            if(image->GetReadPtr(PLANAR_ALPHA))
                ADM_info("We have alpha\n");
            status=true;
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
        _in=in;
        
        image=NULL;;
        admCoreUtils::getLastReadFolder(lastFolder);
        if(param->logoImageFile.size())
            tryToLoadimage(param->logoImageFile.c_str());
        else
            enableLowPart();
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
        myLogo->param.alpha=param->alpha;
        myLogo->param.logoImageFile=param->logoImageFile;
        myLogo->param.fade=param->fade;
        myLogo->_cookie=&ui;
        myLogo->upload();
        myLogo->setPreview(false);

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
        connect( ui.pushButtonSelect,SIGNAL(pressed()),this,SLOT(imageSelect()));
#define SPINNER(x,y) connect(ui.x,SIGNAL(valueChanged(y)),this,SLOT(valueChanged(y)));
        SPINNER(spinX,int);
        SPINNER(spinY,int);
        SPINNER(spinAlpha,int);
        SPINNER(spinFadeInOut,double);
        connect(canvas, SIGNAL(movedSignal(int,int)),this, SLOT(moved(int,int)));
#undef SPINNER
        myLogo->addControl(ui.toolboxLayout);
        myLogo->setTabOrder();
        myLogo->sliderChanged();

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
 * 
 * @param x
 * @param y
 */
void Ui_logoWindow::moved(int x,int y)
{
      if(lock) return;
      lock++;
      myLogo->setXy(x,y);
      myLogo->sameImage();      
      lock--;
}
/**
 * 
 * @param x
 */
 void Ui_logoWindow::preview(int x)
 {
     if(x==Qt::Checked)
     {
         myLogo->setPreview(true);
         myLogo->sameImage();
     }
     else
     {
         myLogo->setPreview(false);
         myLogo->sameImage();
     }
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

/**
    \fn flyLogo ctor
*/
flyLogo::flyLogo (QDialog *parent, uint32_t width, uint32_t height, ADM_coreVideoFilter *in, ADM_QCanvas *canvas, ADM_QSlider *slider)
    : ADM_flyDialogYuv(parent,width,height,in,canvas,slider,RESIZE_AUTO)
{
    in->getTimeRange(&startOffset,&endOffset);
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
 * 
 * @param x
 * @param y
 * @return 
 */
bool flyLogo::setXy(int x,int y)
{
    if(x<0) x=0;
    if(y<0) y=0;
    double scale=(double)(_canvas->width()) / _in->getInfo()->width;
    param.x= (int)((double)x / scale);
    param.y= (int)((double)y / scale);
    upload();
    return true;
}

//************************
/**
    \fn upload
*/
uint8_t flyLogo::upload(void)
{

    Ui_logoDialog *w = (Ui_logoDialog *)this->_cookie;
#define SETSPIN(u,v) w->spin##u->setValue(v);
    SETSPIN(X,param.x)
    SETSPIN(Y,param.y)
    SETSPIN(Alpha,param.alpha)
    SETSPIN(FadeInOut,(double)(param.fade)/1000)

    return 1;
}
/**
        \fn download
*/
uint8_t flyLogo::download(void)
{
    Ui_logoDialog *w=(Ui_logoDialog *)this->_cookie;
#define GETSPIN(u,v) param.v=w->spin##u->value();
    GETSPIN(X,x)
    GETSPIN(Y,y)
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
    if(!parent->image)
        return true;
    
    int targetHeight=out->GetHeight(PLANAR_Y);
    int targetWidth =out->GetWidth(PLANAR_Y);
    
    
    if(param.y>targetHeight) 
        return true;
    if(param.x>targetWidth) 
        return true;

    ADMImage *myImage=parent->image;
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


