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
#include "ADM_scriptIf.h"
#include <stdarg.h>
#include <vector>
#include "ADM_editor/ADM_edit.hxx"
#include "DIA_coreToolkit.h"
void    A_Resync(void);

/* our variables */
static jsLoggerFunc *jsLogger=NULL;
static void *jsLoggerCookie=NULL;
/**
    \fn jsLogger
*/
bool isJsLogRedirected(void)
{
    if(jsLogger) return true;
    return false;
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
                GUI_Error_HIG("Script Error","%s",print_buffer);
            else
                ADM_warning("[JS]%s\n",print_buffer);
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
// EOF

