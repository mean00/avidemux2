/**
    \file ADM_jsUtils
    \brief Simple param -> type utilities
    \author mean fixounet@free.fr 2009
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_JS_UTILS_H
#define ADM_JS_UTILS_H

typedef enum
{
    ADM_JS_INVALID=0,
    ADM_JS_UINT32_T=1,
    ADM_JS_INT32_T,
    ADM_JS_UINT64_T,
    ADM_JS_INT64_T,
    ADM_JS_STRING,
    ADM_JS_BOOL,
    ADM_JS_MAX
}ADM_PARAM_TYPE;

typedef struct
{
    ADM_PARAM_TYPE type;
    void           *value;

}ADM_PARAM_LIST;

bool ADM_jsArg2Vars(JSContext *cx, JSObject *obj, int argc, jsval *argv, int paramNumber, ADM_PARAM_LIST *param);


#endif
