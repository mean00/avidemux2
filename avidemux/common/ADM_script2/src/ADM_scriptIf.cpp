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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_scriptIf.h"
#include <stdarg.h>
#include <vector>
#include "ADM_editor/ADM_edit.hxx"
#include "DIA_coreToolkit.h"


/* our variables */
static scriptLoggerFunc *scriptLogger=NULL;
static void             *scriptLoggerCookie=NULL;
/**
    \fn jsLogger
*/
bool isJsLogRedirected(void)
{
    if(scriptLogger) return true;
    return false;
}
/**
    \fn jsLog
*/
bool jsLog( const char *prf,...)
{
 static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        if(true==isJsLogRedirected())
            scriptLogger(scriptLoggerCookie,SCRIPT_LOG_NORMAL,print_buffer);
        else
        {
                printf("%s",print_buffer);
        }

        return true;
}

/**
    \fn jsLogError
*/
bool jsLogError(const char *prf,...)
{
 static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	prf);
		vsnprintf(print_buffer,1023,prf,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        if(true==isJsLogRedirected())
            scriptLogger(scriptLoggerCookie,SCRIPT_LOG_ERROR,print_buffer);
        else
        {
                GUI_Error_HIG("Script Error","%s",print_buffer);
        }

        return true;
}
/**
    \fn ADM_scriptRegisterLogger
*/
bool ADM_scriptRegisterLogger(void *cookie,scriptLoggerFunc *fun)
{
    scriptLoggerCookie=cookie;
    scriptLogger=fun;
    return true;
}
/**
    \fn ADM_scriptUnregisterLogger
*/
bool ADM_scriptUnregisterLogger(void)
{
    scriptLogger=NULL;
    return true;
}
// EOF

