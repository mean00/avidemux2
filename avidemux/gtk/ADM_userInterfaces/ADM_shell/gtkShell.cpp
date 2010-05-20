/**
        \file gtkShell.cpp
        \brief gtk Specific js interactive shell
*/


#include "ADM_default.h"
#include "ADM_scriptShell.h"
/**
    \fn gtkShellLogger
    \brief Redirect output to the shell
*/
/*
static bool gtkShellLogger(void *cookie,const char *v)
{
    qShell *s=(qShell *)cookie;
    s->print(v);
    return true;
}
*/
/**
        \fn ADM_createJsShell
        \brief create the input for a js shell
*/
bool ADM_startShell(jsShellEvaluate eval)
{
      /*  qShell *s= new qShell(eval);
        ADM_jsRegisterLogger((void *)s,gtkShellLogger);
        s->run();
        ADM_jsUnregisterLogger();
        delete s;
        return true;
        */
        return false;
}
//EOF

