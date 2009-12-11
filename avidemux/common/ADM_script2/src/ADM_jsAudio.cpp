/**
    \file ADM_jsAudio.cpp
    \brief Audio oriented functions
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
#include "A_functions.h"
#include "GUI_ui.h"
#include "ADM_audioFilterInterface.h"
#include "audioEncoderApi.h"
extern ADM_Composer *video_body;
/**
    \fn int jsAudioReset(void);
*/
int jsAudioReset (void)
{
    audioFilterReset();
    return 1;
}
/**
    \fn 
*/  
extern "C" int   jsAudioCodec(const char *a,const char **b) {return 0;}
JSBool jsAdmaudioCodec(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc < 2)
                return JS_FALSE;
        if(JSVAL_IS_STRING(argv[0]) == false || JSVAL_IS_INT(argv[1]) == false )            return JS_FALSE;
        for(int i=2;i<argc;i++)  if(JSVAL_IS_STRING(argv[i]) == false) return JS_FALSE;

        // Get Codec...
        char *name = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
        ADM_LowerCase(name);
        
        // First search the codec by its name
        if(!audioCodecSetByName(name))
        {
                *rval = BOOLEAN_TO_JSVAL(false);
                jsLog(JS_LOG_ERROR,"Cannot set audio codec %s\n",name);
        }
        else
        {
            // begin set bitrate
            uint32_t bitrate=JSVAL_TO_INT(argv[1]);
            // Construct couples
            CONFcouple *c=NULL;
            if(argc>2)
            {
                int nb=argc-2;
                jsArgToConfCouple( nb,&c,  argv+2);
            }
            *rval = BOOLEAN_TO_JSVAL(setAudioExtraConf(bitrate,c));
            if(c) delete c;
        }

        // end set bitrate
        
        return JS_TRUE;
}


//EOF