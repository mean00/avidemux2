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
#ifndef ADM_JS_EDITOR_H
#define ADM_JS_EDITOR_H
#include "ADM_inttype.h"
//#include "jsapi.h"
#ifdef __cplusplus
extern "C" {
#endif

int jsPrintTiming(int framenumber );
int jsDumpSegments (void);
int jsDumpRefVideos (void);
#ifdef __cplusplus
};
#endif

#endif
