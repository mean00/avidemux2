/***************************************************************************

    \file Q_shell.cpp
    \brief UI for qt4 shell interface
    \author mean, fixount@free.fr 2007/2009
        
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <vector>
#include "ADM_inttype.h"
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QKeyEvent>
#include <QtGui/QGraphicsView>
#include "Q_shell.h"
#include "ADM_default.h"
#include "ui_shell.h"

qShell::qShell(jsShellEvaluate *s) : QDialog()
{
    ADM_info("Setting up JS shell..\n");
    evaluator=s;
    ui.setupUi(this);
    connect((ui.evalute),SIGNAL(clicked(bool)),this,SLOT(evaluate(bool)));
}

qShell::~qShell()
{
    ADM_info("Destroying JS shell..\n");
} 
bool qShell::run(void)
{
    this->exec();
    return true;
}
bool            qShell::evaluate(bool x)
{
    ADM_info("Evaluating...\n");
    // 1 Get text from UI
    QString text=ui.textBrowser_2->toPlainText();
    ui.textBrowser->append(text);
    ui.textBrowser_2->setPlainText("");
    evaluator(text.toAscii());
    return true;
}
//EOF


