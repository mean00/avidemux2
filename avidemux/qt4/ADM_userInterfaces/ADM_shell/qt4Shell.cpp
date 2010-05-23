/**
        \file qt4Shell.cpp
        \brief qt4 Specific js interactive shell
*/

#include "Q_shell.h"
#include "ADM_default.h"
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
        qShell *s= new qShell(eval);
        ADM_scriptRegisterLogger((void *)s,qt4ShellLogger);
        s->run();
        ADM_scriptUnregisterLogger();
        delete s;
        return true;
}
//EOF

