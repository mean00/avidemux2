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
#include "ADM_QSettings.h"

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

fitToSizeWindow::fitToSizeWindow(QWidget *parent, resParam *param) : QDialog(parent)
{
    ui.setupUi(this);
    ui.pushButtonSwapDimensions->setText(QString::fromUtf8("\u21C4"));
    _param=param;

    if (_param->firstRun)
    {
        QSettings *qset = qtSettingsCreate();
        if(qset)
        {
            qset->beginGroup("fitToSize");
            _param->rsz.algo = qset->value("defaultAlgo", 1).toInt();
            _param->rsz.pad = qset->value("defaultPadding", 0).toInt();
            // sanitize
            if(_param->rsz.algo >= ui.comboBoxAlgo->count())
                _param->rsz.algo = 1;
            if(_param->rsz.pad >= ui.comboBoxPad->count())
                _param->rsz.pad = 0;
            qset->endGroup();
            delete qset;
            qset = NULL;
        }
    }    
    
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

    preferencesButton = ui.buttonBox->addButton(QT_TRANSLATE_NOOP("fitToSize","Preferences"),QDialogButtonBox::ResetRole);
    preferencesButton->setCheckable(true);
    connect(preferencesButton,SIGNAL(clicked(bool)),this,SLOT(setPreferences(bool)));
    connect(ui.pushButtonSwapDimensions,SIGNAL(clicked(bool)),this,SLOT(swapDimensions(bool)));
    
    connectDimensionControls();

#if !(defined(__APPLE__) && QT_VERSION >= QT_VERSION_CHECK(6,3,0))
    disconnect(ui.buttonBox, &QDialogButtonBox::accepted, this, &fitToSizeWindow::accept);
    disconnect(ui.buttonBox, &QDialogButtonBox::rejected, this, &fitToSizeWindow::reject);
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
#endif
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

void fitToSizeWindow::setPreferences(bool f)
{
    UNUSED_ARG(f);

    QSettings *qset = qtSettingsCreate();
    if(!qset)
    {
        preferencesButton->setChecked(false);
        return;
    }

    qset->beginGroup("fitToSize");

    QDialog dialog(preferencesButton);
    dialog.setWindowTitle(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Preferences")));

    QGroupBox *frameDefaults = new QGroupBox(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Defaults for new filter instances")));

    QLabel *textAlgo = new QLabel(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Resize method:")));

    QComboBox *saveAlgoComboBox = new QComboBox();
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Most recently accepted")),-1);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Bilinear")),0);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Bicubic")),1);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Lanczos")),2);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Spline")),3);
    saveAlgoComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Nearest Neighbor")),4);

    int userData = (qset->value("saveAlgo", 0).toInt() > 0)? -1 : qset->value("defaultAlgo", 1).toInt();

    for(int i = 0; i < saveAlgoComboBox->count(); i++)
    {
        if(userData != saveAlgoComboBox->itemData(i).toInt()) continue;
        saveAlgoComboBox->setCurrentIndex(i);
        break;
    }

    QLabel *textPaddingType = new QLabel(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Padding type:")));

    QComboBox *savePadComboBox = new QComboBox();
    savePadComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Most recently accepted")),-1);
    savePadComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Black Bars")),0);
    savePadComboBox->addItem(QString::fromUtf8(QT_TRANSLATE_NOOP("fitToSize","Echo")),1);

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

void fitToSizeWindow::swapDimensions(bool f)
{
    UNUSED_ARG(f);

    disconnectDimensionControls();

    uint32_t width = ui.spinBoxWidth->value();
    uint32_t height = ui.spinBoxHeight->value();
    ui.spinBoxWidth->setValue(height);
    ui.spinBoxHeight->setValue(width);

    roundUp();
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
bool DIA_fitToSize(uint32_t originalWidth,uint32_t originalHeight,fitToSize *param, bool firstRun)
{
    bool r=false;
    resParam _param={
                        originalWidth,
                        originalHeight,
                        *param,
                        firstRun
                   };


    // Fetch info
    fitToSizeWindow fitToSizeWindow(qtLastRegisteredDialog(), &_param);

    qtRegisterDialog(&fitToSizeWindow);

    if(fitToSizeWindow.exec()==QDialog::Accepted)
    {
        fitToSizeWindow.gather();
        QSettings *qset = qtSettingsCreate();
        if(qset)
        {
            qset->beginGroup("fitToSize");

            if (qset->value("saveAlgo", 0).toInt() == 1)
            {
                qset->setValue("defaultAlgo", _param.rsz.algo);
            }
            if (qset->value("savePad", 0).toInt() == 1)
            {
                qset->setValue("defaultPadding", _param.rsz.pad);
            }

            qset->endGroup();
            delete qset;
            qset = NULL;
        }        
        *param=_param.rsz;
        r=true;
    }

    qtUnregisterDialog(&fitToSizeWindow);

    return r;
}
//********************************************
//EOF
