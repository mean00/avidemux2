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
#include "ADM_tinypy.h"
//extern "C"
//{
#include "tinypy.h"
//}
#define INSTANCE ((tp_vm *)instance)
#define SCRIPT   ((tp_obj *)script)

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
    ADM_error("Cannot initialize tinypy\n");
    return false;
}
/**
    \fn tinypy
    \brief execString
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
        tp_obj c = tp_compile(INSTANCE, program, name);
        tp_exec(INSTANCE, c,INSTANCE->builtins);
    }
    else     
    {
        return false;
    }
    return true;
}

// EOF