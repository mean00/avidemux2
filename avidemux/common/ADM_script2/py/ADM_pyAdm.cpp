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

#include "ADM_cpp.h"
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
#include "ADM_scriptUtils.h"
#include "ADM_scriptGui.h"

#include "scriptDialogFactory/ADM_scriptDFToggle.h"
#include "scriptDialogFactory/ADM_scriptDFInteger.h"
#include "scriptDialogFactory/ADM_scriptDFMenu.h"
#include "scriptDialogFactory/ADM_scriptDialogFactory.h"

#define ADM_PYID_AVIDEMUX 100
#define ADM_PYID_EDITOR   101
#define ADM_PYID_GUI      102
#define ADM_PYID_DIALOGF     200
#define ADM_PYID_DF_TOGGLE   201
#define ADM_PYID_DF_INTEGER  202
#define ADM_PYID_DF_MENU     203


#include "adm_gen.cpp"
#include "editor_gen.cpp"
#include "GUI_gen.cpp"

#include "pyHelpers_gen.cpp"

#include "pyDFToggle_gen.cpp"
#include "pyDFInteger_gen.cpp"
#include "pyDFMenu_gen.cpp"
#include "pyDialogFactory_gen.cpp"

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
    py->registerClass("Avidemux",initClasspyAdm);
    py->registerClass("Editor",initClasspyEditor);
    py->registerClass("Gui",initClasspyGui);
    py->registerClass("DFToggle",initClasspyDFToggle);
    py->registerClass("DFInteger",initClasspyDFInteger);
    py->registerClass("DFMenu",initClasspyDFMenu);
    py->registerClass("DialogFactory",initClasspyDialogFactory);

    py->registerFuncs("test",pyHelpers_functions);

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

