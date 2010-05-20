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

ADM_JSDFMenuHelper::ADM_JSDFMenuHelper(const char *title)
{
	_title = ADM_strdup(title);
	_menuEntries = NULL;
	_index = 0;
}

ADM_JSDFMenuHelper::~ADM_JSDFMenuHelper(void)
{
	if (_title)
		delete _title;

	_title = NULL;

	std::vector<char*>::iterator it;

	for (it = _items.begin(); it != _items.end(); it++)
		delete *it;

	_items.clear();

	if (_menuEntries)
		delete _menuEntries;

	_menuEntries = NULL;
}

void ADM_JSDFMenuHelper::addItem(const char* item)
{
	_items.push_back(ADM_strdup(item));
}

diaElem* ADM_JSDFMenuHelper::getControl(void)
{
	if (_menuEntries)
		delete _menuEntries;

	std::vector<char*>::iterator it;
	int i = 0;

	_menuEntries = new diaMenuEntry[_items.size()];	

	for (it = _items.begin(); it != _items.end(); it++)
	{
		_menuEntries[i].val = i;
		_menuEntries[i].text = *it;
		_menuEntries[i].desc = NULL;

		i++;
	}

	return new diaElemMenu(&_index, _title, _items.size(), _menuEntries);
}

int ADM_JSDFMenuHelper::index(void)
{
	return _index;
}

void ADM_JSDFMenuHelper::setIndex(int index)
{
	_index = index;
}

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

	ADM_JSDFMenuHelper *pObject = new ADM_JSDFMenuHelper(JS_GetStringBytes(JSVAL_TO_STRING(argv[0])));

	if (!JS_SetPrivate(cx, obj, pObject))
		return JS_FALSE;

	*rval = OBJECT_TO_JSVAL(obj);

	return JS_TRUE;
}

void ADM_JSDFMenu::JSDestructor(JSContext *cx, JSObject *obj)
{
	ADM_JSDFMenuHelper *pObject = (ADM_JSDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

	if (pObject)
		delete pObject;
}

JSBool ADM_JSDFMenu::addItem(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
	ADM_JSDFMenuHelper *pObject = (ADM_JSDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

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
		ADM_JSDFMenuHelper *pObject = (ADM_JSDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

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
		ADM_JSDFMenuHelper *pObject = (ADM_JSDFMenuHelper*)JS_GetInstancePrivate(cx, obj, &m_dfMenuHelper, NULL);

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
