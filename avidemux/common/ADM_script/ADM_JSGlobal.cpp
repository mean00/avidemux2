/**
 * 
 * 
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
#include "ADM_JSDirectorySearch.h"

extern uint8_t JS_AvidemuxFunction(JSContext *cx,JSObject *global);
extern void A_Resync(void);
extern char * actual_workbench_file;

void
printJSError(JSContext *cx, const char *message, JSErrorReport *report)
{// begin printJSError
int quiet=GUI_isQuiet();
char buf[4];
FILE *fd = ADM_fopen(report->filename,"rb");
        if(quiet)
                GUI_Verbose();
	if( fd ){
		fread(buf,1,4,fd);
		fclose(fd);
	}
	if( strncmp(buf,"//AD",4) ){
            if (report->filename || report->lineno)
		GUI_Error_HIG("Spidermonkey ECMAScript Error",
		              "%s: line %d:\nMsg: %s\n"
                              "Not an ECMAScript file. Try open it with 'File' -> 'Open...'",
                              report->filename,
                              report->lineno,
                              message);
            else
		GUI_Error_HIG("Spidermonkey ECMAScript Error",
                              "Not an ECMAScript file. Try open it with 'File' -> 'Open...'");
	}else{
		GUI_Error_HIG("Spidermonkey ECMAScript Error", 
			"%s: line %d:\nMsg: %s\n",
			report->filename,
			report->lineno,
			message);
	}
        fprintf(stderr, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  Spidermonkey ECMAScript Error\n");
        if (report->filename || report->lineno)
            fprintf(stderr, "\t%s: line %d:\n\tMsg: %s\n",
                   report->filename, report->lineno, message);

        if(quiet)
                GUI_Quiet();

}// end printJSError

bool SpidermonkeyInit()
{// begin SpidermonkeyInit
	// setup JS
	g_pCx = NULL;
	g_pObject = NULL;
	g_pRt = NULL;
	JSRuntime *rt = JS_NewRuntime(1000000L);
	g_pRt = rt;
	if ( rt == NULL )
	{
		// Do some error reporting
		printf("Spidermonkey failed to initialize runtime!\n");
	}
	else
	{// begin runtime created
		JSContext *cx = JS_NewContext(rt, 8192);
		g_pCx = cx;
		if ( cx == NULL )
		{
			// Do some error reporting
			printf("Spidermonkey failed to initialize context!\n");
		}
		else
		{// begin context created

			JSObject *global = JS_NewObject(cx, &g_globalClass, 0, 0);
			g_pObject = global;
			JS_InitStandardClasses(cx, global);
			// load our custom JS class objects
			JSObject *obj = ADM_JSAvidemux::JSInit(cx, global);
			JSObject *dsObj = ADM_JSDirectorySearch::JSInit(cx, global);
			// register error handler
			JS_SetErrorReporter(cx, printJSError);
                        JS_AvidemuxFunction(cx,global);
			return true;
		}// end context created
	}// end runtime created
	return false;
}// end SpidermonkeyInit

void SpidermonkeyDestroy()
{// begin SpidermonkeyDestroy
#ifdef ADM_JS_THREADSAFE
	JS_SetContextThread(g_pCx);	
#endif
	JS_DestroyContext(g_pCx);
	JS_DestroyRuntime(g_pRt);
}// end SpidermonkeyDestroy

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
        if(ret == false)
        {
                if( actual_workbench_file )
                        ADM_dealloc(actual_workbench_file);
                actual_workbench_file = ADM_strdup(pScriptFile);
        }
        // Notify Spidermonkey that our thread processing has finished
#ifdef ADM_JS_THREADSAFE
        JS_EndRequest(g_pCx);
        JS_ClearContextThread(g_pCx);
#endif
        pthread_mutex_unlock(&g_pSpiderMonkeyMutex);

        return NULL;
}// end StartThreadSpidermonkey

void JS_setSuccess(bool bSuccess)
{// begin JS_setSuccess
	g_bJSSuccess = bSuccess;
	printf("[ECMA] success : %d\n", g_bJSSuccess);
}// end JS_setSuccess

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
