/**
        \file qt4Shell.cpp
        \brief qt4 Specific js interactive shell
*/

#include "Q_shell.h"
#include "ADM_default.h"
#include "ADM_toolkitQt.h"
#include "IScriptEngine.h"

static qShell *s;

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
        s = new qShell(qtLastRegisteredDialog(), engine);
		qtRegisterDialog(s);

		engine->registerEventHandler(qt4ShellLogger);
        s->run();
        engine->unregisterEventHandler(qt4ShellLogger);

		qtUnregisterDialog(s);

        delete s;
        return true;
}
//EOF

