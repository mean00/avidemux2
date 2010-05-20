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
#include "ADM_script2/include/ADM_jsShell.h"
#include "A_functions.h"

#include "adm_gen.h"

#include "adm_gen.cpp"
/**
    \fn    parseTinyPyScript
    \brief Execute a tinyPy script
*/
bool parseTinyPyScript(const char *name)
{
        tinyPy py;
        py.init();
        py.registerFuncs("adm",adm_functions);
        return py.execFile(name);
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
    myPy->init();
    myPy->registerFuncs("adm",adm_functions);

    ADM_startShell(pyEvaluate);
    delete myPy;
    myPy=NULL;
	A_Resync();
    ADM_info("Ending py shell...\n");
	return true;
}

int  py_loadVideo (char *vid )
{
    printf("pyLoadVideo %s\n",vid);
    return 0;
}
int  py_clearSegments (void)
{
    printf("py_clearSegments\n");
    return 0;
}

int  py_appendVideo (char *vid )
{
    printf("py_appendVideo %s\n",vid);
    return 0;
}

int  py_addSegment (int ,float , float )
{
    printf("py_addSegment\n");
    return 0;
}

int  py_setPostProc (int a,int b, int c)
{
    printf("py_setPostProc\n");
    return 0;
}

int  py_getWidth (void)
{
    printf("py_getWidth\n");
    return 0;
}

int  py_getHeight (void)
{
    printf("py_getHeight\n");
    return 0;
}

int  py_getFps1000 (void)
{
    printf("py_getFps1000\n");
    return 0;
}

int  py_audioReset (void)
{
    printf("py_audioReset\n");
    return 0;
}

int  py_audioMixer (char *mixer )
{
    printf("py_audioMixer\n");
    return 0;
}

int  py_clearVideoFilters (void)
{
    printf("py_clearVideoFilters\n");
    return 0;
}

