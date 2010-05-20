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
bool A_setContainer(const char *cont);
/**
    \fn Codec
    
*/
extern "C" int   jsSetContainer(const char *a,const char **b) {return 0;}
JSBool jsAdmsetContainer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{// begin Codec
        
        // default return value
        *rval = BOOLEAN_TO_JSVAL(false);
        if(argc < 1)
        {
            jsLog(JS_LOG_NORMAL,"setContainer needs at least one arg\n");
            return JS_FALSE;
        }
        
        if(JSVAL_IS_STRING(argv[0]) == false)
        {
                jsLog(JS_LOG_NORMAL,"setContainer needs at string arg\n");
                return JS_FALSE;
        }
        char *str = JS_GetStringBytes(JSVAL_TO_STRING(argv[0]));
         jsLog(JS_LOG_NORMAL,"[JS] Selecting container :%s\n",str);
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
