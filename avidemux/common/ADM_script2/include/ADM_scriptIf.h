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
    JS_LOG_NORMAL,
    JS_LOG_ERROR
}JS_LOG_TYPE;
/**
    \typedef jsLoggerFunc
*/
typedef bool (jsLoggerFunc)(void *cookie,JS_LOG_TYPE type,const char *);
/*

*/
bool ADM_jsRegisterLogger(void *cookie,jsLoggerFunc *fun);
bool ADM_jsUnregisterLogger(void);

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
bool jsLog(JS_LOG_TYPE type, const char *fmt,...);

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
