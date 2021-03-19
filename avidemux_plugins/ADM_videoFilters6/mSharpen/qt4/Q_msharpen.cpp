/***************************************************************************
                         Q_Asharp.cpp  -  description
                             -------------------

			     flyDialog for Asharp
			     +Revisted the Gtk2 way

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

#include <QPushButton>
#include "Q_msharpen.h"
#include "ADM_toolkitQt.h"
#include "../ADM_vidMSharpen.h"

//
//	Video is in YV12 Colorspace
//
//
  Ui_msharpenWindow::Ui_msharpenWindow(QWidget *parent,  msharpen *param, ADM_coreVideoFilter *in) : QDialog(parent)
  {
    uint32_t width,height;
        ui.setupUi(this);
        lock=0;
        // Allocate space for green-ised video
        width=in->getInfo()->width;
        height=in->getInfo()->height;

        canvas=new ADM_QCanvas(ui.graphicsView,width,height);
        
        flymsharpen=new flyMSharpen( this,width, height,in,canvas,ui.horizontalSlider);
        memcpy(&(flymsharpen->param),param,sizeof(*param));
        flymsharpen->_cookie=&ui;
        flymsharpen->addControl(ui.toolboxLayout, true);
        flymsharpen->setTabOrder();
        flymsharpen->upload();
        flymsharpen->sliderChanged();

        ui.horizontalSliderStrength->setFocus();

        connect( ui.horizontalSlider,SIGNAL(valueChanged(int)),this,SLOT(sliderUpdate(int)));
#define SPINNER(x) connect( ui.spinBox##x,SIGNAL(valueChanged(int)),this,SLOT(valueChanged(int)));\
		   connect( ui.horizontalSlider##x,SIGNAL(valueChanged(int)),this,SLOT(valueChangedSlider(int)));
#define TOGGLER(x) connect(ui.checkBox##x,SIGNAL(stateChanged(int)),this,SLOT(valueChanged(int)));

        TOGGLER(HQ)
        TOGGLER(Mask)
        TOGGLER(Chroma)

        SPINNER(Threshold);
        SPINNER(Strength);

        QPushButton *resetButton = ui.buttonBox->button(QDialogButtonBox::Reset);
        connect(resetButton,SIGNAL(clicked(bool)),this,SLOT(reset(bool)));

        setModal(true);
  }
  void Ui_msharpenWindow::sliderUpdate(int foo)
  {
    flymsharpen->sliderChanged();
  }
  void Ui_msharpenWindow::gather(msharpen *param)
  {
    
        flymsharpen->download();
        memcpy(param,&(flymsharpen->param),sizeof(*param));
  }
Ui_msharpenWindow::~Ui_msharpenWindow()
{
  if(flymsharpen) delete flymsharpen;
  flymsharpen=NULL; 
  if(canvas) delete canvas;
  canvas=NULL;
}
void Ui_msharpenWindow::valueChanged( int f )
{
  if(lock) return;
  lock++;
  flymsharpen->download();
  flymsharpen->sameImage();
  lock--;
}
void Ui_msharpenWindow::reset(bool checked)
{
    if(lock) return;
    lock++;
    Msharpen::reset(&flymsharpen->param);
    flymsharpen->upload();
    flymsharpen->sameImage();
    lock--;
}
void Ui_msharpenWindow::resizeEvent(QResizeEvent *event)
{
    if(!canvas->height())
        return;
    uint32_t graphicsViewWidth = canvas->parentWidget()->width();
    uint32_t graphicsViewHeight = canvas->parentWidget()->height();
    flymsharpen->fitCanvasIntoView(graphicsViewWidth,graphicsViewHeight);
    flymsharpen->adjustCanvasPosition();
}

void Ui_msharpenWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    flymsharpen->adjustCanvasPosition();
    canvas->parentWidget()->setMinimumSize(30,30); // allow resizing after the dialog has settled
}

#define SYNCSPIN(x) ui.spinBox##x->setValue(ui.horizontalSlider##x->value());
void Ui_msharpenWindow::valueChangedSlider(int f)
{
	flymsharpen->blockChanges(true);

	SYNCSPIN(Threshold);
	SYNCSPIN(Strength);

	flymsharpen->blockChanges(false);
	valueChanged(0);
}
//____________________________________
// EOF


