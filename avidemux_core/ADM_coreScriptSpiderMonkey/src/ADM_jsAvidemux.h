/**
    \file ADM_jsAvidemux
    \brief Standard includes and defines
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_JS_AVIDEMUX_H
#define ADM_JS_AVIDEMUX_H
#include "jsapi.h"
#include "ADM_inttype.h"

#ifdef __cplusplus
extern "C" {
#endif

// non jsapigen function, variables number of args
JSBool jsAdmaddVideoFilter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool jsAdmaudioCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool jsAdmsetContainer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool jsAdmvideoCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

int   jsVideoCodec(const char *a,const char **b);
JSBool jsAdmvideoCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
int   jsVideoFilter(const char *a,const char **b);
JSBool jsAdmaddVideoFilter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
int jsAudioCodec(const char *a,const char **b);
JSBool jsAdmaudioCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
int   jsSetContainer(const char *a,const char **b);
JSBool jsAdmsetContainer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
void jsClearVideoFilters(JSContext *cx);
int jsAudioMixer(JSContext *cx, const char *s);
void jsAudioReset(JSContext *cx);
char *jsGetVideoCodec(JSContext *cx);
int jsGetFps1000(JSContext *cx);
int jsGetWidth(JSContext *cx);
int jsGetHeight(JSContext *cx);
int jsAddSegment(JSContext *cx, int ref, double start, double duration);
void jsClearSegments(JSContext *cx);
int jsSetPostProc(JSContext *cx, int a, int b, int c);
int jsAppendVideo(JSContext *cx, const char *s);
double jsGetMarkerA(JSContext *cx);
double jsGetMarkerB(JSContext *cx);
void jsSetMarkerA(JSContext *cx, double a);
void jsSetMarkerB(JSContext *cx, double b);
uint32_t jsGetResample(JSContext *cx);
void jsSetResample(JSContext *cx, uint32_t fq);
int jsLoadVideo(JSContext *cx, const char *s);

#ifdef __cplusplus
};
#endif

#endif
