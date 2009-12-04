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
#include "ADM_default.h"
#include <string.h>
#include <pthread.h>
#include "DIA_coreToolkit.h"

#include "ADM_JSGlobal.h"
#include "ADM_JSDebug.h"
#include "ADM_JSif.h"
#include "ADM_editor/ADM_edit.hxx"
/**/

JS_PROTOTYPE displayError;
JS_PROTOTYPE displayInfo;
JS_PROTOTYPE print;
JS_PROTOTYPE displayInfo;
JS_PROTOTYPE assertTest;
JS_PROTOTYPE crashTest;
JS_PROTOTYPE help;
JS_PROTOTYPE dumpEditing;
JS_PROTOTYPE dumpTiming;
/**/

extern JSFunctionSpec *ADM_JsAudioGetFunctions(void);
extern JSFunctionSpec *ADM_JsVideoGetFunctions(void);
extern JSFunctionSpec *ADM_JsClassGetFunctions(void);
extern JSFunctionSpec *ADM_JsDebugGetFunctions(void);

/**/
static JSFunctionSpec adm_debug_functions[] = {
  /*    name          native          nargs    */
  {"displayError",  displayError,       1},
  {"displayInfo",   displayInfo,        1},
  {"print",         print,              1},
  {"assert",        assertTest,         0},
  {"crashTest",     crashTest,          0},
  {"help",          help,               0},
  {"dumpEditing",   dumpEditing,        0},
  {"dumpTiming",    dumpTiming,         0},
  {0}
};
extern ADM_Composer *video_body;
void ADM_dumpJSHooks(void);
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

/**
    \fn displayError
    \brief error popup
*/
JSBool displayError(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin displayError
	// default return value
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;
	char  *stringa = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	GUI_Verbose();
	GUI_Error_HIG("Error",stringa);
	GUI_Quiet();

	return JS_TRUE;
}// end displayError
/**
    \fn displayInfo
    \brief info popup
*/

JSBool displayInfo(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin displayInfo
	// default return value
	if(argc != 1)
		return JS_FALSE;
	if(JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;
	char  *stringa = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
	GUI_Verbose();
	GUI_Info_HIG(ADM_LOG_IMPORTANT,"Info",stringa);
	GUI_Quiet();
	return JS_TRUE;
}// end displayInfo
 /**
    \fn print
*/
JSBool print(JSContext *cx, JSObject *obj, uintN argc, 
                                       jsval *argv, jsval *rval)
{// begin print
        if(argc != 1)
                return JS_FALSE;
        char *out=JS_GetStringBytes(JS_ValueToString(cx, argv[0]));
        jsLog(JS_LOG_NORMAL,"%s",out);
        return JS_TRUE;
}// end print
/**
    \fn crashTest
    \brief Force a crash
*/
JSBool crashTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  
  int *foobar=NULL;
  *foobar=0; // CRASH!
  return JS_TRUE;
}
/**
    \fn assertTest
    \brief Force a crash
*/
JSBool assertTest(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  
  ADM_assert(0);
  return JS_TRUE;
}

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
        if(!argc)
        {
            jsLog(JS_LOG_NORMAL,"help(\"video\"); or help(\"audio\"); or help(\"debug\"); or debug(\"functions\"");
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
            DUMPTABLE(functions,ADM_JsClassGetFunctions);
        }
        if(table) dumpFunc(table);
        else jsLog(JS_LOG_ERROR,"%s not found",f);
  
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
// EOF