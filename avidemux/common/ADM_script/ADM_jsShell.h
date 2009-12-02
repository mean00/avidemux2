/**
    \file ADM_jsShell.h
    \brief Base class for js interactive shell
    \author mean, fixounet@free.fr
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_JS_SHELL_H
#define ADM_JS_SHELL_H

typedef bool (jsShellEvaluate)(const char *str);
bool ADM_startShell(jsShellEvaluate *eval);

typedef bool (jsLoggerFunc)(void *cookie,const char *);
bool ADM_jsRegisterLogger(void *cookie,jsLoggerFunc *fun);
bool ADM_jsUnregisterLogger(void);

#endif // ADM_JS_SHELL_H