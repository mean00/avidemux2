/**
        \file qt4Shell.cpp
        \brief qt4 Specific js interactive shell
*/

#include "Q_shell.h"
#include "ADM_default.h"
#include "ADM_toolkitQt.h"
/**
    \fn qt4ShellLogger
    \brief Redirect output to the shell
*/
static bool qt4ShellLogger(void *cookie,SCRIPT_LOG_TYPE type,const char *v)
{
    qShell *s=(qShell *)cookie;
    s->print(type,v);
    return true;
}

/**
        \fn ADM_createJsShell
        \brief create the input for a js shell
*/
bool ADM_startShell(jsShellEvaluate eval)
{
        qShell *s= new qShell(qtLastRegisteredDialog(), eval);
		qtRegisterDialog(s);

        ADM_scriptRegisterLogger((void *)s,qt4ShellLogger);
        s->run();

        ADM_scriptUnregisterLogger();
		qtUnregisterDialog(s);

        delete s;
        return true;
}
//EOF

