/**
    \file   ADM_JSDFToggle.cpp
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
#include "ADM_JSDFToggle.h"

ADM_JSDFToggleHelper::ADM_JSDFToggleHelper(const char *title)
{
	_title = ADM_strdup(title);
	_value = 0;
}

ADM_JSDFToggleHelper::~ADM_JSDFToggleHelper(void)
{
	if (_title)
		delete _title;

	_title = NULL;


}


diaElem* ADM_JSDFToggleHelper::getControl(void)
{
    return new   diaElemToggle(&_value,_title,NULL);
}

uint32_t ADM_JSDFToggleHelper::value(void)
{
	return _value;
}

void ADM_JSDFToggleHelper::setValue(uint32_t index)
{
	_value = index;
}
/*************************************************/
JSPropertySpec ADM_JSDFToggle::properties[] = 
{ 
	{ "value", valueProperty, JSPROP_ENUMERATE },
	{ 0 }
};

JSFunctionSpec ADM_JSDFToggle::methods[] =
{
	{ 0 }
};

JSClass ADM_JSDFToggle::m_dfToggleHelper =
{
	"DFToggle", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	ADM_JSDFToggle::JSGetProperty, ADM_JSDFToggle::JSSetProperty,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, ADM_JSDFToggle::JSDestructor
};

JSObject *ADM_JSDFToggle::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
	return JS_InitClass(cx, obj, proto, &m_dfToggleHelper, 
		ADM_JSDFToggle::JSConstructor, 1,
		ADM_JSDFToggle::properties, ADM_JSDFToggle::methods,
		NULL, NULL);
}
/**
    \fn ctor
*/
JSBool ADM_JSDFToggle::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	if (argc != 1)
		return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[0]))
		return JS_FALSE;

	ADM_JSDFToggleHelper *pObject = new ADM_JSDFToggleHelper(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));

	if (!JS_SetPrivate(cx, obj, pObject))
		return JS_FALSE;

	*rval = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
}

void ADM_JSDFToggle::JSDestructor(JSContext *cx, JSObject *obj)
{
	ADM_JSDFToggleHelper *pObject = (ADM_JSDFToggleHelper*)JS_GetInstancePrivate(cx, obj, &m_dfToggleHelper, NULL);

	if (pObject)
		delete pObject;
}

JSBool ADM_JSDFToggle::JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_JSDFToggleHelper *pObject = (ADM_JSDFToggleHelper*)JS_GetInstancePrivate(cx, obj, &m_dfToggleHelper, NULL);

		switch(JSVAL_TO_INT(id))
		{
			case valueProperty:
			{
				*vp = INT_TO_JSVAL(pObject->value());
				break;
			}
		}
	}

	return JS_TRUE;
}

JSBool ADM_JSDFToggle::JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_JSDFToggleHelper *pObject = (ADM_JSDFToggleHelper*)JS_GetInstancePrivate(cx, obj, &m_dfToggleHelper, NULL);

		switch(JSVAL_TO_INT(id))
		{
			case valueProperty:
			{
				if (JSVAL_IS_INT(*vp))
					pObject->setValue(JSVAL_TO_INT(*vp));

				break;
			}
			default:
				return JS_FALSE;
		}
	}

	return JS_TRUE;
}
