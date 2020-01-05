/**
    \file Q_shell.h
*/
#ifndef Q_SHELL_H
#define Q_SHELL_H

#include "ui_shell.h"

#include "ADM_inttype.h"
#include "IScriptEngine.h"

#include <vector>

typedef struct
{
    std::string name;
    QString *command;
} shellHistoryEntry;

/**
    \class ADM_jsQt4Shell
*/
class qShell: public QDialog
{
	Q_OBJECT
#ifdef SCRIPT_SHELL_HISTORY_VERBOSE
    bool                 dumpHistory(void);
#endif
    void                 showEvent(QShowEvent *ev);
protected:
    IScriptEngine      *_engine;
    Ui_SpiderMonkeyShell ui;
    bool                 eventFilter(QObject* watched, QEvent* event);
    bool                 previousCommand(void);
    bool                 nextCommand(void);
    std::vector<shellHistoryEntry> *_history;
    int                  addToHistory(QString &str);
    int                  indexWrite;
    int                  indexRead;
public:
                    qShell(QWidget *parent, IScriptEngine *engine, std::vector <shellHistoryEntry> *commands);
    virtual         ~qShell() ;
    bool            run(void);
    bool            print(IScriptEngine::EventType type, const char *s);
public slots:
    bool            evaluate(bool x);
    bool            clear(bool x);
};

#endif
