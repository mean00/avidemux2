/***************************************************************************
                                Q_log.cpp
                                -----------

    copyright            : (C) 2021 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include "ADM_inttype.h"
#include "Q_log.h"
#include "ADM_toolkitQt.h"
#include <QScrollBar>

Ui_logWindow::Ui_logWindow(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
        const char * log = ADM_debugLogRead();
        if (log==NULL)
            log="error";
        QFont font("Monospace");
        font.setPointSize(8);
        font.setStyleHint(QFont::TypeWriter);
        QTextDocument *doc = ui.logTextEdit->document();
        doc->setDefaultFont(font);
        ui.logTextEdit->setLineWrapMode(QTextEdit::NoWrap);
	ui.logTextEdit->setPlainText(QString(log));
        ui.logTextEdit->moveCursor(QTextCursor::End);
        ui.logTextEdit->ensureCursorVisible();
}

void DIA_log(void)
{
	Ui_logWindow dialog(qtLastRegisteredDialog());

	qtRegisterDialog(&dialog);
	dialog.exec();
	qtUnregisterDialog(&dialog);
}
