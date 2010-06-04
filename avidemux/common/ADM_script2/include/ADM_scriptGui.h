/**
    \file ADM_scriptGui.h
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
#ifndef ADM_JS_GUI_H
#define ADM_JS_GUI_H
#include "ADM_inttype.h"
#ifdef __cplusplus
extern "C" {
#endif
// All the function returns NULL in case of cancel
char *scriptFileSelRead(const char *title);
char *scriptFileSelWrite(const char *title);
char *scriptDirSelect(const char *title);


#ifdef __cplusplus
};
#endif

#endif
