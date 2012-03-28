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
#include "ADM_toolkitQt.h"

/**
    \fn qShell
*/
qShell::qShell(QWidget *parent, IScriptEngine *engine) : QDialog(parent)
{
    ADM_info("Setting up JS shell..\n");
    _engine = engine;
    for(int i=0;i<Q_SHELL_HISTORY;i++)
                    history[i]=NULL;

    ui.setupUi(this);
    ui.textBrowser_2->installEventFilter(this);
    connect((ui.evalute),SIGNAL(clicked(bool)),this,SLOT(evaluate(bool)));
    connect((ui.clear),SIGNAL(clicked(bool)),this,SLOT(clear(bool)));
    print(IScriptEngine::Information, "Enter your commands then press the evaluate button or CTRL+ENTER.\n");
    print(IScriptEngine::Information, "You can use CTRL+PageUP and CTRL+Page Down to recall previous commands\nReady.\n");
    indexRead=indexWrite=0;
}
/**
    \fn ~qShell
*/
qShell::~qShell()
{
    ADM_info("Destroying JS shell..\n");
    for(int i=0;i<Q_SHELL_HISTORY;i++)
    {
            if(history[i]) delete history[i];
            history[i]=NULL;
    }

}
bool qShell::run(void)
{
    this->exec();
    return true;
}
/**
        \fn evaluate
        \brief callback for the evaluate button
*/
bool            qShell::evaluate(bool x)
{
    ADM_info("Evaluating...\n");
    // 1 Get text from UI
    QString text=ui.textBrowser_2->toPlainText();
    int dex=indexWrite & (Q_SHELL_HISTORY-1);
    if(history[dex]) delete history[dex];
    history[dex]=new QString(text);
    indexWrite++;
    indexRead=indexWrite-1; // Points to the last one

    ui.textBrowser->setFontItalic(true);
    ui.textBrowser->append(text);
    ui.textBrowser->setFontItalic(false);
    ui.textBrowser_2->setPlainText("");
    _engine->runScript(text.toAscii().constData(), IScriptEngine::Normal);
    return true;
}
/**
    \fn print
*/
bool qShell::print(IScriptEngine::EventType type,const char *s)
{
    QString string(s);

    switch(type)
    {
        case IScriptEngine::Information: ui.textBrowser->setTextColor(QColor(0,0,0));break;
        case IScriptEngine::Error: ui.textBrowser->setTextColor(QColor(255,0,0));break;
    }
    ui.textBrowser->insertPlainText(string);
    ui.textBrowser->setTextColor(QColor(0,0,0));
    return true;
}
/**
    \fn clear
*/
bool qShell::clear(bool x)
{
    ui.textBrowser->clear();
    return true;
}
/**
    \fn eventFilter
    \brief
*/
bool qShell::eventFilter(QObject* watched, QEvent* event)
{
	if(event->type()==QEvent::KeyPress)
    {
			QKeyEvent *keyEvent = (QKeyEvent*)event;
            if(keyEvent->modifiers() == Qt::ControlModifier)
                switch (keyEvent->key())
                {
                    case Qt::Key_PageUp:   previousCommand();return true;break;
                    case Qt::Key_PageDown: nextCommand();return true;break;
                    default:break;
                }
    }
    return QObject::eventFilter(watched, event);
}
/**
    \fn previousCommand
*/
bool   qShell::previousCommand(void)
{
    int dex=indexRead & (Q_SHELL_HISTORY-1);
    QString *copy=history[dex];
    if(indexRead) indexRead--;
    if(!copy) return true;
    ui.textBrowser_2->clear();
    ui.textBrowser_2->append(*copy);
    return true;
}
/**
    \fn nextCommand
*/
bool   qShell::nextCommand(void)
{
    if(indexRead+1<indexWrite) indexRead++;
            else return true;
    int dex=indexRead & (Q_SHELL_HISTORY-1);
    QString *copy=history[dex];
    if(!copy) return true;
    ui.textBrowser_2->clear();
    ui.textBrowser_2->append(*copy);
    return true;
}
//EOF


