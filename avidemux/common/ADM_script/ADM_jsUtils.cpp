/**
    \file ADM_jsUtils
    \brief Simple param -> type utilities
    \author mean fixounet@free.fr 2009
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include <string.h>
#include <pthread.h>
#include "DIA_coreToolkit.h"

#include "ADM_JSGlobal.h"
#include "ADM_JSDebug.h"
#include "ADM_JSif.h"
#include "ADM_jsUtils.h"
/**
    \fn ADM_jsArg2Vars  
    \brief convert jsvals to native type with checking
*/
bool ADM_jsArg2Vars(JSContext *cx, JSObject *obj, int argc, jsval *argv, int paramNumber, ADM_PARAM_LIST *param)
{
    if(paramNumber!=argc)
    {
        ADM_warning("Wrong number of parameters : %d vs %d\n",argc,paramNumber);
        return false;
    }
    for(int i=0;i<argc;i++)
    {
        jsval j=argv[i];
        ADM_PARAM_LIST *p=param+i;
        switch(p->type)
        {
            case ADM_JS_UINT64_T:
            case ADM_JS_UINT32_T:
            case ADM_JS_INT64_T:
            case ADM_JS_INT32_T:
                {
                        if(!JSVAL_IS_NUMBER(j))
                        {
                            ADM_warning("Expected number and got %d\n",j);
                            return false;
                        }
                        // If it is an int...
                        double v=0;
                        if(JSVAL_IS_INT(j)) 
                        {
                            v=(int64_t)JSVAL_TO_INT(j);
                            //ADM_warning("Value is int :%"LLD"\n",JSVAL_TO_INT(j));
                        }
                        if(JSVAL_IS_DOUBLE(j)) 
                        {
                            v=(int64_t)*(JSVAL_TO_DOUBLE(j));
                            //ADM_warning("Value is float :%f\n",(float)*(JSVAL_TO_DOUBLE(j)));
                        }
                        // 
                        //ADM_warning("%f\n",(float)v);
                        // Affect
                        switch(p->type)
                        {
                            case ADM_JS_UINT64_T: *(uint64_t *)p->value=(uint64_t)v;break;
                            case ADM_JS_UINT32_T: *(uint32_t *)p->value=(uint32_t)v;break;
                            case ADM_JS_INT64_T:  *(int64_t *)p->value=(int64_t)v;break;
                            case ADM_JS_INT32_T:  *(int32_t *)p->value=(int32_t)v;break;
                            default: ADM_assert(0);break;
                        }
                 }
                        break;
            case  ADM_JS_STRING:
                        if(!JSVAL_IS_STRING(j))
                        {
                            ADM_warning("Expected string and got %d\n",j);
                            return false;
                        }
                        p->value=(void *)(JSVAL_TO_STRING(j));
                        break;
            case  ADM_JS_BOOL:
                        if(!JSVAL_IS_BOOLEAN(j))
                        {
                            ADM_warning("Expected boolean and got %d\n",j);
                            return false;
                        }
                        *(bool *)p->value=JSVAL_TO_BOOLEAN(j);
                        break;
            default:
                    ADM_assert(0);
                    break;
        }

    }
    return true;
}
// EOF
