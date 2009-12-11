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
#ifdef __cplusplus
extern "C" {
#endif

int jsLoadVideo(const char *c);
int jsAppendVideo(const char *c);
int jsClearSegments(void);
int jsAddSegment(int ref, double start, double duration);
JSBool jsAdmvideoCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
#ifdef __cplusplus
};
#endif

#endif
