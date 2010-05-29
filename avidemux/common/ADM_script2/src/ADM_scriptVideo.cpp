/**
    \file ADM_jsVideo.cpp
    \brief Video oriented functions
    \author mean (c) 2009 fixounet@free.fr


    jsapigen does not like much variable number of arguments
    In that case, we patch the generated file to go back to native spidermonkey api


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
//#include "ADM_js.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_scriptAvidemux.h"
#include "ADM_scriptVideo.h"
#include "A_functions.h"
#include "ADM_videoEncoderApi.h"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"
#include "GUI_ui.h"
#include "ADM_scriptCommon.h"
#include "ADM_scriptIf.h"
extern ADM_Composer *video_body;
bool A_setVideoCodec(const char *nm);
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
/**
    \fn scriptSetVideoCodec
*/
int     scriptSetVideoCodec(const char *codec,CONFcouple *c)
{       
        bool r=true;
        if(A_setVideoCodec(codec)==false)
        {
            jsLog("Could not select codec %s\n",codec);
            r=false;
        }else
        {        
            if(c)
            {
                r=videoEncoder6_SetConfiguration(c);
            }
        }
        if(c)
            delete c;
        return r;
}
/**
    \fn jsSetPostProc
*/
int jsSetPostProc (int a,int b, int c)
{
    return video_body->setPostProc(a,b,c);
}


/**
     \fn jsClearFilters
*/
int jsClearVideoFilters()
{
    return ADM_vf_clearFilters();
}
/**
    \fn jsGetMarkerA
*/
double jsGetMarkerA(void)
{
    return (double)video_body->getMarkerAPts();

}
/**
    \fn jsGetMarkerB
*/
double jsGetMarkerB(void)
{
    return (double)video_body->getMarkerBPts();
}
/**
    \fn jsSetMarkerA
*/
void   jsSetMarkerA(double a)
{
    video_body->setMarkerAPts( (uint64_t)a);
}
/**
    \fn jsGetMarkerB
*/
void   jsSetMarkerB(double b)
{
    video_body->setMarkerBPts( (uint64_t)b);
}

//EOF
