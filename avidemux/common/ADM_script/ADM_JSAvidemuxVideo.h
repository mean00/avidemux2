#ifndef _ADM_JSAVIDEMUXVIDEO_H
#define _ADM_JSAVIDEMUXVIDEO_H

#pragma once

// Spidermonkey
#include "ADM_libraries/ADM_smjs/jsapi.h"
#include "ADM_AvidemuxVideo.h"

class ADM_JSAvidemuxVideo
{
public:
	ADM_JSAvidemuxVideo(void) : m_pObject(NULL) {}
	virtual ~ADM_JSAvidemuxVideo(void);

	static JSBool JSGetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool JSSetProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
	static JSBool JSConstructor(JSContext *cx, JSObject *obj, uintN argc, 
								jsval *argv, jsval *rval);
	static void JSDestructor(JSContext *cx, JSObject *obj);
	static JSObject *JSInit(JSContext *cx, JSObject *obj, JSObject *proto = NULL);
	static JSBool Clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Add(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool ClearFilters(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool AddFilter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool IndexMPEG(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Codec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool codecPlugin(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool CodecConf(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool Save(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool SaveJPEG(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool ListBlackFrames(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool PostProcess(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool RebuildIBFrames(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

	static JSBool SetFps1000(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetFps1000(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool GetNbFrames(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetWidth(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetHeight(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool GetFCC(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

	static JSBool isVopPacked(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool hasGmc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool hasQpel(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
        static JSBool getFrameSize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
	static JSBool getFrameType(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);


	static JSPropertySpec avidemuxvideo_properties[];
	static JSFunctionSpec avidemuxvideo_methods[];
	enum
	{
                videoprocess_prop,
	};
	static JSClass m_classAvidemuxVideo;

//protected:
	void setObject(ADM_AvidemuxVideo *pObject);
	ADM_AvidemuxVideo *getObject();

private:
	ADM_AvidemuxVideo *m_pObject;

};

#endif
