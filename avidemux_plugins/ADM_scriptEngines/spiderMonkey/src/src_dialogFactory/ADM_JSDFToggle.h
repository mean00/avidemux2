/**
    \file   ADM_JSDFToggle.h
    \brief  JS / DF binding
    \author gruntster/mean 2010


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _ADM_JSDFToggle_H
#define _ADM_JSDFToggle_H

#include "jsapi.h"
#include "DIA_factory.h"
#include "ADM_JSDF.h"
#include "ADM_scriptDF.h"

/**
    \class ADM_JSDFToggle
*/
class ADM_JSDFToggle
{
public:
	ADM_JSDFToggle(void) {}

	static JSBool         JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								jsval *argv, jsval *rval);
	static void           JSDestructor(JSContext *cx, JSObject *obj);
	static JSObject       *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);
	static JSBool         JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool         JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSPropertySpec properties[];
	static JSFunctionSpec methods[];
	static JSClass        m_dfToggleHelper;

	enum
	{
		valueProperty
	};
};

#endif
