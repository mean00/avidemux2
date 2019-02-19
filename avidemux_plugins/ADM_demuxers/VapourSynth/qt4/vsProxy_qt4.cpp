/**
         \file Simple proxy for vsProxy
         \brief external job control
         \author mean fixounet@free.fr (c) 2015
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QFile>
#include <QFileDialog>

#include "vsProxy_qt4.h"
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_threads.h"
#ifndef __APPLE__
#include "ADM_memsupport.h"
#endif
#include "ADM_crashdump.h"
#include "prefs.h"
#include "ADM_last.h"
#include "../ADM_vsProxy.h"
#ifdef _WIN32
#include "ADM_win32.h"
#endif
/**
    \fn main
*/
int main(int argc, char *argv[])
{
#ifndef __APPLE__
    ADM_InitMemcpy();
#endif
    ADM_initBaseDir(argc,argv);
    initPrefs();
    if(!prefs->load())
        ADM_warning("Cannot load preferences.\n");
#ifdef _WIN32
    win32_netInit();
#endif
    QApplication app(argc, argv);
    vsWindow vs;
    vs.show();
    app.exec();
    // We don't save prefs to avoid overwriting Avidemux configuration
    // with an older copy at the cost of not retaining the location of
    // the last loaded .vpy file across sessions.
    destroyPrefs();
    return 0;
}
/**
 * 
 */
vsWindow::vsWindow()
{
    ui.setupUi(this);
    ui.lineFile->setEnabled(false);
    ui.pushButtonRun->setEnabled(false);
    connect( ui.pushButtonRun,SIGNAL(clicked(bool)),this,SLOT(runOrStop()));
    connect( ui.pushFileSel,SIGNAL(clicked(bool)),this,SLOT(selectFile()));
    
}
/**
 * 
 */
vsWindow::~vsWindow()
{
    
}
/**
 * 
 */
void                vsWindow::selectFile()
{
    std::string dir;
    admCoreUtils::getLastReadFolder(dir);
    QString last = QString::fromStdString(dir);
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open VapourSynth File"), last, tr("VS Script Files (*.vpy)"));
    printf("File selected : %s\n",fileName.toUtf8().constData());
    if(!fileName.size()) 
        return;
    QFile qfile(fileName);
    if(!qfile.exists())
    {
        ui.lineFile->setText(QString(""));
        ui.pushButtonRun->setEnabled(false);
    }else
    {
        ui.lineFile->setText(fileName);
        ui.labelStatus->setText(QString("..."));
        ui.pushButtonRun->setEnabled(true);
        admCoreUtils::setLastReadFolder(std::string(fileName.toUtf8().constData()));
    }
}
/**
 * 
 */
void                vsWindow::runOrStop()
{
    ui.pushFileSel->setEnabled(false);
    ui.pushButtonRun->setEnabled(false);
    ui.spinboxPort->setEnabled(false);
    ui.labelStatus->setText(QString("Running...."));
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    std::string fileName=std::string(ui.lineFile->text().toUtf8().constData());
    vapourSynthProxy vs;
    vs.run(ui.spinboxPort->value(),fileName.c_str());

    ui.labelStatus->setText(QString("Exited (error ?)"));
    ui.spinboxPort->setEnabled(true);
    ui.pushButtonRun->setEnabled(true);
    ui.pushFileSel->setEnabled(true);
}

//EOF
