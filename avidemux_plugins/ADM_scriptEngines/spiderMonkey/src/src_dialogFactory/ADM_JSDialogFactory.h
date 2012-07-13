/**
    \file   ADM_JSDialogFactory.h
    \brief  JS / DF binding
    \author gruntster 2010


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_JSDialogFactory_H
#define _ADM_JSDialogFactory_H

#include "jsapi.h"
#include "DIA_factory.h"
#include "ADM_JSDFMenu.h"
#include "ADM_scriptDFMenu.h"
#include <vector>
/**
    \class ADM_JSDialogFactory
*/

class ADM_JSDialogFactory
{
public:
	ADM_JSDialogFactory(void) {}
	virtual ~ADM_JSDialogFactory(void);

	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								jsval *argv, jsval *rval);
	static void JSDestructor(JSContext *cx, JSObject *obj);
	static JSObject *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);
	static JSBool addControl(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool show(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

	static JSFunctionSpec methods[];
	static JSClass m_dialogFactoryHelper;
};

#endif
