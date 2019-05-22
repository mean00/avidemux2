/***************************************************************************                         
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
#include "Q_blackenBorders.h"
#include "ADM_toolkitQt.h"

//
//	Video is in RGB Colorspace
//
//
Ui_blackenWindow::Ui_blackenWindow(QWidget* parent, blackenBorder *param,ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
    ui.setupUi(this);    
    lock=0;
    // Allocate space for green-ised video
    width=in->getInfo()->width;
    height=in->getInfo()->height;

    canvas=new ADM_QCanvas(ui.graphicsView,width,height);
    
    myBlacken=new flyBlacken( this,width, height,in,canvas,ui.horizontalSlider);
    myBlacken->left=param->left;
    myBlacken->right=param->right;
    myBlacken->top=param->top;
    myBlacken->bottom=param->bottom;
    myBlacken->_cookie=&ui;
    myBlacken->addControl(ui.toolboxLayout);
    myBlacken->upload();
    myBlacken->sliderChanged();


    connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
    connect( ui.pushButtonReset,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
    SPINNER(Left);
    SPINNER(Right);
    SPINNER(Top);
    SPINNER(Bottom);

    setModal(true);
  }
  void Ui_blackenWindow::sliderUpdate(int foo)
  {
    myBlacken->sliderChanged();
  }
  void Ui_blackenWindow::gather(blackenBorder *param)
  {

    myBlacken->download();
    param->left=myBlacken->left;
    param->right=myBlacken->right;
    param->top=myBlacken->top;
    param->bottom=myBlacken->bottom;
}
Ui_blackenWindow::~Ui_blackenWindow()
{
  if(myBlacken) 
      delete myBlacken;
  myBlacken=NULL; 
  if(canvas) 
      delete canvas;
  canvas=NULL;
}
void Ui_blackenWindow::valueChanged( int f )
{
  if(lock) return;
  lock++;
  myBlacken->download();
  myBlacken->sameImage();
  lock--;
}

void Ui_blackenWindow::reset( bool f )
{
    myBlacken->left=0;
    myBlacken->right=0;
    myBlacken->bottom=0;
    myBlacken->top=0;
    lock++;
    myBlacken->upload();
    myBlacken->sameImage();
    lock--;
}

void Ui_blackenWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myBlacken->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myBlacken->adjustCanvasPosition();
}

void Ui_blackenWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myBlacken->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing both ways after the dialog has settled
}

//************************
uint8_t flyBlacken::upload(void)
{
Ui_blackenDialog *w=(Ui_blackenDialog *)_cookie;

    w->spinBoxLeft->setValue(left);
    w->spinBoxRight->setValue(right);
    w->spinBoxTop->setValue(top);
    w->spinBoxBottom->setValue(bottom);
    return 1;
}
uint8_t flyBlacken::download(void)
{
    int reject=0;
    Ui_blackenDialog *w=(Ui_blackenDialog *)_cookie;
#define SPIN_GET(x,y) x=w->spinBox##y->value();
        SPIN_GET(left,Left);
        SPIN_GET(right,Right);
        SPIN_GET(top,Top);
        SPIN_GET(bottom,Bottom);

        printf("%d %d %d %d\n",left,right,top,bottom);

        left&=0xffffe;
        right&=0xffffe;
        top&=0xffffe;
        bottom&=0xffffe;

        if((top+bottom)>_h)
                {
                        top=bottom=0;
                        reject=1;
                }
        if((left+right)>_w)
                {
                        left=right=0;
                        reject=1;
                }
        if(reject)
                upload();
        return true;
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
bool DIA_getBlackenParams(	blackenBorder *param,ADM_coreVideoFilter *in)
{
    bool ret=0;

    Ui_blackenWindow dialog(qtLastRegisteredDialog(), param,in);
    qtRegisterDialog(&dialog);

    if(dialog.exec()==QDialog::Accepted)
    {
        dialog.gather(param); 
        ret=1;
    }
    qtUnregisterDialog(&dialog);
    return ret;
}
//____________________________________
// EOF
