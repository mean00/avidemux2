/***************************************************************************
    copyright            : (C) 2001 by mean
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

#include <math.h>

#include "Q_resize.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_toolkitQt.h"

static double aspectRatio[2][3]={
                              {1.,0.888888,1.19}, // NTSC 1:1 4:3 16:9
                              {1.,1.066667,1.43} // PAL  1:1 4:3 16:9
                            };
#define aprintf

resizeWindow::resizeWindow(QWidget *parent, resParam *param) : QDialog(parent)
 {
     ui.setupUi(this);
	 lastPercentage = 100;
     _param=param;
     ui.spinBoxWidth->setValue(_param->rsz.width);
     ui.spinBoxHeight->setValue(_param->rsz.height);
     ui.horizontalSlider->setValue(100);
	 ui.comboBoxAlgo->setCurrentIndex(_param->rsz.algo);
     ui.comboBoxSource->setCurrentIndex(_param->rsz.sourceAR);
     ui.comboBoxDestination->setCurrentIndex(_param->rsz.targetAR);
     updateWidthHeightSpinners();

	 connect(ui.comboBoxSource, SIGNAL(currentIndexChanged(int)), this, SLOT(aspectRatioChanged(int)));
	 connect(ui.comboBoxDestination, SIGNAL(currentIndexChanged(int)), this, SLOT(aspectRatioChanged(int)));
	 connect(ui.checkBoxRoundup, SIGNAL(toggled(bool)), this, SLOT(roundupToggled(bool)));
	 connect(ui.lockArCheckBox, SIGNAL(toggled(bool)), this, SLOT(lockArToggled(bool)));
	 connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));

	 connectDimensionControls();
 }

 void resizeWindow::gather(void)
 {
    _param->rsz.width=ui.spinBoxWidth->value();
    _param->rsz.height=ui.spinBoxHeight->value();
    _param->rsz.algo=ui.comboBoxAlgo->currentIndex();
    _param->rsz.sourceAR=ui.comboBoxSource->currentIndex();
    _param->rsz.targetAR=ui.comboBoxDestination->currentIndex();
 }
 
 void resizeWindow::sliderChanged(int value)
{
	disconnectDimensionControls();

	percentageSpinBoxChanged(value);

	connectDimensionControls();
}

void resizeWindow::percentageSpinBoxChanged(int value)
{
	disconnectDimensionControls();

	if (ui.checkBoxRoundup->isChecked())
	{
		if (lastPercentage > value)
			ui.spinBoxWidth->setValue(ui.spinBoxWidth->value() - 16);
		else
			ui.spinBoxWidth->setValue(ui.spinBoxWidth->value() + 16);
	}
	else
	{
		float width = _param->originalWidth;

		width /= 100.;
		width *= value;

		ui.spinBoxWidth->setValue(floor(width + 0.5));
	}

	updateWidthHeightSpinners(false);

	lastPercentage = ui.percentageSpinBox->value();

	connectDimensionControls();
}

void resizeWindow::widthSpinBoxChanged(int value)
{
	disconnectDimensionControls();

	if (ui.lockArCheckBox->isChecked())
		updateWidthHeightSpinners(false);

	connectDimensionControls();
}

void resizeWindow::heightSpinBoxChanged(int value)
{
	disconnectDimensionControls();

	if (ui.lockArCheckBox->isChecked())
		updateWidthHeightSpinners(true);

	connectDimensionControls();
}

void resizeWindow::aspectRatioChanged(int index)
{
	disconnectDimensionControls();

	if (ui.lockArCheckBox->isChecked())
		updateWidthHeightSpinners();

	connectDimensionControls();
}

void resizeWindow::disconnectDimensionControls()
{
	disconnect(ui.spinBoxHeight, SIGNAL(valueChanged(int)), this, SLOT(heightSpinBoxChanged(int)));
	disconnect(ui.spinBoxWidth, SIGNAL(valueChanged(int)), this, SLOT(widthSpinBoxChanged(int)));
	disconnect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	disconnect(ui.percentageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(percentageSpinBoxChanged(int)));
}

void resizeWindow::connectDimensionControls()
{
	connect(ui.spinBoxHeight, SIGNAL(valueChanged(int)), this, SLOT(heightSpinBoxChanged(int)));
	connect(ui.spinBoxWidth, SIGNAL(valueChanged(int)), this, SLOT(widthSpinBoxChanged(int)));
	connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
	connect(ui.percentageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(percentageSpinBoxChanged(int)));
}

void resizeWindow::updateWidthHeightSpinners(bool useHeightAsRef)
{
	int sar = ui.comboBoxSource->currentIndex();
	int dar = ui.comboBoxDestination->currentIndex();
	float x = ui.spinBoxWidth->value();
	float y = ui.spinBoxHeight->value();
	float sr_mul = 1.;
	float dst_mul = 1.;
	float ar = 1.;

	if (sar)
	{  // source is 4/3 or 16/9
		sr_mul = aspectRatio[_param->pal][sar];
	}

	if (dar)
	{  // dst is 4/3 or 16/9
		dst_mul = 1 / aspectRatio[_param->pal][dar];
	}

	aprintf("source mul %02.2f , dst mul: %02.2f\n", sr_mul, dst_mul);

	ar = _param->originalWidth / (_param->originalHeight / (sr_mul * dst_mul));

	if (useHeightAsRef)
		x = y * ar;
	else
		y = x / ar;

	aprintf("x, y: %03f, %03f\n", x, y);

	int xx = floor(x + 0.5);
	int yy = floor(y + 0.5);

	if (xx & 1)
		xx--;

	if (yy & 1)
		yy--;

	roundUp(xx, yy);
	updateSlider();
}

void resizeWindow::updateSlider()
{
	float percent = (((float)ui.spinBoxWidth->value() / (float)_param->originalWidth) * 100.) + 0.5;

	ui.horizontalSlider->setValue(percent);
	ui.percentageSpinBox->setValue(percent);
}

void resizeWindow::roundUp(int xx, int yy)
{
	float erx = 0.;
	float ery = 0.;

	if (ui.checkBoxRoundup->checkState())
	{
		int ox = xx, oy = yy;

		xx = (xx + 7) & 0xfffff0;
		yy = (yy + 7) & 0xfffff0;

		erx = xx - ox;
		erx = erx / xx;
		ery = yy - oy;
		ery = ery / yy;

		aprintf("x: %d -> %d : err %f\n", ox, xx, erx);
		aprintf("y: %d -> %d : err %f\n", oy, yy, ery);
	}

	ui.spinBoxWidth->setValue(xx);
	ui.spinBoxHeight->setValue(yy);

	ui.labelErrorXY->setText(QString("%1").arg(erx * 100., 0, 'f', 2) + " / " + QString("%1").arg(ery * 100., 0, 'f', 2));
}

void resizeWindow::lockArToggled(bool toggled)
{
	if (ui.lockArCheckBox->isChecked())
		widthSpinBoxChanged(0);
	else
		ui.checkBoxRoundup->setChecked(false);
}

void resizeWindow::roundupToggled(bool toggled)
{
	if (toggled)
	{
		disconnectDimensionControls();

		ui.spinBoxWidth->setSingleStep(16);
		ui.spinBoxHeight->setSingleStep(16);
		widthSpinBoxChanged(0);

		connectDimensionControls();
	}
	else
	{
		ui.spinBoxWidth->setSingleStep(2);
		ui.spinBoxHeight->setSingleStep(2);
	}
}

void resizeWindow::okButtonClicked()
{
	if (ui.spinBoxWidth->value() & 1 || ui.spinBoxHeight->value() & 1)
		GUI_Error_HIG(QT_TRANSLATE_NOOP("resize","Width and height cannot be odd"), NULL);
	else
		accept();
}

/**
    \fn DIA_resize
    \brief Handle resize dialo
*/
bool         DIA_resize(uint32_t originalWidth,uint32_t originalHeight,uint32_t fps1000,swresize *resize)
{
bool r=false;
      resParam param={  
                        originalWidth,
                        originalHeight,
                        fps1000,
                        0,
                        *resize
                    };
      //
      if(fps1000>24600 && fps1000<25400)
        {
                param.pal=1;
        }
       

     // Fetch info
     resizeWindow resizewindow(qtLastRegisteredDialog(), &param);

     qtRegisterDialog(&resizewindow);

     if(resizewindow.exec()==QDialog::Accepted)
     {
       resizewindow.gather();
       *resize=param.rsz;
       r=true;
     }

	 qtUnregisterDialog(&resizewindow);

     return r;
}  
//********************************************
//EOF
