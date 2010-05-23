/**
    \file ADM_JSif.cpp
    \brief interface to js
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_JS_IF_H
#define ADM_JS_IF_H

/**
    \enum JS_LOG_TYPE
*/
typedef enum
{
    SCRIPT_LOG_NORMAL,
    SCRIPT_LOG_ERROR
}SCRIPT_LOG_TYPE;
/**
    \typedef jsLoggerFunc
*/
typedef bool (scriptLoggerFunc)(void *cookie,SCRIPT_LOG_TYPE type,const char *);
/*
    Interface used by shell
*/
bool ADM_scriptRegisterLogger(void *cookie,scriptLoggerFunc *fun);
bool ADM_scriptUnregisterLogger(void);
/**

*/
bool jsLog(const char *fmt,...);
bool jsLogError(const char *fmt,...);
/**
    \fn parseECMAScript
    \brief Compile & execute ecma script
*/
bool parseECMAScript(const char *name);

/**
    \fn    interactiveECMAScript
    \brief interprete & execute ecma script (interactive)
*/
bool interactiveECMAScript(const char *name);
/**
    \fn jsLog

*/
bool scriptLog(SCRIPT_LOG_TYPE type, const char *fmt,...);

/**
    \fn SpidermonkeyInit
*/
bool SpidermonkeyInit(void);
/**
    \fn SpidermonkeyDestroy
*/
void SpidermonkeyDestroy(void);
/**
    \fn SpidermonkeyExit
*/
bool SpidermonkeyExit(void);

/**
    \fn parseTinyPyScript
*/
bool parseTinyPyScript(const char *name);
/**
    \fn interactiveTinyPy
*/
bool interactiveTinyPy(void);


#endif
