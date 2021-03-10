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

#include <cmath>

#include "Q_fitToSize.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidFitToSize.h"

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

fitToSizeWindow::fitToSizeWindow(QWidget *parent, resParam *param) : QDialog(parent)
{
    ui.setupUi(this);
    _param=param;

    ui.comboBoxRoundup->setCurrentIndex(_param->rsz.roundup);

    ui.spinBoxWidth->setKeyboardTracking(false);
    ui.spinBoxHeight->setKeyboardTracking(false);
    ui.percentageSpinBox->setKeyboardTracking(false);

    ui.spinBoxWidth->setValue(_param->rsz.width & 0xfffffe);
    ui.spinBoxHeight->setValue(_param->rsz.height & 0xfffffe);
    ui.horizontalSlider->setValue(round(_param->rsz.tolerance*100.0));
    ui.percentageSpinBox->setValue(round(_param->rsz.tolerance*100.0));
    ui.comboBoxAlgo->setCurrentIndex(_param->rsz.algo);
    ui.comboBoxPad->setCurrentIndex(_param->rsz.pad);
    roundupChanged(_param->rsz.roundup);


    connect(ui.comboBoxRoundup, SIGNAL(currentIndexChanged(int)), this, SLOT(roundupChanged(int)));
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));

    ui.labelInputSize->setText(" " + QString("%1 x %2").arg(_param->originalWidth).arg(_param->originalHeight));
    printInfo();

    connectDimensionControls();
}

void fitToSizeWindow::gather(void)
{
    _param->rsz.width=ui.spinBoxWidth->value();
    _param->rsz.height=ui.spinBoxHeight->value();
    _param->rsz.algo=ui.comboBoxAlgo->currentIndex();
    _param->rsz.pad=ui.comboBoxPad->currentIndex();
    _param->rsz.roundup=ui.comboBoxRoundup->currentIndex();
    _param->rsz.tolerance = (float)(ui.percentageSpinBox->value())/100.0;
}

void fitToSizeWindow::printInfo()
{
    int ox,oy;
    ox = _param->originalWidth;
    oy = _param->originalHeight;
    int x,y;
    x = ui.spinBoxWidth->value() & 0xfffffe;
    y = ui.spinBoxHeight->value() & 0xfffffe;
    float tolerance;
    tolerance = (float)(ui.percentageSpinBox->value())/100.0;

    float inAR;
    inAR  = (float)ox/(float)oy;

    int sx,sy;
    int pleft,pright,ptop,pbot;

    ADMVideoFitToSize::getFitParameters(ox, oy, x, y, tolerance, &sx, &sy, &pleft, &pright, &ptop, &pbot);

    float stretchError;
    stretchError = (((float)sx/(float)sy)/inAR - 1.0)*100.0;

    ui.labelStretchSize->setText(" " + QString("%1 x %2").arg(sx).arg(sy));
    ui.labelStretchError->setText(QString::fromUtf8("\u0394") + " = " + QString("%1%2").arg(stretchError < 0.0 ? '-' : '+').arg(fabs(stretchError), 0, 'f', 2) + "%");
    ui.labelPadding->setText(" " + QString("[%1,..,%2] x [%3,..,%4]").arg(pleft).arg(pright).arg(ptop).arg(pbot));
}

void fitToSizeWindow::sliderChanged(int value)
{
    disconnectDimensionControls();

    ui.percentageSpinBox->setValue(value);
    printInfo();

    connectDimensionControls();
}

void fitToSizeWindow::percentageSpinBoxChanged(int value)
{
    disconnectDimensionControls();

    ui.horizontalSlider->setValue(value);
    printInfo();

    connectDimensionControls();
}

void fitToSizeWindow::dimensionSpinBoxChanged(int value)
{
    disconnectDimensionControls();

    roundUp();
    printInfo();

    connectDimensionControls();
}

void fitToSizeWindow::disconnectDimensionControls()
{
    disconnect(ui.spinBoxHeight, SIGNAL(valueChanged(int)), this, SLOT(dimensionSpinBoxChanged(int)));
    disconnect(ui.spinBoxWidth, SIGNAL(valueChanged(int)), this, SLOT(dimensionSpinBoxChanged(int)));
    disconnect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    disconnect(ui.percentageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(percentageSpinBoxChanged(int)));
}

void fitToSizeWindow::connectDimensionControls()
{
    connect(ui.spinBoxHeight, SIGNAL(valueChanged(int)), this, SLOT(dimensionSpinBoxChanged(int)));
    connect(ui.spinBoxWidth, SIGNAL(valueChanged(int)), this, SLOT(dimensionSpinBoxChanged(int)));
    connect(ui.horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    connect(ui.percentageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(percentageSpinBoxChanged(int)));
}

void fitToSizeWindow::roundUp()
{
    int x,y;
    x = ui.spinBoxWidth->value() & 0xfffffe;
    y = ui.spinBoxHeight->value() & 0xfffffe;

    int roundup_index = ui.comboBoxRoundup->currentIndex();

    if (roundup_index > 0)
    {
        float mod = (32 >> roundup_index);

        x = (int)(round((float)x/mod)*mod);
        y = (int)(round((float)y/mod)*mod);
    }

    if (x < 16) x = 16;
    if (y < 16) y = 16;
    ui.spinBoxWidth->setValue(x);
    ui.spinBoxHeight->setValue(y);
}

void fitToSizeWindow::roundupChanged(int index)
{
    disconnectDimensionControls();
    if (index > 0)
    {

        ui.spinBoxWidth->setSingleStep((16 << 1) >> index);
        ui.spinBoxHeight->setSingleStep((16 << 1) >> index);
        roundUp();

    }
    else
    {
        ui.spinBoxWidth->setSingleStep(2);
        ui.spinBoxHeight->setSingleStep(2);
    }
    printInfo();
    connectDimensionControls();
}

void fitToSizeWindow::okButtonClicked()
{
    if (ui.spinBoxWidth->value() & 1 || ui.spinBoxHeight->value() & 1)
        GUI_Error_HIG(QT_TRANSLATE_NOOP("fitToSize","Width and height cannot be odd"), NULL);
    else
        accept();
}

/**
    \fn DIA_fitToSize
*/
bool DIA_fitToSize(uint32_t originalWidth,uint32_t originalHeight,fitToSize *param)
{
    bool r=false;
    resParam _param={
                        originalWidth,
                        originalHeight,
                        *param
                   };


    // Fetch info
    fitToSizeWindow fitToSizeWindow(qtLastRegisteredDialog(), &_param);

    qtRegisterDialog(&fitToSizeWindow);

    if(fitToSizeWindow.exec()==QDialog::Accepted)
    {
        fitToSizeWindow.gather();
        *param=_param.rsz;
        r=true;
    }

    qtUnregisterDialog(&fitToSizeWindow);

    return r;
}
//********************************************
//EOF
