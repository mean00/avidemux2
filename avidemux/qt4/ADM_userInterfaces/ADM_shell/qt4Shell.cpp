/**
        \file qt4Shell.cpp
        \brief qt4 Specific js interactive shell
*/

#include "Q_shell.h"
#include "ADM_default.h"

/**
        \fn ADM_createJsShell
        \brief create the input for a js shell
*/
bool ADM_startShell(jsShellEvaluate eval)
{
        qShell *s= new qShell(eval);
        s->run();
        delete s;
        return true;
}
//EOF

