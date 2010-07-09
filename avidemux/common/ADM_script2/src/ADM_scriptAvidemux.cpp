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
#include "ADM_cpp.h"
#include "ADM_editor/ADM_edit.hxx"
#include "A_functions.h"
#include "ADM_muxerProto.h"
#include "GUI_ui.h"
#include "ADM_scriptIf.h"
#include "ADM_scriptCommon.h"
extern ADM_Composer *video_body;
/**
    \fn ADM_JSAvidemux
    \brief Select the current container from a string
*/
bool A_setContainer(const char *cont)
{
    int idx=ADM_MuxerIndexFromName(cont);
    if(idx==-1)
    {
        ADM_error("Cannot find muxer for format=%s\n",cont);
        return false;
    }
    ADM_info("setting container as index %d\n",idx);
    UI_SetCurrentFormat(idx);
    return true;
}

/**
    \fn jsLoadFile
*/
int scriptLoadVideo(const char *s)
{
int ret=0;
        jsLog("[Js] Loading video %s",s);
        if(A_openAvi(s)) 
        {
          ret=1;
        }
        
    return ret;
}

/**
    \fn jsAppendFile
*/
int scriptAppendVideo(const char *s)
{
int ret=0;
        
        if(A_appendAvi(s)) 
        {
          ret=1;
        }
        
    return ret;
}

/**
    \fn jsClearSegments
*/
int scriptClearSegments(void)
{
    video_body->clearSegment();
    return 1;
}
/**
    \fn jsAddSegment

*/
int  scriptAddSegment(int ref, double start, double duration)
{
    if(true==video_body->addSegment(ref,(uint64_t)start,(uint64_t)duration)) 
    {
        if(1==video_body->getNbSegment()) // We just added our first seg...
        {
                ADM_info("First segment, refreshing screen\n");
                A_Rewind();
        }
        return 1;
    }
    return 0;
}
/**
    \fn scriptGetNbSegment()

*/
int  scriptGetNbSegment(void)
{
   return video_body->getNbSegment() ;
}
void scriptDumpSegment(int i)
{
        video_body->dumpSegment(i);
        return ;
}
// EOF
