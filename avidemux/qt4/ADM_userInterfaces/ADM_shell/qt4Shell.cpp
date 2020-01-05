/**
        \file qt4Shell.cpp
        \brief qt4 Specific js interactive shell
*/

#include "Q_shell.h"
#include "ADM_default.h"
#include "ADM_toolkitQt.h"
#include "IScriptEngine.h"

static qShell *s;
static std::vector <shellHistoryEntry> shellHistory;

/**
    \fn qt4ShellLogger
    \brief Redirect output to the shell
*/
static void qt4ShellLogger(IScriptEngine::EngineEvent *event)
{
    s->print(event->eventType, event->message);
}

/**
        \fn ADM_createJsShell
        \brief create the input for a js shell
*/
bool ADM_startShell(IScriptEngine *engine)
{
        s = new qShell(qtLastRegisteredDialog(), engine, &shellHistory);
		qtRegisterDialog(s);

		engine->registerEventHandler(qt4ShellLogger);
        s->run();
        engine->unregisterEventHandler(qt4ShellLogger);

		qtUnregisterDialog(s);

        delete s;
        return true;
}

/**
    \fn ADM_clearQtShellHistory
    \brief free memory
*/
int ADM_clearQtShellHistory(void)
{
    int freed=0;
    int size=shellHistory.size();
    ADM_info("Clearing script shell history (%d entries)\n",size);
    for(int i=0;i<size;i++)
    {
        shellHistoryEntry e=shellHistory.at(i);
        if(!e.command) continue;
        QString *s=e.command;
        QByteArray b=s->toUtf8();
        freed+=b.size();
        delete s;
        s=NULL;
    }
    shellHistory.clear();
    if(freed)
        ADM_info("Freed %d bytes allocated for commands.\n",freed);
    return freed;
}
//EOF

