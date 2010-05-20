/**
    \file ADM_jsDialogFactory.h
    \brief Base class for dialog factory mapping
    \author mean, fixounet@free.fr
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_JS_DIALOG_H
#define ADM_JS_DIALOG_H
#include "ADM_scriptIf.h"
extern "C"  JSObject *jsDialogFactoryInit(JSContext *cx,JSObject *obj);

#endif // ADM_JS_SHELL_H
