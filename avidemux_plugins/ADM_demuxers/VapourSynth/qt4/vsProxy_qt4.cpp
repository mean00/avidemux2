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
#include <QtCore/QFile>
#include <QFileDialog>

#include "vsProxy_qt4.h"
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_threads.h"
#include "ADM_memsupport.h"
#include "ADM_crashdump.h"
#include "../ADM_vsProxy.h"

/**
    \fn main
*/
int main(int argc, char *argv[])
{
    ADM_InitMemcpy();
#ifdef _WIN32
    win32_netInit();
#endif
    QApplication app(argc, argv);
    vsWindow vs;
    vs.show();
    app.exec();
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
    QString fileName = QFileDialog::getOpenFileName(this,
         tr("Open VapourSynth File 1"), "", tr("VS File Files (*.vpy)"));
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
        ui.pushButtonRun->setEnabled(true);
    }
    
}
/**
 * 
 */
void                vsWindow::runOrStop()
{
    
     ui.pushButtonRun->setEnabled(false);
     ui.labelStatus->setText(QString("Running...."));
     std::string fileName=std::string(ui.lineFile->text().toUtf8().constData());
     vapourSynthProxy vs;
                      
     vs.run(9999,fileName.c_str());
     ui.labelStatus->setText(QString("Exited (error ?)"));
}

//EOF
