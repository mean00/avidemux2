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

#include "Q_reverse.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "ADM_toolkitQt.h"
#include "ADM_vidReverse.h"
#include "ADM_vidMisc.h"
#include "DIA_factory.h"
#include "ADM_last.h"
#include <inttypes.h>

#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif

reverseWindow::reverseWindow(QWidget *parent, reverse *param, ADM_coreVideoFilter *in) : QDialog(parent)
{
    ui.setupUi(this);
    _param=param;

    markerA = in->getInfo()->markerA;
    markerB = in->getInfo()->markerB;
    duration = in->getInfo()->totalDuration;
    bytesPerSec = in->getInfo()->width;
    bytesPerSec *= in->getInfo()->height;
    bytesPerSec *= 1.5;	// YUV420
    double fps = 1000000.0 / (double)in->getInfo()->frameIncrement;	//frameIncrement; /// Average delta time between 2 frames in useconds ~ 1/fps
    bytesPerSec *= fps;
    bytesPerSec *= 1.1;	///margin

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(okButtonClicked()));

    connect(ui.pushButtonTManual,SIGNAL(clicked(bool)),this,SLOT(manualTimeEntry(bool)));
    connect(ui.pushButtonTMarker,SIGNAL(clicked(bool)),this,SLOT(timesFromMarkers(bool)));

    admCoreUtils::getLastReadFile(filePath);
    
    QString warning_text = QString(QT_TRANSLATE_NOOP("reverse","THIS FILTER SUFFER A FEW LIMITATION:"));
    warning_text += QString("\n•");
    warning_text += QString(QT_TRANSLATE_NOOP("reverse","The start time MUST match a presentation timestamp (PTS) of a frame. Recommended to use the markers. Avoid filters that change timing and/or FPS."));
    warning_text += QString("\n•");
    warning_text += QString(QT_TRANSLATE_NOOP("reverse","This filter requires to buffer the entire section to be reversed, a temporary file will be saved in the same directory as the source video. Make sure there is enough space, and write premission."));
    warning_text += QString("\n•");
    warning_text += QString(QT_TRANSLATE_NOOP("reverse","The preview of the reverseal is ONLY possible, if you start palying before (or exactly at the start of) the time scope. Else you will see a green screen."));
    warning_text += QString("\n•");
    warning_text += QString(QT_TRANSLATE_NOOP("reverse","When encoding or play in preview reaches the filter's start time, the application may appear unresponsive, while buffering the section. Be patient."));
    ui.textEditWarnings->setText(warning_text);

    printInfo();

}

void reverseWindow::gather(void)
{
    _param->fileBuffer = ui.lineEditFileName->text().toUtf8().constData();
}

void reverseWindow::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    QFontMetrics fm = ui.labelTScope->fontMetrics();
    QString text = QString(QT_TRANSLATE_NOOP("reverse","Time scope: "));
    text += QString("000:00:00,000 - 000:00:00,000");
    ui.labelTScope->setMinimumWidth(1.05 * fm.boundingRect(text).width());
    text = QString(QT_TRANSLATE_NOOP("reverse","Duration: "));
    text += QString("000:00:00,000---");
    ui.labelDuration->setMinimumWidth(1.05 * fm.boundingRect(text).width());

    adjustSize();
}

void reverseWindow::manualTimeEntry(bool f)
{
    uint32_t mx=(uint32_t)(duration/1000LL);

    diaElemTimeStamp start(&(_param->startTime),QT_TRANSLATE_NOOP("reverse","_Start time:"),0,mx);
    diaElemTimeStamp end(&(_param->endTime),QT_TRANSLATE_NOOP("reverse","_End time:"),0,mx);
    diaElem *elems[2]={&start,&end};

    if(diaFactoryRun(QT_TRANSLATE_NOOP("reverse","Manual time entry"),2+0*1,elems))
    {
        if(_param->startTime > _param->endTime)
        {
            uint32_t tmp=_param->startTime;
            _param->startTime=_param->endTime;
            _param->endTime=tmp;
        }
        
        printInfo();
    }
}

void reverseWindow::timesFromMarkers(bool f)
{
    _param->startTime = markerA / 1000LL;
    _param->endTime = markerB / 1000LL;
    if(_param->startTime > _param->endTime)
    {
        uint32_t tmp=_param->startTime;
        _param->startTime=_param->endTime;
        _param->endTime=tmp;
    }
    
    printInfo();
}

void reverseWindow::valueChanged(int f)
{
    printInfo();
}


void reverseWindow::printInfo()
{

    QString tstr = QString(QT_TRANSLATE_NOOP("reverse","Time scope: "));
    tstr += QString(ADM_us2plain(_param->startTime*1000LL));
    tstr += QString(" - ");
    tstr += QString(ADM_us2plain(_param->endTime*1000LL));
    ui.labelTScope->setText(tstr);

    uint32_t durationMs = (_param->endTime - _param->startTime);

    tstr = QString(QT_TRANSLATE_NOOP("reverse","Duration: "));
    tstr += QString(ADM_us2plain(durationMs*1000LL));
    ui.labelDuration->setText(tstr);
    
    double GBytes = ((bytesPerSec * durationMs) / 1000.0) / 1073741824.0;
    char GBstr[16];
    snprintf(GBstr, 16, "%.03f",GBytes);
    ui.labelStorage->setTextFormat(Qt::RichText);
    tstr = QString("<html><head/><body><p><span style=\" font-size:14pt; font-weight:600;\">");
    tstr += QString(QT_TRANSLATE_NOOP("reverse","Required storage for buffering: "));
    tstr += QString(GBstr);
    tstr += QString(QT_TRANSLATE_NOOP("reverse"," GiB"));
    tstr += QString("</span></p><hr></body></html>");
    ui.labelStorage->setText(tstr);
    
    QString fn = QString::fromStdString(filePath);
    fn += QString(".");
    char tshex[17];
    snprintf(tshex,17, "%08" PRIX32 "%08" PRIX32, _param->startTime, _param->endTime);
    fn += QString(tshex);
    fn += QString(".rev");
    ui.lineEditFileName->clear();
    ui.lineEditFileName->insert(fn);
    ui.lineEditFileName->setDisabled(false);
}


void reverseWindow::okButtonClicked()
{
   bool fileError = false;

    fileError = true;
    std::string fn = ui.lineEditFileName->text().toUtf8().constData();
    FILE * fd = ADM_fopen(fn.c_str(), "w+");
    if (fd)
    {
        if (ADM_fwrite("writetest16bytes",1,16,fd) == 16)	// successfull write
            fileError = false;
        ADM_fclose(fd);
        if (!fileError)
        {
            fileError = true;
            fd = ADM_fopen(fn.c_str(), "w+");
            if (fd)
            {
                fileError = false;
                ADM_fclose(fd);
            }
        }
    }

    if (fileError)
        GUI_Error_HIG(QT_TRANSLATE_NOOP("reverse","Could not create or write the buffer file!"), NULL);
    else
        accept();
}

/**
    \fn DIA_reverse
*/
bool DIA_reverse(reverse *param, ADM_coreVideoFilter *in)
{
    bool r=false;

    // Fetch info
    reverseWindow reverseWindow(qtLastRegisteredDialog(), param, in);

    qtRegisterDialog(&reverseWindow);

    if(reverseWindow.exec()==QDialog::Accepted)
    {
        reverseWindow.gather();
        r=true;
    }

    qtUnregisterDialog(&reverseWindow);

    return r;
}
//********************************************
//EOF
