/***************************************************************************
    \file Q_gui2_menu.cpp
    \brief Handle custom menus
    \author mean/gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"
#include "ADM_inttype.h"
#include <string>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QKeyEvent>
#include <QtGui/QGraphicsView>

#include "Q_gui2.h"
#include "ADM_default.h"

#include "gui_action.hxx"
#include "DIA_fileSel.h"
#include "ADM_vidMisc.h"
#include "prefs.h"
#include "avi_vars.h"

#include "ADM_render/GUI_renderInternal.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_muxerProto.h"
#include "T_vumeter.h"
#include "DIA_coreToolkit.h"

using std::string;

#define JS_CUSTOM 0
#define PY_CUSTOM 1
#define PY_AUTO   2
#define NB_POOL 3
static char     *customNames[3][ADM_MAX_CUSTOM_SCRIPT];
static QAction  *customActions[3][ADM_MAX_CUSTOM_SCRIPT];
static uint32_t ADM_nbCustom[3]={0,0,0};
/**
    \fn clearCustomMenu
*/
void MainWindow::clearCustomMenu(void)
{
    for(int pool=0;pool<NB_POOL;pool++)
	if (ADM_nbCustom[pool])
	{
		for(int i = 0; i < ADM_nbCustom[pool]; i++)
		{
            switch(pool)
            {
            case PY_CUSTOM:
                disconnect(customActions[pool][i], SIGNAL(triggered()), this, SLOT(customPy()));
                break;
            case JS_CUSTOM:
                disconnect(customActions[pool][i], SIGNAL(triggered()), this, SLOT(customJs()));
                break;
            case PY_AUTO:
                disconnect(customActions[pool][i], SIGNAL(triggered()), this, SLOT(autoPy()));
                break;
            }
			delete customActions[pool][i];
			delete customNames[pool][i];
		}
		ADM_nbCustom[pool] = 0;
	}
    pyMenu->clear();
    jsMenu->clear();
    autoMenu->clear();
}
/**
    \fn buildCustomMenu
*/
void MainWindow::buildCustomMenu(void)
{
	clearCustomMenu();

	char *customdir = ADM_getCustomDir();

	if (!customdir)
	{
		printf("No custom dir...\n");
		return;
	}
    string customFolder(customdir);
    for(int pool=0;pool<NB_POOL;pool++)
    {
        string myDir,subDir;
        string ext;
        /* Collect the name */
        switch(pool)
        {
            case JS_CUSTOM:
                subDir=string("/js/");
                ext=string(".js");
                break;
            case PY_CUSTOM:
                subDir=string("/py/");
                ext=string(".py");
                break;
            case PY_AUTO:
                continue;
        }
        myDir=customFolder+subDir;
        if (! buildDirectoryContent(&(ADM_nbCustom[pool]), myDir.c_str(), customNames[pool], ADM_MAX_CUSTOM_SCRIPT,ext.c_str()))
        {
            ADM_warning("Failed to build custom dir content (%s)\n",myDir.c_str());
            continue;
        }

        if(ADM_nbCustom[pool])
        {
            ADM_info("Found %u custom script(s), adding them (%s)\n", ADM_nbCustom[pool],subDir.c_str());

            for(int i=0; i < ADM_nbCustom[pool]; i++)
            {
                QAction *action= new QAction(QString::fromUtf8(ADM_GetFileName(customNames[pool][i])), NULL);
                //ADM_info("\t%s\n",ADM_GetFileName(customNames[pool][i]));
                customActions[pool][i] = action;
                switch(pool)
                {
                case JS_CUSTOM:
                {
                    jsMenu->addAction(action);
                    connect(action, SIGNAL(triggered()), this, SLOT(customJs()));
                    break;
                }
                case PY_CUSTOM:
                {
                    pyMenu->addAction(action);
                    connect(action, SIGNAL(triggered()), this, SLOT(customPy()));   
                    break;
                }
                case PY_AUTO:
                    continue;
                }
            }
        }
        else
            ADM_info("No custom scripts\n");
    }
	ADM_info("Custom menu built\n");
}

/**
\fn     custom
\brief  Invoked when one of the custom script has been called
*/
void MainWindow::customScript(int pool,int base,QObject *ptr)
{
	
	for(int i=0;i<ADM_nbCustom[pool];i++)
	{
		if(customActions[pool][i]==ptr)
		{
			printf("[Custom] %d/%d scripts\n",i,(int)ADM_nbCustom[pool]);
			HandleAction( (Action)(base+i));
			return; 
		}
	}
	printf("[Custom] Not found\n");

}
void MainWindow::customJs(void)
{
	printf("[Custom] Js Invoked\n");
	QObject *ptr=sender();
	if(!ptr) return;
    customScript(JS_CUSTOM,ACT_CUSTOM_BASE_JS,ptr);
}
void MainWindow::customPy(void)
{
	printf("[Custom] Python Invoked\n");
	QObject *ptr=sender();
	if(!ptr) return;
    customScript(PY_CUSTOM,ACT_CUSTOM_BASE_PY,ptr);
}
void MainWindow::autoPy(void)
{
	printf("[auto] Python Invoked\n");
	QObject *ptr=sender();
	if(!ptr) return;
    customScript(PY_AUTO,ACT_AUTO_BASE_PY,ptr);
}

/**
    Get the custom entry 

*/

const char * GUI_getCustomJsScript(uint32_t nb)
{
	ADM_assert(nb<ADM_nbCustom[JS_CUSTOM]);
	return customNames[JS_CUSTOM][nb];

}
const char * GUI_getCustomPyScript(uint32_t nb)
{
	ADM_assert(nb<ADM_nbCustom[PY_CUSTOM]);
	return customNames[PY_CUSTOM][nb];

}
const char * GUI_getAutoPyScript(uint32_t nb)
{
	ADM_assert(nb<ADM_nbCustom[PY_AUTO]);
	return customNames[PY_AUTO][nb];

}

//********************************************
//EOF
