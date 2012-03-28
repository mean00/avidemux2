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
#include "ADM_cpp.h"
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

void MainWindow::addScriptEnginesToFileMenu(vector<MenuEntry>& fileMenu)
{
	for (int i = 0; i < fileMenu.size(); i++)
	{
		if (fileMenu[i].type == MENU_SEPARATOR)
		{
			if (this->_scriptEngines.size() > 0)
			{
				fileMenu.insert(
					fileMenu.begin() + i, MenuEntry {MENU_SEPARATOR, "-", NULL, ACT_DUMMY, NULL, NULL});

				i++;
			}

			for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
			{
				vector<MenuEntry>::iterator it = fileMenu.begin() + i;
				Action firstMenuId = (Action)(ACT_SCRIPT_ENGINE_FIRST + (engineIndex * 3));
				string itemName = "Project";

				if (engineIndex > 0)
				{
					itemName = string(_scriptEngines[engineIndex]->name()) + " " + itemName;
				}

				it = fileMenu.insert(it, MenuEntry {MENU_SUBMENU, itemName, NULL, ACT_DUMMY, NULL, NULL});
				it = fileMenu.insert(it + 1, MenuEntry {MENU_SUBACTION, "Run Project...", NULL, firstMenuId, NULL, NULL});
				it = fileMenu.insert(it + 1, MenuEntry {MENU_SUBACTION, "Save as Project...", NULL, (Action)(firstMenuId + 2), NULL, NULL});
				i += 3;
			}

			break;
		}
	}
}

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
            default:
                ADM_assert(0);
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
    char *autoDir =   ADM_getAutoDir();
    ADM_info("Auto script folder %s\n",autoDir);
	if (!customdir)
	{
		printf("No custom dir...\n");
		return;
	}
    string customFolder(customdir);
    string autoFolder(autoDir);
    string topDir;
    for(int pool=0;pool<NB_POOL;pool++)
    {
        string myDir,subDir;
        string ext,topDir;
        /* Collect the name */
        switch(pool)
        {
            case JS_CUSTOM:
                subDir=string("js");
                ext=string(".js");
                topDir=customFolder;
                break;
            case PY_CUSTOM:
                subDir=string("py");
                ext=string(".py");
                topDir=customFolder;
                break;
            case PY_AUTO:
                subDir=string("");
                ext=string(".py");
                topDir=autoFolder;
                break;
            default:
                ADM_assert(0);
                break;
        }
        myDir=topDir+subDir;
        ADM_info("Scanning %s\n",myDir.c_str());
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
                const char *menuName=ADM_GetFileName(customNames[pool][i]);
                char *dot=(char *)menuName; // Same as menuName but writtable
                if(pool==PY_AUTO)
                {
                    if(!strncmp(menuName,"ADM_",4)) // Dont display py script starting by ADM_
                            continue;
                }
                // Remove .py or .js
                uint32_t strLen=strlen(menuName);
                if(strLen>3 && menuName[strLen-3]=='.')
                        dot[strLen-3]=0;
                //
                QAction *action= new QAction(QString::fromUtf8(menuName), NULL);
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
                    autoMenu->addAction(action);
                    connect(action, SIGNAL(triggered()), this, SLOT(autoPy()));
                    break;
                default:
                    ADM_assert(0);
                    break;

                }
            }
        }
        else
            ADM_info("No custom scripts in %s\n",myDir.c_str());
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
