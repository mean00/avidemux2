/**
    \file   ADM_JSDFInteger.cpp
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
#include "ADM_JSDFInteger.h"

ADM_JSDFIntegerHelper::ADM_JSDFIntegerHelper(const char *title,int32_t mn,int32_t mx)
{
	_title = ADM_strdup(title); 
    _min=mn;
    _max=mx;
	_value = 0;
}

ADM_JSDFIntegerHelper::~ADM_JSDFIntegerHelper(void)
{
	if (_title)
		delete _title;

	_title = NULL;


}


diaElem* ADM_JSDFIntegerHelper::getControl(void)
{
    return new diaElemInteger(&_value,_title, _min,_max);
}

int32_t ADM_JSDFIntegerHelper::value(void)
{
	return _value;
}

void ADM_JSDFIntegerHelper::setValue(int32_t index)
{
	_value = index;
}
/*************************************************/
JSPropertySpec ADM_JSDFInteger::properties[] = 
{ 
	{ "value", valueProperty, JSPROP_ENUMERATE },
	{ 0 }
};

JSFunctionSpec ADM_JSDFInteger::methods[] =
{
	{ 0 }
};

JSClass ADM_JSDFInteger::m_dfIntegerHelper =
{
	"DFInteger", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	ADM_JSDFInteger::JSGetProperty, ADM_JSDFInteger::JSSetProperty,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, ADM_JSDFInteger::JSDestructor
};

JSObject *ADM_JSDFInteger::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
	return JS_InitClass(cx, obj, proto, &m_dfIntegerHelper, 
		ADM_JSDFInteger::JSConstructor, 1,
		ADM_JSDFInteger::properties, ADM_JSDFInteger::methods,
		NULL, NULL);
}
/**
    \fn ctor
*/
JSBool ADM_JSDFInteger::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	if (argc != 3)
		return JS_FALSE;

	if (!JSVAL_IS_STRING(argv[0]))
		return JS_FALSE;

	if (!JSVAL_IS_INT(argv[1]))
		return JS_FALSE;

	if (!JSVAL_IS_INT(argv[2]))
		return JS_FALSE;


	ADM_JSDFIntegerHelper *pObject = new ADM_JSDFIntegerHelper(
                                                    JS_GetStringBytes(JSVAL_TO_STRING(argv[0])),
                                                    JSVAL_TO_INT(argv[1]),
                                                    JSVAL_TO_INT(argv[2])
                                                );

	if (!JS_SetPrivate(cx, obj, pObject))
		return JS_FALSE;

	*rval = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
}

void ADM_JSDFInteger::JSDestructor(JSContext *cx, JSObject *obj)
{
	ADM_JSDFIntegerHelper *pObject = (ADM_JSDFIntegerHelper*)JS_GetInstancePrivate(cx, obj, &m_dfIntegerHelper, NULL);

	if (pObject)
		delete pObject;
}

JSBool ADM_JSDFInteger::JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_JSDFIntegerHelper *pObject = (ADM_JSDFIntegerHelper*)JS_GetInstancePrivate(cx, obj, &m_dfIntegerHelper, NULL);

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

JSBool ADM_JSDFInteger::JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
	if (JSVAL_IS_INT(id)) 
	{
		ADM_JSDFIntegerHelper *pObject = (ADM_JSDFIntegerHelper*)JS_GetInstancePrivate(cx, obj, &m_dfIntegerHelper, NULL);

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
