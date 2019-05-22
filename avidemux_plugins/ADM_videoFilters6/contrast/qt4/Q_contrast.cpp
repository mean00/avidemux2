/***************************************************************************
                          DIA_crop.cpp  -  description
                             -------------------

                            GUI for cropping including autocrop
                            +Revisted the Gtk2 way
                             +Autocrop now in RGB space (more accurate)

    begin                : Fri May 3 2002
    copyright            : (C) 2002/2007 by mean
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

#include "Q_contrast.h"
#include "ADM_toolkitQt.h"

//
//	Video is in YV12 Colorspace
//
//
  Ui_contrastWindow::Ui_contrastWindow(QWidget* parent, contrast *param,ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);

        scene=new QGraphicsScene(this);
        ui.graphicsViewHistogram->setScene(scene);
        ui.graphicsViewHistogram->scale(1.0,1.0);

        myCrop=new flyContrast( this,width, height,in,canvas,ui.horizontalSlider,scene);
        memcpy(&(myCrop->param),param,sizeof(contrast));
        myCrop->_cookie=&ui;
        myCrop->addControl(ui.toolboxLayout);
        myCrop->upload();
        myCrop->sliderChanged();

        previewState=true;
        ui.checkBox_Enabled->setChecked(true);


        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.dial##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int))); 
          SPINNER(Brightness);
          SPINNER(Contrast);

        setDialTitles();
        // Allocate enough width for dial titles to avoid shifting elements of the dialog
        // on transition e.g. 100% <-> 99% with large font sizes
        QString text=QString(QT_TRANSLATE_NOOP("contrast","Contrast"))+QString(": 100 %");
        QString text2=QString(QT_TRANSLATE_NOOP("contrast","Brightness"))+QString(": -100");
        QFontMetrics fm=ui.labelContrast->fontMetrics();
        QFontMetrics fm2=ui.labelBrightness->fontMetrics();
        int labelContrastWidth=fm.boundingRect(text).width()+8; // 8px security margin
        int labelBrightnessWidth=fm2.boundingRect(text2).width()+8; // 8px security margin
        ui.labelContrast->setMinimumWidth(labelContrastWidth);
        ui.labelBrightness->setMinimumWidth(labelBrightnessWidth);

          connect( ui.checkBoxU,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));
          connect( ui.checkBoxV,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int))); 
          connect( ui.checkBoxY,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));  
          connect( ui.checkBox_Enabled,SIGNAL(stateChanged(int)),this,SLOT(previewActivated(int)));  
          connect( ui.toolButton__DVD2PC,SIGNAL(pressed()),this,SLOT(dvd2PC()));  

        setModal(true);
  }
/**
 * 
 * @param foo
 */  
  void Ui_contrastWindow::sliderUpdate(int foo)
  {
    myCrop->sliderChanged();
  }
/**
 * 
 * @param param
 */  
  void Ui_contrastWindow::gather(contrast *param)
  {

        myCrop->download();
        memcpy(param,&(myCrop->param),sizeof(contrast));
  }
  /**
   * 
   */
Ui_contrastWindow::~Ui_contrastWindow()
{
  if(myCrop) delete myCrop;
  myCrop=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
  scene=NULL;
}

void Ui_contrastWindow::dvd2PC()
{
   if(lock) return;
  lock++;
  myCrop->param.coef=1.16;
  myCrop->param.offset=-16;
  myCrop->upload();
  myCrop->sameImage();
  setDialTitles();
  lock--;
}
/**
 * 
 * @param a
 */
void Ui_contrastWindow::previewActivated(int a)
{
    previewState=a;
    myCrop->setState(a);
    myCrop->sameImage();
}
/**
 * 
 * @param f
 */
void Ui_contrastWindow::valueChanged( int f )
{
  if(lock) return;
  lock++;
  myCrop->download();
  myCrop->sameImage();
  setDialTitles();
  lock--;
}

/**
 *    \fn setDialTitles
 */
void Ui_contrastWindow::setDialTitles(void)
{
    QString title=QString(QT_TRANSLATE_NOOP("contrast","Contrast"))+QString(": %1 %").arg((int)(100*myCrop->param.coef));
    QString title2=QString(QT_TRANSLATE_NOOP("contrast","Brightness"))+QString(": %2").arg(myCrop->param.offset);
    ui.labelContrast->setText(title);
    ui.labelBrightness->setText(title2);
}

/**
    \fn resizeEvent
*/
void Ui_contrastWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    myCrop->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    myCrop->adjustCanvasPosition();
}

/**
    \fn showEvent
*/
void Ui_contrastWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    myCrop->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define MYSPIN(x) w->dial##x
#define MYCHECK(x) w->checkBox##x
/**
 * 
 * @return 
 */
uint8_t flyContrast::upload(void)
{
      Ui_contrastDialog *w=(Ui_contrastDialog *)_cookie;

        MYSPIN(Contrast)->setValue((uint32_t)(param.coef*100));
        MYSPIN(Brightness)->setValue(param.offset);
#define CHECKSET(a,b) MYCHECK(a)->setChecked(param.b)

        CHECKSET(Y,doLuma);
        CHECKSET(U,doChromaU);
        CHECKSET(V,doChromaV);
        return 1;
}
/**
 * 
 * @return 
 */
uint8_t flyContrast::download(void)
{
       Ui_contrastDialog *w=(Ui_contrastDialog *)_cookie;
         param.coef=MYSPIN(Contrast)->value()/100.;
         param.offset=MYSPIN(Brightness)->value();
#define CHECKGET(a,b) param.b=MYCHECK(a)->isChecked()

        CHECKGET(Y,doLuma);
        CHECKGET(U,doChromaU);
        CHECKGET(V,doChromaV);
return 1;
}

/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
bool DIA_getContrast(ADM_coreVideoFilter *in,contrast *param)
{
    uint8_t ret=0;
    Ui_contrastWindow dialog(qtLastRegisteredDialog(), param,in);
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


