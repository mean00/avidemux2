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
        scene->setSceneRect(0,0,256,128);
        ui.graphicsViewHistogram->setScene(scene);
        ui.graphicsViewHistogram->scale(1.0,1.0);

        myCrop=new flyContrast( this,width, height,in,canvas,ui.horizontalSlider,scene);
        memcpy(&(myCrop->param),param,sizeof(contrast));
        myCrop->_cookie=&ui;
        myCrop->addControl(ui.toolboxLayout, ControlOption::PeekOriginalBtn);
        myCrop->setTabOrder();
        myCrop->upload();

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
          connect( ui.toolButton__DVD2PC,SIGNAL(pressed()),this,SLOT(dvd2PC()));  

        QT6_CRASH_WORKAROUND(contrastWindow)

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

        tablesPopulated = false;
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

        if(oldCoef != param.coef || oldOffset != param.offset)
        {
            tablesPopulated = false;
            oldCoef = param.coef;
            oldOffset = param.offset;
        }

#define CHECKGET(a,b) param.b=MYCHECK(a)->isChecked()

        CHECKGET(Y,doLuma);
        CHECKGET(U,doChromaU);
        CHECKGET(V,doChromaV);
return 1;
}
/**
 * \fn setTabOrder
 */
void flyContrast::setTabOrder(void)
{
    Ui_contrastDialog *w=(Ui_contrastDialog *)_cookie;
    std::vector<QWidget *> controls;
#define PUSH_SPIN(x) controls.push_back(w->dial##x);
#define PUSH_CHECK(x) controls.push_back(w->checkBox##x);
    PUSH_SPIN(Contrast)
    PUSH_SPIN(Brightness)
    PUSH_CHECK(Y)
    PUSH_CHECK(U)
    PUSH_CHECK(V)

    controls.push_back(w->toolButton__DVD2PC);
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


