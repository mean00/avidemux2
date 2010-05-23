/**
    \file ADM_js
    \brief Standard includes and defines
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_JS_H
#define ADM_JS_H


#include "ADM_default.h"
#include <string.h>
#include <pthread.h>
#include <vector>
using std::vector;
#include "DIA_coreToolkit.h"
#include "jsapi.h"
#include "ADM_jsUtils.h"
#include "ADM_scriptShell.h"
#include "ADM_scriptDebug.h"
#include "ADM_jsDFactory.h"

typedef JSBool JS_PROTOTYPE(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

bool jsLog( const char *fmt,...);
bool jsLogError( const char *fmt,...);
/**
    \struct ADM_JS_HOOK
*/
typedef struct
{
    const char      *name;
    const char      *text;
    JSFunctionSpec  *jsFunctions;
}ADM_JS_HOOK;
extern vector <ADM_JS_HOOK >jsHooks;
#if !defined(ADM_JS_THREADSAFE)
    #define enterLock() {}
    #define leaveLock() {}
#else
    #define enterLock() jsrefcount nRefCount = JS_SuspendRequest(cx)
    #define leaveLock() {JS_ResumeRequest(cx,nRefCount); }
#endif

#endif
