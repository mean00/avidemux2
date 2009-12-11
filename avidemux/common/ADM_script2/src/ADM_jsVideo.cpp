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
#include "ADM_js.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_jsAvidemux.h"
#include "ADM_jsVideo.h"
#include "A_functions.h"
#include "ADM_videoEncoderApi.h"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"
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
    \fn Codec
    
*/
extern "C" int   jsVideoCodec(const char *a,const char **b) {return 0;}
JSBool jsAdmvideoCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Codec
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc <1)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false )
        {
                jsLog(JS_LOG_ERROR,"Cannot set codec, first parameter is not a string\n");
                return JS_FALSE;
        }
        char *codec=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        // Set codec.
        
        if(A_setVideoCodec(codec)==false)
        {
            jsLog(JS_LOG_ERROR,"Could not select codec %s\n",codec);
            return JS_FALSE;
        }
        CONFcouple *c;
        jsArgToConfCouple(argc-1,&c,argv+1);
        *rval = BOOLEAN_TO_JSVAL( videoEncoder6_SetConfiguration(c));
        jsLog(JS_LOG_NORMAL,"Selected codec %s\n",codec);
        if(c) delete c;
        return JS_TRUE;
}// end Codec

/**
    \fn Codec
    
*/
extern "C" int   jsVideoFilter(const char *a,const char **b) {return 0;}
JSBool jsAdmaddVideoFilter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Codec
   uint32_t filterTag;

        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc == 0)
                return JS_FALSE;
        char *filterName=JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        filterTag = ADM_vf_getTagFromInternalName(filterName);
        jsLog(JS_LOG_NORMAL,"Adding Filter %s -> %"LU"... \n",filterName,filterTag);

        
        CONFcouple *c=NULL;
        if(argc)
            jsArgToConfCouple(argc-1,&c,argv+1);
        *rval=BOOLEAN_TO_JSVAL(  ADM_vf_addFilterFromTag(filterTag,c,false));
        if(c) delete c;
        
        return JS_TRUE;
}// end Codec
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
    \fn jsGetMarkerA
*/
double jsGetMarkerA(void)
{

}
/**
    \fn jsGetMarkerB
*/
double jsGetMarkerB(void)
{

}
/**
    \fn jsSetMarkerA
*/
void   jsSetMarkerA(double a)
{

}
/**
    \fn jsGetMarkerB
*/
void   jsSetMarkerB(double b)
{

}

//EOF