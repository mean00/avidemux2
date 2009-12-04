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
#include "ADM_default.h"
#include <string.h>
#include <pthread.h>
#include "DIA_coreToolkit.h"
#include "ADM_JSGlobal.h"
#include "ADM_JSAvidemux.h"
#include "ADM_jsShell.h"
#include "ADM_JSif.h"
#include <stdarg.h>

void    A_Resync(void);
static jsLoggerFunc *jsLogger=NULL;
static void *jsLoggerCookie=NULL;

/**
    \fn parseECMAScript
    \brief Compile & execute ecma script
*/
bool parseECMAScript(const char *name)
{// begin parseECMAScript
	jsval rval;
	uintN lineno = 0;
	g_bJSSuccess = 0;
	printf("Spidermonkey compiling \"%s\"...",name);
	JSScript *pJSScript = JS_CompileFile(g_pCx, g_pObject, name);
	printf("Done.\n");
	if(pJSScript != NULL)
	{// begin execute external file
		printf("Spidermonkey executing \"%s\"...",name);
		JSBool ok = JS_ExecuteScript(g_pCx, g_pObject, pJSScript, &rval);
		JS_DestroyScript(g_pCx,pJSScript);
		printf("Done.\n");
	}// end execute external file
        // Run garbage collector now, it is safe
    JS_GC(g_pCx);
	A_Resync();
	return g_bJSSuccess;
}// end parseECMAScript
/**
    \fn jsLogger
*/
bool isJsLogRedirected(void)
{
    if(jsLogger) return true;
    return false;
}
/**
    \fn jsEvaluate
*/
static bool jsEvaluate(const char *str)
{
jsval rval;
   JS_EvaluateScript(g_pCx,g_pObject,str,strlen(str),"dummy",1,&rval);
   return true; 
}
/**
    \fn    interactiveECMAScript
    \brief interprete & execute ecma script (interactive)
*/
bool interactiveECMAScript(const char *name)
{
    ADM_startShell(jsEvaluate);
    JS_GC(g_pCx);
	A_Resync();
    ADM_info("Ending JS shell...\n");
	return true;
}
/**
    \fn jsLog
*/
bool jsLog(JS_LOG_TYPE type, const char *prf,...)
{
 static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        if(true==isJsLogRedirected())
            jsLogger(jsLoggerCookie,type,print_buffer);
        else
        {
            if(type==JS_LOG_ERROR)
                GUI_Error_HIG("Spidermonkey ECMAScript Error","%s",print_buffer);
            else
                ADM_warning("[JS]%s",print_buffer);
        }
        return true;
}
/**
    \fn ADM_jsRegisterLogger
*/
bool ADM_jsRegisterLogger(void *cookie,jsLoggerFunc *fun)
{
    jsLoggerCookie=cookie;
    jsLogger=fun;
    return true;
}
/**
    \fn ADM_jsUnregisterLogger
*/
bool ADM_jsUnregisterLogger(void)
{
    jsLogger=NULL;
    return true;
}
