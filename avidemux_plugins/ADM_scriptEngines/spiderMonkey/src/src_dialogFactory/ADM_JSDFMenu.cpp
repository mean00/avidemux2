/**
    \file   ADM_JSDFMenu.cpp
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
#include "ADM_JSDFMenu.h"
#include "ADM_scriptDFMenu.h"


JSPropertySpec ADM_JSDFMenu::properties[] = 
{ 
	{ "index", indexProperty, JSPROP_ENUMERATE },
	{ 0 }
};

JSFunctionSpec ADM_JSDFMenu::methods[] =
{
	{ "addItem", addItem, 1, 0, 0 },
	{ 0 }
};

JSClass ADM_JSDFMenu::m_dfMenuHelper =
{
	"DFMenu", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	ADM_JSDFMenu::JSGetProperty, ADM_JSDFMenu::JSSetProperty,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, ADM_JSDFMenu::JSDestructor
};

JSObject *ADM_JSDFMenu::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
	return JS_InitClass(cx, obj, proto, &m_dfMenuHelper, 
		ADM_JSDFMenu::JSConstructor, 1,
		ADM_JSDFMenu::properties, ADM_JSDFMenu::methods,
		NULL, NULL);
}

JSBool ADM_JSDFMenu::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	if (argc != 1)
		return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[0]))
		return JS_FALSE;

	ADM_scriptDFMenuHelper *pObject = new ADM_scriptDFMenuHelper(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));

	if (!JS_SetPrivate(cx, obj, pObject))
		return JS_FALSE;

	*rval = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
}

void ADM_JSDFMenu::JSDestructor(JSContext *cx, JSObject *obj)
{
	ADM_scriptDFMenuHelper *pObject = (ADM_scriptDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

	if (pObject)
		delete pObject;
}

JSBool ADM_JSDFMenu::addItem(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	ADM_scriptDFMenuHelper *pObject = (ADM_scriptDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

	if (argc != 1)
		return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[0]))
		return JS_FALSE;

	pObject->addItem(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));

	return JS_TRUE;
}

JSBool ADM_JSDFMenu::JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_scriptDFMenuHelper *pObject = (ADM_scriptDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

		switch(JSVAL_TO_INT(id))
		{
			case indexProperty:
			{
				*vp = INT_TO_JSVAL(pObject->index());
				break;
			}
		}
	}

	return JS_TRUE;
}

JSBool ADM_JSDFMenu::JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_scriptDFMenuHelper *pObject = (ADM_scriptDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

		switch(JSVAL_TO_INT(id))
		{
			case indexProperty:
			{
				if (JSVAL_IS_INT(*vp))
					pObject->setIndex(JSVAL_TO_INT(*vp));

				break;
			}
			default:
				return JS_FALSE;
		}
	}

	return JS_TRUE;
}
