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
#ifndef ADM_TINYPY_H
#define ADM_TINYPY_H
#include "tinypy.h"
/**
    \struct tyFunc
*/
typedef struct
{
    const char *funcName;
    tp_obj (*funcCall)(TP);
}pyFuncs;
typedef bool (pyLoggerFunc)(const char *);

typedef tp_obj (pyRegisterClass)(tp_vm *vm);
/**
    \class tinyPy
*/
class tinyPy 
{
protected:
        void *instance;
public:
                tinyPy(void);
        bool    init(void);
        bool    registerFuncs(const char *group,pyFuncs *funcs);
        bool    registerClass(const char *className,pyRegisterClass *pyclass);
                ~tinyPy(void);
        bool    execString(const char *s);
        bool    execFile(const char *f);
        bool    dumpInternals(void);
static  bool    registerLogger(pyLoggerFunc func);
static  bool    unregisterLogger(void);
        bool    dumpBuiltin(void);
};

/**
    \class tinyParams
*/
class tinyParams
{
protected:
        tp_vm *tp;
public:
        tinyParams(tp_vm *i) {tp=i;}
        int    asInt(void);
        float  asFloat(void);
        double asDouble(void);
const   char  *asString(void);
        int    nbParam(void);
        void   raise(const char *fmt,...);
        const char *typeAsString(int type);
};
#endif
