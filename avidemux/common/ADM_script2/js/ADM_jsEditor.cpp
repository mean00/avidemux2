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
#include "ADM_scriptEditor.h"
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
        int64_t delta=0;
        if(pts!=ADM_NO_PTS && dts!=ADM_NO_PTS) delta=(int64_t)pts-(int64_t)dts;
        jsLog("Frame %"LU" : Flags 0x%"LX" pts=%"LLD" dts=%"LLD" delta=%"LLD" ms",framenumber,flags,pts,dts,delta/1000LL);
    }else
    {
        jsLog("Cannot get info for frame %"LU,framenumber);
    }
    return 0;
}

/**
    \fn    jsDumpSegments
    \brief dump segment, video & all
*/
int jsDumpSegments (void)
{// begin PostProcess
        enterLock();
        video_body->dumpSegments();
        leaveLock(); 
        return 0;
}// end PostProcess
/**
        \fn jsDumpRefVideos
*/
int jsDumpRefVideos (void)
{
        enterLock();
        video_body->dumpRefVideos();
        leaveLock(); 

        return 0;
}
/**
    \fn dumpTiming
    \brief dump segment, video & all
*/
JSBool dumpTiming(void)
{// begin PostProcess
        
  
        enterLock();
        video_body->dumpTiming();
        leaveLock(); 
        
        return 0;
}// end PostProcess
// EOF
