/**
    \file ADM_jsLoad.cpp
    \brief Load oriented functions
    \author mean (c) 2009 fixounet@free.fr


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
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_jsAvidemux.h"
#include "A_functions.h"
extern ADM_Composer *video_body;
/**
    \fn jsLoadFile
*/
int jsLoadVideo(const char *s)
{
int ret=0;
        enterLock();
        if(A_openAvi(s)) 
        {
          ret=1;
        }
        leaveLock();
    return ret;
}

/**
    \fn jsAppendFile
*/
int jsAppendVideo(const char *s)
{
int ret=0;
        enterLock();
        if(A_appendAvi(s)) 
        {
          ret=1;
        }
        leaveLock();
    return ret;
}

/**
    \fn jsClearSegments
*/
int jsClearSegments(void)
{
    video_body->clearSegment();
    return 1;
}
/**
    \fn jsAddSegment

*/
int  jsAddSegment(int ref, double start, double duration)
{
    if(true==video_body->addSegment(ref,(uint64_t)start,(uint64_t)duration)) return 1;
    return 0;
}


// EOF