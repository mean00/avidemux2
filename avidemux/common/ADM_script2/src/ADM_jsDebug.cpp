/**
    \file ADM_JSDebug.cpp
    \brief Debug oriented functions for avidemux JS/JS shell
    \author mean (c) 2009 fixounet@free.fr


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_js.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_jsDebug.h"
/**/
/**/


extern ADM_Composer *video_body;
void ADM_dumpJSHooks(void);
#if 0
/**
    \fn JS_AvidemuxRegisterDebugFunction
*/
bool JS_AvidemuxRegisterDebugFunction(JSContext *cx,JSObject *global)
{
	if(JS_DefineFunctions(cx, global, adm_debug_functions) == true)
		return true;

	ADM_warning("Unable to define functions\n");
	return false;
}

/**
    \fn ADM_JsDebugGetFunctions

*/
JSFunctionSpec *ADM_JsDebugGetFunctions(void)
{
    return adm_debug_functions;
}
#endif
/**
    \fn displayError
    \brief error popup
*/
void jsPopupError(const char *s)
{// begin displayError
	
	GUI_Verbose();
	GUI_Error_HIG("Error",s);
	GUI_Quiet();

}// end displayError
/**
    \fn displayInfo
    \brief info popup
*/

void jsPopupInfo(const char *s)
{// begin displayInfo
	
	GUI_Verbose();
	GUI_Info_HIG(ADM_LOG_IMPORTANT,"Info",s);
	GUI_Quiet();
	
}// end displayInfo
 /**
    \fn print
*/
void jsPrint(const char *s)
{// begin print
        jsLog(JS_LOG_NORMAL,"%s",s);
}// end print
void jsPrint2(const char *s)
{// begin print
        jsLog(JS_LOG_NORMAL,"%s",s);
}// end print


static void dumpFunc(JSFunctionSpec *f)
{
    while(f->name)
    {
        jsLog(JS_LOG_NORMAL,"     %s(..)",f->name);
        f++;
    }
}
/**
    \fn help
    \brief dump avidemux specific functions
*/
JSBool help(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
#if 0
        if(!argc)
        {
            jsLog(JS_LOG_NORMAL,"help(\"class\"); or help(\"video\"); or help(\"audio\"); or help(\"debug\"); or debug(\"functions\"");
            return JS_TRUE;
        }
        if(argc != 1)
        {
          jsLog(JS_LOG_ERROR,"help accepts only one arg, type help() to get them\n");
          return JS_FALSE;
        } 
        JSFunctionSpec *table=NULL;
        char *f=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        
#define DUMPTABLE(x,y) if(!strcasecmp(f,#x)) table=y();
        if(f)
        {
            DUMPTABLE(audio,ADM_JsAudioGetFunctions);
            DUMPTABLE(video,ADM_JsVideoGetFunctions);
            DUMPTABLE(debug,ADM_JsDebugGetFunctions);
            DUMPTABLE(functions,ADM_JsFunctionGetFunctions);
            DUMPTABLE(class,ADM_JsClassGetFunctions);
        }
        if(table) dumpFunc(table);
        else jsLog(JS_LOG_ERROR,"%s not found",f);
#endif  
  return JS_TRUE;
}
/**
    \fn dumpEditing
    \brief dump segment, video & all
*/
JSBool dumpEditing(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
uint32_t frame;
uint32_t sz;
        if(argc)
        {
            return JS_FALSE;
        }
        enterLock();
        video_body->dumpEditing();
        leaveLock(); 
        
        return JS_TRUE;
}// end PostProcess
/**
    \fn dumpTiming
    \brief dump segment, video & all
*/
JSBool dumpTiming(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin PostProcess
uint32_t info;
uint32_t frame;
uint32_t sz;
        if(argc != 0)
          return JS_FALSE;
  
        enterLock();
        video_body->dumpTiming();
        leaveLock(); 
        
        return JS_TRUE;
}// end PostProcess
/**
    \fn printJSError
*/
void  printJSError(JSContext *cx, const char *message, JSErrorReport *report)
{// begin printJSError
int quiet=GUI_isQuiet();
char buf[4];
FILE *fd = ADM_fopen(report->filename,"rb");
    if(quiet)
            GUI_Verbose();
	if( fd )
    {
		fread(buf,1,4,fd);
		fclose(fd);
	}
	if( strncmp(buf,"//AD",4) )
    {
            if (report->filename || report->lineno)
                jsLog(JS_LOG_ERROR,"%s: line %d:\nMsg: %s\n",
                              report->filename,
                              report->lineno,
                              message);
            else
                jsLog(JS_LOG_ERROR,"Error");
    
	}else
    {
            jsLog(JS_LOG_ERROR,"%s: line %d:\nMsg: %s\n",report->filename,report->lineno,message);
	}
       
    if(quiet)
            GUI_Quiet();

}// end printJSError
// EOF