/**
    \file ADM_jsVideo.cpp
    \brief Video oriented functions
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
#include "ADM_jsVideo.h"
#include "A_functions.h"
#include "ADM_videoEncoderApi.h"
#include "GUI_ui.h"
extern ADM_Composer *video_body;
bool A_setVideoCodec(const char *nm);
/**
    \fn jsSetPostProc
*/
int jsSetPostProc (int a,int b, int c)
{
    return video_body->setPostProc(a,b,c);
}

/**
    \fn jsLoadFile
*/
int jsVideoCodec(const char *codec,const char **params)
{
    // First how many params ?
    const char **p=params;
    int nb=0;
    while(*p) {nb++;p++;}
       
        if(A_setVideoCodec(codec)==false)
        {
            jsLog(JS_LOG_ERROR,"Could not select codec %s\n",codec);
            return 0;
        }
        CONFcouple *c;
        stringsToConfCouple(nb,&c,params);
        if(false==videoEncoder6_SetConfiguration(c))
        {
            jsLog(JS_LOG_NORMAL,"Selected codec %s\n",codec);
            return 0;
        }
    
    return 1;
}

/**
    \fn A_setVideoCodec
*/
bool A_setVideoCodec(const char *nm)
{
    int idx=videoEncoder6_GetIndexFromName(nm);
    if(idx==-1)
    {
        ADM_error("No such encoder :%s\n",nm);
    }
    // Select by index
    videoEncoder6_SetCurrentEncoder(idx);
    UI_setVideoCodec(idx);
    return true;
}
//EOF