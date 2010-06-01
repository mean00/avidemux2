/***************************************************************************
    \file  ADM_tinypy.cpp
    \brief Wrapper around tinypy
    \author mean fixounet@free.fr (c) 2010

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include <stdarg.h>
#include "ADM_tinypy.h"
//extern "C"
//{
#include "tinypy.h"
#include "init_math.cpp"
//}
#define INSTANCE ((tp_vm *)instance)
#define SCRIPT   ((tp_obj *)script)

pyLoggerFunc *pyLog=NULL;
static tp_obj    tinyPy_dumpBuiltin(tp_vm *vm);
static pyFuncs addons[]={{"help",tinyPy_dumpBuiltin},{NULL,NULL}};
/**

*/
bool    tinyPy::registerLogger(pyLoggerFunc func)
{
    pyLog=func;
    return true;
}
bool    tinyPy::unregisterLogger(void)
{
    pyLog=NULL;
    return true;
}
/**
    \fn pyPrintfd
*/
bool pyPrintf(const char *fmt,...)
{
        static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	fmt);
		vsnprintf(print_buffer,1023,fmt,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        //printf("%s",print_buffer);
        //jsLog(JS_LOG_NORMAL,print_buffer);
        if(!pyLog) printf("%s",print_buffer);
            else pyLog(print_buffer);
        return true;
}

/**
    \fn tinypy
    \brief ctor
*/
tinyPy::tinyPy(void)
{
    instance=NULL;
}
/**
    \fn tinypy
    \brief dtor

*/
tinyPy::~tinyPy()
{
    if(INSTANCE)
    {
        tp_deinit(INSTANCE);
        instance=NULL;
    }
}
/**
    \fn tinypy
    \brief init
*/
bool tinyPy::init(void)
{
    ADM_warning("Init tinypy\n");
    ADM_assert(!instance);
    instance=(void *)tp_init(0,NULL);
    if(!instance)
    {
        ADM_error("Cannot initialize tinypy\n");
        return false;   
    }
    math_init(INSTANCE);
    registerFuncs("addons",addons);
    return true;
}
/**
    \fn execString
    \brief eval a python string
*/
bool tinyPy::execString(const char *s)
{
    if(!instance)
    {
        ADM_warning("No instance\n");
        return false;
    }
    tp_obj name = tp_string("avidemux6");
    tp_obj program = tp_string(s);
    if(!setjmp(INSTANCE->nextexpr))
    {
        tp_obj c=tp_eval(INSTANCE,s,INSTANCE->builtins);
    }
    else     
    {
        return false;
    }
    return true;
}
/**
    \fn execFile
    \brief execute a python script
*/
bool tinyPy::execFile(const char *f)
{
    if(!instance)
    {
        ADM_warning("No instance\n");
        return false;
    }
    if(!setjmp(INSTANCE->nextexpr))
    {
        tp_import(INSTANCE,f,"avidemux6",NULL,0);
    }
    else     
    {
        return false;
    }
    return true;
}
/**
    \fn dumpInternals
*/
bool tinyPy::dumpInternals(void)
{
  
    return true;

}
/**
    \fn registerFuncs
*/
bool    tinyPy::registerFuncs(const char *group,pyFuncs *funcs)
{
    ADM_info("Registering group %s\n",group);
    while(funcs->funcName)
    {
        ADM_info("Registering :%s\n",funcs->funcName);
        tp_set(INSTANCE, INSTANCE->builtins, 
                tp_string(funcs->funcName), 
                tp_fnc(INSTANCE, funcs->funcCall));
        funcs++;
    }
    return true;
}
/**
    \fn registerClass
*/
bool    tinyPy::registerClass(const char *className,pyRegisterClass classPy)
{
    ADM_info("Registering class:%s\n",className);
    tp_set(INSTANCE, INSTANCE->builtins, tp_string(className), classPy(INSTANCE));
    return true;

}
/**
    \fn dumpBuiltin
*/
tp_obj    tinyPy_dumpBuiltin(tp_vm *vm)
{
    ADM_info("Dumping builtins\n");
    tp_obj builtins=vm->builtins;
    // It is a dict..
    _tp_dict *dict=builtins.dict.val;
    ADM_info("%d elems\n",dict->len);
    for( int i=0;i<dict->len;i++)
    {
        tp_item *item=&(dict->items[i]);
        const char *str=item->key.string.val;
        if(str)
            pyPrintf("%s\n",str);
    }
    return tp_None;
}
//*********************************************
#define preamble(xtype) tp_obj obj=TP_OBJ();\
                       if(obj.type!=xtype) \
                        { \
                            raise("Expected %s, got %s\n",typeAsString(xtype),typeAsString(obj.type)); \
                        }
/**
   \fn  asInt
*/
 int    tinyParams::asInt(void)
{
    preamble(TP_NUMBER);
    return (int)obj.number.val;
}
/**
   \fn  asFloat
*/
float    tinyParams::asFloat(void)
{
    preamble(TP_NUMBER);
    return (float)obj.number.val;
}
/**
   \fn  asDouble
*/

double tinyParams::asDouble(void)
{
    preamble(TP_NUMBER);
    return (double)obj.number.val;
}
/**
   \fn  asString
*/

const char *tinyParams::asString(void)
{
    preamble(TP_STRING);
    return obj.string.val;
}
/**
   \fn  asThis
*/

void  *tinyParams::asThis(tp_obj *self,int id)
{
    tp_obj cdata = tp_get(tp, *self, tp_string("cdata"));
    if(cdata.data.magic!=id) 
    { 
        raise("Bad class : Expected %d, got %d\n",id,cdata.data.magic); \
    }
    return cdata.data.val;
}

/**
   \fn  typeAsString
    \brief return the type given as a string
*/

const char *tinyParams::typeAsString(int type)
{
    switch(type)
    {
        case TP_NUMBER: return "Number";break;
        case TP_STRING: return "String";break;
        case TP_LIST:   return "List";break;
        case TP_DICT:   return "Dict";break;
        case TP_FNC:    return "Function";break;
        case TP_DATA:   return "Data";break;
    }
    return "???";
}
/**
    \fn raise
    \brief raise an exception
*/
void tinyParams::raise(const char *fmt,...)
{
char print_buffer[1024];
    va_list         list;
    va_start(list,  fmt);
    vsnprintf(print_buffer,1023,fmt,list);
    va_end(list);
    print_buffer[1023]=0; // ensure the string is terminated
    ADM_error("%s",print_buffer);
    _tp_raise(tp,tp_None);
}
/**
    \fn makeCouples
    \brief convert couples into char *first and *couples c, c can be null
*/
bool    tinyParams::makeCouples(CONFcouple **c)
{
    int nb=nbParamsLeft();
    if(!nb)
    {
        *c=NULL;
        return true;
    }
    const char *s[nb];
    for(int i=0;i<nb;i++) s[i]=asString();
    return stringsToConfCouple(nb,c,s);
}
// EOF