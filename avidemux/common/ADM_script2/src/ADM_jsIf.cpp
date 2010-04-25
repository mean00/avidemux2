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

#include "ADM_js.h"
#include <stdarg.h>
#include <vector>
#include "ADM_editor/ADM_edit.hxx"
void    A_Resync(void);

/* our variables */
static jsLoggerFunc *jsLogger=NULL;
static void *jsLoggerCookie=NULL;
vector <ADM_JS_HOOK >jsHooks;
extern ADM_Composer *video_body;
extern bool ADM_JSDialogFactoryInit(JSContext *cx, JSObject *obj);
#define JSVAR(a,b,c) a b=c

#if defined( __MINGW32__) 
 pthread_t g_pThreadSpidermonkey ;
#else
JSVAR( pthread_t, g_pThreadSpidermonkey , 0);
#endif
JSVAR( pthread_mutex_t, g_pSpiderMonkeyMutex , PTHREAD_MUTEX_INITIALIZER);
// expose our main javascript context to the entire program
static bool g_bJSSuccess=false;
static JSObject   *g_pObject=NULL;
static JSContext  *g_pCx=NULL;
static JSRuntime  *g_pRt=NULL;

static JSClass global_class = {
            "global", JSCLASS_GLOBAL_FLAGS,
            JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
            JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
            JSCLASS_NO_OPTIONAL_MEMBERS
            };

extern void  printJSError(JSContext *cx, const char *message, JSErrorReport *report);
/**
    \fn parseECMAScript
    \brief Compile & execute ecma script
*/
bool parseECMAScript(const char *name)
{// begin parseECMAScript
	jsval rval;
	uintN lineno = 0;
	g_bJSSuccess = 0;
	ADM_info("Spidermonkey compiling \"%s\"...",name);
	JSScript *pJSScript = JS_CompileFile(g_pCx, g_pObject, name);
	ADM_info("Done.\n");
	if(pJSScript != NULL)
	{// begin execute external file
		printf("Spidermonkey executing \"%s\"...\n",name);
		JSBool ok = JS_ExecuteScript(g_pCx, g_pObject, pJSScript, &rval);
		JS_DestroyScript(g_pCx,pJSScript);
		ADM_info("Done.\n");
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
/**
    \fn jsRegisterAvidemux
*/
extern "C" JSFunctionSpec  *jsGetIfFunctions(void);
extern "C" JSFunctionSpec  *jsGetTestFunctions(void);
extern "C" JSFunctionSpec  *jsGetAdmFunctions(void);
extern "C" JSFunctionSpec  *jsGetEditFunctions(void);
extern "C" JSFunctionSpec  *jsGetDialogFactoryFunctions(void);
extern "C" JSObject *jsEditorInit(JSContext *cx,JSObject *obj);
extern "C" JSObject *jsAvidemuxInit(JSContext *cx,JSObject *obj);
static bool registerOne(const char *name,const char *text,JSFunctionSpec *s,JSContext *cx,JSObject *obj)
{
    if (JS_DefineFunctions(cx, obj, s) != JS_TRUE) 
    {
            ADM_error("Cannot register %s functions\n",name);
            return false;
    }
    ADM_info("Registered %s functions\n",name);
    ADM_JS_HOOK h;
    h.name=name;    
    h.text=text;
    h.jsFunctions=s;
    jsHooks.push_back(h);
    return true;
}
static void  dump(JSFunctionSpec *f)
{
    while(f->name)
    {
        jsLog(JS_LOG_NORMAL,"    %s",f->name);
        f++;
    }
}
/**
    \fn jsHelp
    \brief help command

*/
extern "C" 
{
void jsHelp(const char *s)
{
int n=jsHooks.size();
    if(!s) goto none;
    {
    for(int i=0;i<n;i++)
        if(!strcasecmp(s,jsHooks[i].name))
        {
            const char *t=jsHooks[i].text;
            if(t)
                jsLog(JS_LOG_NORMAL,"%s",t);
            return dump(jsHooks[i].jsFunctions);
        }
    }
//    if(!strcasecmp(s,"load")) return dump(jsGetAvidemuxFunctions());
none:
        jsLog(JS_LOG_NORMAL,"please use help(\"xxx\") with xx among");

        for(int i=0;i<n;i++)
            jsLog(JS_LOG_NORMAL,"    %s",jsHooks[i].name);

}
} // extern "C"
/**
    \fn jsRegisterAvidemux
    \brief Register avidemux hookd
*/
static bool jsRegisterAvidemux(JSContext *cx,JSObject *obj)
{
ADM_JS_HOOK h;
        registerOne("Debug","",   jsGetIfFunctions(),    cx,obj);
        registerOne("Test","", jsGetTestFunctions(),  cx,obj);
        
        // Register also our class (for  help() )
            h.name="adm";
            h.text="Please prefix this with adm.";
            h.jsFunctions=jsGetAdmFunctions();
            jsHooks.push_back(h);
            jsAvidemuxInit(cx,obj);
        // Register also edit
            h.name="editor";
            h.text="Please prefix this with editor.";
            h.jsFunctions=jsGetEditFunctions();
            jsEditorInit(cx,obj);
            jsHooks.push_back(h);
        // Register dialogFactory functions
#if 0
            h.name="dialog";
            h.text=".";
            h.jsFunctions=jsGetDialogFactoryFunctions();
            jsHooks.push_back(h);
#endif
            return ADM_JSDialogFactoryInit(cx,obj);
}
/**
    \fn SpidermonkeyInit
*/
bool SpidermonkeyInit()
{// begin SpidermonkeyInit
	// setup JS
	g_pCx = NULL;
	g_pObject = NULL;
	g_pRt = NULL;
	JSRuntime *rt = JS_NewRuntime(1000000L);
	g_pRt = rt;
	if ( !rt  )
	{
		// Do some error reporting
		ADM_error("Spidermonkey failed to initialize runtime!\n");
        return false;
	}
	
// begin runtime created
    g_pCx = JS_NewContext(rt, 8192);
    if ( !g_pCx  )
    {
        // Do some error reporting
        ADM_error("Spidermonkey failed to initialize context!\n");
        return false;
    }
   
   // begin context created
    g_pObject = JS_NewObject(g_pCx, &global_class, 0, 0);
    if(!g_pObject)
    {
        ADM_error("Cannot initialize object\n");
        return false;
    }
    if(JS_TRUE!=JS_InitStandardClasses(g_pCx, g_pObject))
    {
        ADM_error("Cannot initialize standard classes\n");
        return false;
    }
    // register error handler
    JS_SetErrorReporter(g_pCx, printJSError);
        
    //register our functions
    jsRegisterAvidemux(g_pCx,g_pObject);
    ADM_info("Spidermonkey initialized\n");
    return true;

}// end SpidermonkeyInit
/**
    \fn SpidermonkeyDestroy
*/
void SpidermonkeyDestroy()
{// begin SpidermonkeyDestroy
#ifdef ADM_JS_THREADSAFE
	JS_SetContextThread(g_pCx);	
#endif
	JS_DestroyContext(g_pCx);
	JS_DestroyRuntime(g_pRt);
}// end SpidermonkeyDestroy
/**
    \fn StartThreadSpidermonkey
*/
void *StartThreadSpidermonkey(void *pData)
{// begin StartThreadSpidermonkey
        pthread_mutex_lock(&g_pSpiderMonkeyMutex);
        /*
        The following mailling list post describes how to CORRECTLY use
        the threading API support with Spidermonkey
        "Thread from SpiderMonkey newsgroup"
        http://archive.gingerall.cz/archives/public/sablot2004/msg00117.html
        */
        // Notify the Spidermonkey that we'll be processing in a thread
#ifdef ADM_JS_THREADSAFE
        JS_SetContextThread(g_pCx);
        JS_BeginRequest(g_pCx);
#endif
        bool ret = false;
        const char *pScriptFile = static_cast<const char *>(pData);
        ret = parseECMAScript(pScriptFile);
        if(ret == true)
        {
                video_body->setProjectName(pScriptFile);
        }
        // Notify Spidermonkey that our thread processing has finished
#ifdef ADM_JS_THREADSAFE
        JS_EndRequest(g_pCx);
        JS_ClearContextThread(g_pCx);
#endif
        pthread_mutex_unlock(&g_pSpiderMonkeyMutex);

        return NULL;
}// end StartThreadSpidermonkey
/**
    \fn JS_setSuccess
*/
extern "C" void jsSetSuccess(int bSuccess)
{// begin JS_setSuccess
	g_bJSSuccess = bSuccess;
	jsLog(JS_LOG_NORMAL,"[ECMA] success : %d\n", g_bJSSuccess);
}// end JS_setSuccess
/**
    \fn SpidermonkeyExit
*/
bool SpidermonkeyExit(void)
{
    ADM_info("Waiting for Spidermonkey to finish...\n");
    pthread_mutex_lock(&g_pSpiderMonkeyMutex);
    ADM_info("Cleaning up Spidermonkey.\n");
    SpidermonkeyDestroy();
    pthread_mutex_unlock(&g_pSpiderMonkeyMutex);
    return true;
}
//EOF
