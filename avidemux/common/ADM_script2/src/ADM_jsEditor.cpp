/**
    \file ADM_JSif.cpp
    \brief interface to js
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_js.h"
#include <stdarg.h>
#include <vector>
#include "ADM_jsEditor.h"
#include "ADM_editor/ADM_edit.hxx"
extern ADM_Composer *video_body;

/**
    \fn jsPrintTiming
*/
int jsPrintTiming(int framenumber )
{
    uint32_t flags;
    uint64_t pts,dts;
    if(true==video_body->getVideoPtsDts(framenumber, &flags,&pts,&dts))
    {
        jsLog(JS_LOG_NORMAL,"Frame %"LU" : Flags 0x%"LX" pts=%"LLD" dts=%"LLD"\n",framenumber,flags,pts,dts);
    }else
    {
        jsLog(JS_LOG_NORMAL,"Cannot get info for frame %"LU,framenumber);
    }
    return 0;
}
// EOF
