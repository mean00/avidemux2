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
#include "ADM_inttype.h"
#include <QKeyEvent>
#include <QAction>

#ifdef __APPLE__
#include <QMenuBar>
#endif

#include "Q_shell.h"
#include "ADM_default.h"
#include "ui_shell.h"
#include "ADM_toolkitQt.h"

/**
    \fn qShell
*/
qShell::qShell(QWidget *parent, IScriptEngine *engine, std::vector <shellHistoryEntry> *commands) : QDialog(parent)
{
    ADM_info("Setting up shell for %s...\n",engine->name().c_str());
    _engine = engine;
    _history = commands;
    indexRead=indexWrite=0;
    for(int i=0;i<_history->size();i++)
    {
        shellHistoryEntry e=_history->at(i);
        if(e.name!=_engine->name()) continue;
        indexRead++;
        indexWrite++;
    }
    ui.setupUi(this);
    ui.textBrowser_2->installEventFilter(this);

    QAction *ev = new QAction(this);
    ev->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(ev,SIGNAL(triggered(bool)),this,SLOT(evaluate(bool)));
    addAction(ev);

    connect((ui.evalute),SIGNAL(clicked(bool)),this,SLOT(evaluate(bool)));
    connect((ui.clear),SIGNAL(clicked(bool)),this,SLOT(clear(bool)));
#ifndef __APPLE__
    print(IScriptEngine::Information, QT_TRANSLATE_NOOP("qshell","Enter your commands then press the evaluate button or CTRL+ENTER.\n"));
    print(IScriptEngine::Information, QT_TRANSLATE_NOOP("qshell","You can use CTRL+PageUP and CTRL+Page Down to recall previous commands\nReady.\n"));
#else
    print(IScriptEngine::Information, QT_TRANSLATE_NOOP("qshell","Enter your commands then press the evaluate button or ⌘⏎.\n"));
    print(IScriptEngine::Information, QT_TRANSLATE_NOOP("qshell","You can use ⌥⌘▲ and ⌥⌘▼ to recall previous commands.\nReady.\n"));

    /* On macOS, global menus containing actions with keyboard shortcuts assigned
    are highlighted if corresponding keys are pressed. This happens even if menus
    belong to the parent of a modal dialog, i.e. when all actions in a menu are
    disabled and no key events are propagated to the parent. Add an empty menu bar
    which will replace the menu bar from the main window as a crude hack to avoid
    the "Go" menu flashing on every keypress of an arrow key. */
    QMenuBar *dummyBar = new QMenuBar(this);
#endif
#ifdef SCRIPT_SHELL_HISTORY_VERBOSE
    dumpHistory();
#endif
}
/**
    \fn ~qShell
*/
qShell::~qShell()
{
    ADM_info("Destroying JS shell..\n");
}
/**
    \fn run
*/
bool qShell::run(void)
{
    this->exec();
    return true;
}
/**
    \fn addToHistory
*/
int qShell::addToHistory(QString &command)
{
    int r=0;
    int size=_history->size();
    // remove duplicates
    for(int i=0;i<size;i++)
    {
        shellHistoryEntry e=_history->at(size-i-1);
        if(e.name!=_engine->name())
            continue;
        if(*(e.command)==command)
        {
            if(!i) // nothing to do
                return r;
            QString *cmd=e.command;
            delete cmd;
            e.command=NULL;
            std::vector <shellHistoryEntry>::iterator it=_history->begin();
            it+=size-i-1;
            _history->erase(it);
            size--;
            r--;
        }
    }
    // append our entry
    shellHistoryEntry e;
    e.name=_engine->name();
    QString *cmd=new QString(command);
    e.command=cmd;
    _history->push_back(e);
    r++;
    return r;
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
    indexWrite+=addToHistory(text);
    if(indexWrite<1) indexWrite=1;
    indexRead=indexWrite-1; // Points to the last one

    ui.textBrowser->setFontItalic(true);
    ui.textBrowser->append(text);
    ui.textBrowser->setFontItalic(false);
    ui.textBrowser_2->setPlainText("");
    _engine->runScript(text.toUtf8().constData(), IScriptEngine::Normal);
#ifdef SCRIPT_SHELL_HISTORY_VERBOSE
    dumpHistory();
#endif
    return true;
}
#ifdef SCRIPT_SHELL_HISTORY_VERBOSE
/**
    \fn dumpHistory
*/
bool qShell::dumpHistory(void)
{
    if(_history->empty())
    {
        ADM_info("History is empty.\n");
        return true;
    }
    int count=0;
    for(int i=0;i<_history->size();i++)
    {
        shellHistoryEntry e=_history->at(i);
        if(e.name!=_engine->name())
            continue;
        QString *cmd=e.command;
        printf(" --> history entry %d: \"%s\"\n",count++,cmd->toUtf8().constData());
    }
    return true;
}
#endif
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
        default: break;
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
    \fn showEvent
    \brief set focus to text input
*/
void qShell::showEvent(QShowEvent *ev)
{
    ui.textBrowser_2->setFocus();
    QWidget::showEvent(ev);
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
#ifndef __APPLE__
        if(keyEvent->modifiers() == Qt::ControlModifier)
        {
            switch (keyEvent->key())
            {
                case Qt::Key_PageUp: previousCommand();return true;
                case Qt::Key_PageDown: nextCommand();return true;
                default:break;
            }
        }
#else
        if((keyEvent->modifiers() & Qt::AltModifier) && (keyEvent->modifiers() & Qt::ControlModifier))
        {
            switch (keyEvent->key())
            {
                case Qt::Key_Up: previousCommand();return true;
                case Qt::Key_Down: nextCommand();return true;
                default:break;
            }
        }
#endif
    }
    return QObject::eventFilter(watched, event);
}
/**
    \fn previousCommand
*/
bool   qShell::previousCommand(void)
{
    if(indexWrite<1)
        return false;
    int count=0;
    int size=_history->size();
    QString *copy=NULL;
    if(indexRead>=indexWrite) indexRead=indexWrite-1;
    // Retrieve text from UI
    QString text=ui.textBrowser_2->toPlainText();
    for(int i=0;i<size;i++)
    {
        shellHistoryEntry e=_history->at(i);
        if(e.name!=_engine->name())
            continue;
        if(count==indexRead)
        {
            copy=e.command;
            if(indexRead>0 && *copy==text) // The command is already filled in, skip it
            {
                indexRead--;
                return previousCommand();
            }
            break;
        }
        count++;
    }
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
    if(indexRead+1>=indexWrite)
        return true;
    QString *copy=NULL;
    int count=0;
    int size=_history->size();
    if(indexRead+1<indexWrite) indexRead++;
    for(int i=0;i<size;i++)
    {
        shellHistoryEntry e=_history->at(i);
        if(e.name!=_engine->name())
            continue;
        if(count==indexRead)
        {
            copy=e.command;
            break;
        }
        count++;
    }
    if(!copy) return true;
    ui.textBrowser_2->clear();
    ui.textBrowser_2->append(*copy);
    return true;
}
//EOF


