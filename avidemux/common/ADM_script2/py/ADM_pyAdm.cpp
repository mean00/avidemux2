/**
    \file ADM_pyADM.cpp
    \author mean fixounet@free.fr 2010

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
#include <stdarg.h>
#include <vector>
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_coreTinyPy/include/tinypy.h"
#include "ADM_coreTinyPy/include/ADM_tinypy.h"
#include "ADM_script2/include/ADM_scriptShell.h"
#include "A_functions.h"
#include "ADM_scriptCommon.h"
#include "ADM_scriptVideo.h"
#include "ADM_scriptAudio.h"
#include "ADM_scriptEditor.h"

#include "adm_gen.cpp"
#include "editor_gen.cpp"
extern pyRegisterClass initClasspyAdm;
extern pyRegisterClass initClasspyEditor;
/**

*/
static  bool pyLogger(const char *s)
{
    jsLog(s);
    return true;
}
/**
    \fn initPy
*/  
static bool initPy(tinyPy *py)
{
    py->init();
    //py->registerFuncs("adm",adm_functions);
    py->registerClass("adm",initClasspyAdm);
    py->registerClass("Editor",initClasspyEditor);
    tinyPy::registerLogger(pyLogger);
    return true;
}
static bool deinitPy(tinyPy *py)
{
    tinyPy::unregisterLogger();
    return true;
}
/**
    \fn    parseTinyPyScript
    \brief Execute a tinyPy script
*/
bool parseTinyPyScript(const char *name)
{
        tinyPy py;
        initPy(&py);
        bool r=py.execFile(name);
        deinitPy(&py);
        return r;
}

/**
    \fn jsEvaluate
*/
static tinyPy *myPy=NULL;
static bool pyEvaluate(const char *str)
{
    ADM_assert(myPy);
    return myPy->execString(str);
}
/**
    \fn    interactiveTinyPy
    \brief interprete & execute python script (interactive)
*/
bool interactiveTinyPy(void)
{
    myPy=new tinyPy;
    initPy(myPy);
    ADM_startShell(pyEvaluate);
    deinitPy(myPy);
    delete myPy;
    myPy=NULL;
	A_Resync();
    ADM_info("Ending py shell...\n");
	return true;
}

// EOF

