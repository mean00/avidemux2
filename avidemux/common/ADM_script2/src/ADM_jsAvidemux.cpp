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
#include "ADM_muxerProto.h"
#include "GUI_ui.h"
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
    UI_SetCurrentFormat(idx);
    return true;
}

/**
    \fn jsLoadFile
*/
int jsLoadVideo(const char *s)
{
int ret=0;
        
        if(A_openAvi(s)) 
        {
          ret=1;
        }
        
    return ret;
}

/**
    \fn jsAppendFile
*/
int jsAppendVideo(const char *s)
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
    \fn Codec
    
*/
extern "C" int   jsSetContainer(const char *a,const char **b) {return 0;}
JSBool jsAdmsetContainer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Codec
    
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc < 1)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false)
                return JS_FALSE;
        char *str = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        
        if(A_setContainer(str))
        {
            CONFcouple *c;
            jsArgToConfCouple(argc-1,&c,argv+1);
            int idx=ADM_MuxerIndexFromName(str);
            if(idx!=-1)
            {
                *rval = BOOLEAN_TO_JSVAL( ADM_mx_setExtraConf(idx,c));
            }
            if(c) delete c;
        }
        return JS_TRUE;
}// end Codec


// EOF