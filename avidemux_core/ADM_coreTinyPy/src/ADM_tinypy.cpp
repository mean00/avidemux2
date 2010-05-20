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

bool pyPrintf(const char *fmt,...)
{
        static char print_buffer[1024];
  	
		va_list 	list;
		va_start(list,	fmt);
		vsnprintf(print_buffer,1023,fmt,list);
		va_end(list);
		print_buffer[1023]=0; // ensure the string is terminated
        printf("TP>%s",print_buffer);
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
    if(instance) return true;   
    math_init(INSTANCE);
    ADM_error("Cannot initialize tinypy\n");
    return false;
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
    printf("Registering group %s\n",group);
    while(funcs->funcName)
    {
        printf("Registering :%s\n",funcs->funcName);
        tp_set(INSTANCE, INSTANCE->builtins, 
                tp_string(funcs->funcName), 
                tp_fnc(INSTANCE, funcs->funcCall));
        funcs++;
    }
    return true;
}
// EOF