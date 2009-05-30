#ifndef _ADM_JSAVIDEMUX_H
#define _ADM_JSAVIDEMUX_H

#pragma once


// Spidermonkey
#include "ADM_libraries/ADM_smjs/jsapi.h"
#include "ADM_Avidemux.h"
class ADM_JSAvidemux
{
public:
	ADM_JSAvidemux(void) : m_pObject(NULL) {}
	virtual ~ADM_JSAvidemux(void);

	static JSBool JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								jsval *argv, jsval *rval);
	static void JSDestructor(JSContext *cx, JSObject *obj);
	static JSObject *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);
	static JSBool Load(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Append(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Delete(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Save(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Exit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
/*	static JSBool SaveDVD(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool SaveOGM(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);*/
	static JSBool GoToTime(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool LoadFilters(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool ClearSegments(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool AddSegment(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool forceUnpack(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool setContainer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool rebuildIndex(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool smartcopyMode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

        

	static JSPropertySpec avidemux_properties[];
	static JSFunctionSpec avidemux_methods[];
	enum
	{
		markerA_prop,
		markerB_prop,
		video_prop,
		audio_prop,
		container_prop,
		currentframe_prop,
		fps_prop
	};
	static JSClass m_classAvidemux;

//protected:
	void setObject(ADM_Avidemux *pObject);
	ADM_Avidemux *getObject();

private:
	ADM_Avidemux *m_pObject;

};

#endif
