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

static double aspectRatio[2][5]={
    {
        1.,
        0.888889, // NTSC 720x480 DAR 4:3 PAR 8:9
        0.909091, // NTSC 704x480 DAR 4:3 PAR 10:11
        1.185185, // NTSC 720:480 DAR 16:9 PAR 32:27
        1.212121  // NTSC 704:480 DAR 16:9 PAR 40:33
    },
    {
        1.,
        1.066667, // PAL 720:576 DAR 4:3 PAR 16:15
        1.090909, // PAL 704:576 DAR 4:3 PAR 12:11
        1.422222, // PAL 720:576 DAR 16:9 PAR 64:45
        1.454545  // PAL 704:576 DAR 16:9 PAR 16:11
    }
};

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

resizeWindow::resizeWindow(QWidget *parent, resParam *param) : QDialog(parent)
{
    ui.setupUi(this);
    lastPercentage = 100;
    labelOutArFWidth = labelOutArFWidth10 = 0;
    _param=param;

#define ADD_PAR(x) ui.comboBoxSource->addItem(x); ui.comboBoxDestination->addItem(x);

    if(_param->pal)
    {
        ADD_PAR("PAL 720:576 DAR 4:3 PAR 16:15")
        ADD_PAR("PAL 704:576 DAR 4:3 PAR 12:11")
        ADD_PAR("PAL 720:576 DAR 16:9 PAR 64:45")
        ADD_PAR("PAL 704:576 DAR 16:9 PAR 16:11")
    }else
    {
        ADD_PAR("NTSC 720x480 DAR 4:3 PAR 8:9")
        ADD_PAR("NTSC 704x480 DAR 4:3 PAR 10:11")
        ADD_PAR("NTSC 720:480 DAR 16:9 PAR 32:27")
        ADD_PAR("NTSC 704:480 DAR 16:9 PAR 40:33")
    }

    ui.lockArCheckBox->setChecked(_param->rsz.lockAR);

#define STR(x) #x
#define MKSTRING(x) STR(x)
#define CHECK_COMBO(x,y,z) if(_param->rsz.x>=ui.comboBox##y->count()) \
    { \
        ADM_warning("Invalid " MKSTRING(x) " index %d, using default = %d\n",_param->rsz.x,z); \
        _param->rsz.x = z; \
    } \
    ui.comboBox##y->setCurrentIndex(_param->rsz.x);
    CHECK_COMBO(roundup,Roundup,0)
    CHECK_COMBO(algo,Algo,1) // bicubic
    CHECK_COMBO(sourceAR,Source,0)
    CHECK_COMBO(targetAR,Destination,0)
#undef CHECK_COMBO

    ui.spinBoxWidth->setKeyboardTracking(false);
    ui.spinBoxHeight->setKeyboardTracking(false);
    ui.percentageSpinBox->setKeyboardTracking(false);

    ui.spinBoxWidth->setValue(_param->rsz.width & 0xfffffe);
    ui.spinBoxHeight->setValue(_param->rsz.height & 0xfffffe);
    ui.horizontalSlider->setValue(100);
    if(_param->rsz.lockAR)
        updateWidthHeightSpinners();
    enableControls(_param->rsz.lockAR);
    roundupChanged(_param->rsz.roundup);
    
    printOutAR(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

    connect(ui.comboBoxSource, SIGNAL(currentIndexChanged(int)), this, SLOT(aspectRatioChanged(int)));
    connect(ui.comboBoxDestination, SIGNAL(currentIndexChanged(int)), this, SLOT(aspectRatioChanged(int)));
    connect(ui.comboBoxRoundup, SIGNAL(currentIndexChanged(int)), this, SLOT(roundupChanged(int)));
    connect(ui.lockArCheckBox, SIGNAL(toggled(bool)), this, SLOT(lockArToggled(bool)));
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));

    connectDimensionControls();
}

void resizeWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    /* Avoid shifting the layout displaying output aspect ratio
    by setting a sufficient minimum width based on font metrics. */
    QFontMetrics fm = ui.labelOutArFloat->fontMetrics(); // we may assume that both labels use the same font
    QString text = "0.0000";
    labelOutArFWidth = 1.05 * fm.boundingRect(text).width();
    text = "00.0000";
    labelOutArFWidth10 = 1.05 * fm.boundingRect(text).width();
    ui.labelOutArFloat->setMinimumWidth(labelOutArFWidth);
    text = "(00:00)";
    ui.labelOutArDef->setMinimumWidth(1.05 * fm.boundingRect(text).width());
}

void resizeWindow::gather(void)
{
    _param->rsz.width=ui.spinBoxWidth->value();
    _param->rsz.height=ui.spinBoxHeight->value();
    _param->rsz.algo=ui.comboBoxAlgo->currentIndex();
    _param->rsz.sourceAR=ui.comboBoxSource->currentIndex();
    _param->rsz.targetAR=ui.comboBoxDestination->currentIndex();
    _param->rsz.lockAR=ui.lockArCheckBox->isChecked();
    _param->rsz.roundup=ui.comboBoxRoundup->currentIndex();
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

    float width = _param->originalWidth;

    width /= 100.;
    width *= value;
    width += 0.5;

    uint32_t iw = floor(width);

    int roundup_index = ui.comboBoxRoundup->currentIndex();

    if (roundup_index > 0)
    {
        int mask = (0xfffff0 << 1) >> roundup_index;
        int increment = (16 >> roundup_index) - 1;
        int rmod = (16 << 1) >> roundup_index;

        iw = (iw + increment) & mask;
        if ((int)iw == ui.spinBoxWidth->value())
        {
            if (lastPercentage > value)
                iw = iw >= 32 ? iw - rmod : rmod;
            else
                iw += rmod;
        }
    }

    ui.spinBoxWidth->setValue(iw);

    updateWidthHeightSpinners(false);

    lastPercentage = ui.percentageSpinBox->value();

    printOutAR(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

    connectDimensionControls();
}

void resizeWindow::widthSpinBoxChanged(int value)
{
    disconnectDimensionControls();

    if (ui.lockArCheckBox->isChecked())
        updateWidthHeightSpinners(false);
    else
        roundUp(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

    printOutAR(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

    connectDimensionControls();
}

void resizeWindow::heightSpinBoxChanged(int value)
{
    disconnectDimensionControls();

    if (ui.lockArCheckBox->isChecked())
        updateWidthHeightSpinners(true);
    else
        roundUp(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

    printOutAR(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

    connectDimensionControls();
}

void resizeWindow::aspectRatioChanged(int index)
{
    disconnectDimensionControls();

    if (ui.lockArCheckBox->isChecked())
        updateWidthHeightSpinners();

    printOutAR(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

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
    {  // source pixel aspect ratio is not 1:1
        sr_mul = aspectRatio[_param->pal][sar];
    }

    if (dar)
    {  // dst pixel aspect ratio is not 1:1
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

    int roundup_index = ui.comboBoxRoundup->currentIndex();

    if (roundup_index > 0)
    {
        int ox = xx, oy = yy;
        int mask = (0xfffff0 << 1) >> roundup_index;
        int increment = (16 >> roundup_index) - 1;

        xx = (xx + increment) & mask;
        yy = (yy + increment) & mask;

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

void resizeWindow::printOutAR(int w, int h)
{
    static const double arlist[][2] = {
        {9, 16}, {1, 1}, {6, 5}, {5, 4}, {4, 3}, {11, 8}, {1.43, 1}, {3, 2}, {14, 9}, {16, 10}, {5, 3}, {16, 9}, {1.85, 1}, {1.9, 1}, {2, 1}, {2.2, 1}, {64, 27}, {2.35, 1}, {2.39, 1}, {2.4, 1}, {2.76, 1}, {32, 9}, {18, 5}, {4, 1}
    };
    
    double xx = w;
    double yy = h;
    
    if (ui.lockArCheckBox->isChecked())
    {
        int sar = ui.comboBoxSource->currentIndex();
        int dar = ui.comboBoxDestination->currentIndex();
        if (sar)
            yy /= aspectRatio[_param->pal][sar];
        if (dar)
            yy /= aspectRatio[_param->pal][dar];
    }
    
    double outar = xx/yy;
    outar = round(outar*10000.0) / 10000.0;
    
    double mindiff = 9999;
    int minindex = 0;
    for (int i=0; i<sizeof(arlist) / sizeof(arlist[0]); i++)
    {
        double lsar = arlist[i][0] / arlist[i][1];
        if (mindiff > fabs(lsar - outar))
        {
            mindiff = fabs(lsar - outar);
            minindex = i;
        }
    }
    
    double lsar = arlist[minindex][0] / arlist[minindex][1];
    if (outar > lsar)
        mindiff = outar/lsar - 1.0;
    else
        mindiff = lsar/outar - 1.0;

    if (outar < 10.)
    {
        if (labelOutArFWidth > 0)
            ui.labelOutArFloat->setMinimumWidth(labelOutArFWidth);
    } else
    {
        if (labelOutArFWidth10 > 0)
            ui.labelOutArFloat->setMinimumWidth(labelOutArFWidth10);
    }

    QString arfloat = QString("%1").arg(outar, 0, 'f', 4);
    ui.labelOutArFloat->setText(arfloat);

    QString arstr;
    if (mindiff <= 0.005)
        arstr = QString("(%1:%2)").arg(arlist[minindex][0]).arg(arlist[minindex][1]);
    ui.labelOutArDef->setText(arstr);
}

void resizeWindow::lockArToggled(bool toggled)
{
    if (ui.lockArCheckBox->isChecked())
        widthSpinBoxChanged(0);
    /*else
        ui.comboBoxRoundup->setCurrentIndex(0);*/

    printOutAR(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());

    enableControls(toggled);
}

void resizeWindow::roundupChanged(int index)
{
    if (index > 0)
    {
        disconnectDimensionControls();

        ui.spinBoxWidth->setSingleStep((16 << 1) >> index);
        ui.spinBoxHeight->setSingleStep((16 << 1) >> index);
        widthSpinBoxChanged(0);

        connectDimensionControls();
    }
    else
    {
        ui.spinBoxWidth->setSingleStep(2);
        ui.spinBoxHeight->setSingleStep(2);
    }

    printOutAR(ui.spinBoxWidth->value(), ui.spinBoxHeight->value());
}

void resizeWindow::enableControls(bool lockArChecked)
{
#define ENABLE(x) ui.x->setEnabled(lockArChecked);

    ENABLE(label_3)
    ENABLE(label_4)
    ENABLE(label_5)
    ENABLE(label_8)
    ENABLE(label_9)
    ENABLE(label_10)
    ENABLE(horizontalSlider)
    ENABLE(percentageSpinBox)
    ENABLE(labelErrorXY)
    ENABLE(comboBoxSource)
    ENABLE(comboBoxDestination)
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
bool DIA_resize(uint32_t originalWidth,uint32_t originalHeight,uint32_t fps1000,swresize *resize)
{
    bool r=false;
    resParam param={
                        originalWidth,
                        originalHeight,
                        fps1000,
                        0,
                        *resize
                   };


    if((fps1000>24600 && fps1000<25400) || (fps1000>49200 && fps1000<50800))
        param.pal=1;

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
