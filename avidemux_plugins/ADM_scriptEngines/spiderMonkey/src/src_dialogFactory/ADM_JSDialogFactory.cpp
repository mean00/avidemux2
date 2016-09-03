/**
    \file   ADM_JSDialogFactory.cpp
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
#include "ADM_JSDialogFactory.h"
#include "ADM_scriptDialogFactory.h"

JSFunctionSpec ADM_JSDialogFactory::methods[] =
{
	{ "addControl", addControl, 1, 0, 0 },
	{ "show", show, 0, 0, 0 },
	{ 0 }
};

JSClass ADM_JSDialogFactory::m_dialogFactoryHelper =
{
	"DialogFactory", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, JS_PropertyStub,
	JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub, ADM_JSDialogFactory::JSDestructor
};

JSObject *ADM_JSDialogFactory::JSInit(JSContext *cx, JSObject *obj, JSObject *proto)
{
	return JS_InitClass(cx, obj, proto, &m_dialogFactoryHelper, 
		ADM_JSDialogFactory::JSConstructor, 1,
		NULL, ADM_JSDialogFactory::methods,
		NULL, NULL);
}

JSBool ADM_JSDialogFactory::JSConstructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	if (argc != 1)
		return JS_FALSE;

	if (JSVAL_IS_STRING(argv[0]) == false)
		return JS_FALSE;

	ADM_scriptDialogFactoryHelper *pObject = new ADM_scriptDialogFactoryHelper(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));

	if (!JS_SetPrivate(cx, obj, pObject))
		return JS_FALSE;

	*rval = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
}

void ADM_JSDialogFactory::JSDestructor(JSContext *cx, JSObject *obj)
{
	ADM_scriptDialogFactoryHelper *pObject = (ADM_scriptDialogFactoryHelper*)JS_GetInstancePrivate(cx, obj, &m_dialogFactoryHelper, NULL);

	if (pObject != NULL)
		delete pObject;
}

JSBool ADM_JSDialogFactory::addControl(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	ADM_scriptDialogFactoryHelper *p = (ADM_scriptDialogFactoryHelper*)JS_GetInstancePrivate(cx, obj, &m_dialogFactoryHelper, NULL);

	if (argc != 1)
		return JS_FALSE;

	p->addControl((ADM_scriptDFMenuHelper*)JS_GetPrivate(cx, JSVAL_TO_OBJECT(argv[0])));

	return JS_TRUE;
}

JSBool ADM_JSDialogFactory::show(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	ADM_scriptDialogFactoryHelper *p = (ADM_scriptDialogFactoryHelper*)JS_GetInstancePrivate(cx, obj, &m_dialogFactoryHelper, NULL);

	if (argc != 0)
		return JS_FALSE;

	int controlCount;
	diaElem **elems = p->getControls(&controlCount);

	*rval = BOOLEAN_TO_JSVAL(diaFactoryRun(p->title(), controlCount, elems));

	for (int i = 0; i < controlCount; i++)
		delete elems[i];

	delete elems;

	return JS_TRUE;
}
