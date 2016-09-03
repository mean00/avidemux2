/**
    \file   ADM_JSDFMenu.h
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

#ifndef _ADM_JSDFMenu_H
#define _ADM_JSDFMenu_H

#include "jsapi.h"
#include "DIA_factory.h"
#include "ADM_JSDF.h"
#include "ADM_scriptDF.h"
#include <vector>

/**
    \class ADM_JSDFMenu
*/
class ADM_JSDFMenu
{
public:
	ADM_JSDFMenu(void) {}

	static JSBool   JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								jsval *argv, jsval *rval);
	static void     JSDestructor(JSContext *cx, JSObject *obj);
	static JSObject *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);
	static JSBool   JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool   JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

	static JSBool   addItem(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);	

	static JSPropertySpec properties[];
	static JSFunctionSpec methods[];
	static JSClass m_dfMenuHelper;

	enum
	{
		indexProperty
	};
};

#endif
