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
#include "ADM_scriptIf.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_scriptCommon.h"
#include "fourcc.h"
extern ADM_Composer *video_body;
/**
    \fn jsGetWidth
*/
int scriptGetWidth ( void)
{
aviInfo info;
        video_body->getVideoInfo(&info);
        return info.width;
}
/**
    \fn jsGetHeight
*/
int scriptGetHeight ( void)
{
aviInfo info;
        video_body->getVideoInfo(&info);
        return info.height;
}
/**
    \fn jsGetFps1000
*/
int scriptGetFps1000 ( void)
{
aviInfo info;
        video_body->getVideoInfo(&info);
        return info.fps1000;
}
/**
    \fn jsGetVideoCodec
*/
char *scriptGetVideoCodec ( void)
{
uint32_t fcc;
aviInfo info;
        video_body->getVideoInfo(&info);
        fcc=info.fcc;
        ADM_strdup(fourCC::tostring(fcc));
}
// EOF
