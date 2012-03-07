/**
    \file Q_shell.h
*/
#ifndef Q_SHELL_H
#define Q_SHELL_H

#include <QtGui/QItemDelegate>
#include "ui_shell.h"

#include "ADM_inttype.h"
#include "IScriptEngine.h"

/**
    \class ADM_jsQt4Shell
*/
#define Q_SHELL_HISTORY 8 // Must be a power of 2!
class qShell: public QDialog
{
	Q_OBJECT
protected:
    IScriptEngine      *_engine;
    Ui_SpiderMonkeyShell ui;
    bool                 eventFilter(QObject* watched, QEvent* event);
    bool                 previousCommand(void);
    bool                 nextCommand(void);
    QString              *history[Q_SHELL_HISTORY];
    int                  indexWrite;
    int                  indexRead;
public:
                    qShell(QWidget *parent, IScriptEngine *engine);
    virtual         ~qShell() ;
    bool            run(void);
    bool            print(IScriptEngine::EVENT_TYPE type, const char *s);
public slots:
    bool            evaluate(bool x);
    bool            clear(bool x);
};

#endif
