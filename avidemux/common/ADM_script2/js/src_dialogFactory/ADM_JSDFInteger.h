/**
    \file   ADM_JSDFInteger.h
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

#ifndef _ADM_JSDFInteger_H
#define _ADM_JSDFInteger_H

#include "jsapi.h"
#include "DIA_factory.h"
#include "ADM_JSDF.h"
/**
    \class ADM_JSDFIntegerHelper
*/
class ADM_JSDFIntegerHelper : public ADM_JSDFBaseHelper
{
private:
	char                *_title;
	int32_t             _value;
    int32_t             _min,_max;

public:
                     ADM_JSDFIntegerHelper(const char *title,int32_t mn, int32_t mx);
                     ~ADM_JSDFIntegerHelper(void);
	virtual diaElem* getControl(void);
            int32_t value(void);
            void     setValue(int32_t value);
};
/**
    \class ADM_JSDFInteger
*/
class ADM_JSDFInteger
{
protected:
    int32_t   minimum,maximum;
public:
	ADM_JSDFInteger(int32_t mn,int32_t mx) {minimum=mn;maximum=mx;}

	static JSBool         JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								jsval *argv, jsval *rval);
	static void           JSDestructor(JSContext *cx, JSObject *obj);
	static JSObject       *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);
	static JSBool         JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool         JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSPropertySpec properties[];
	static JSFunctionSpec methods[];
	static JSClass        m_dfIntegerHelper;

	enum
	{
		valueProperty
	};
};

#endif
