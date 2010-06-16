/***************************************************************************
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr
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
static char     *customNames[2][ADM_MAC_CUSTOM_SCRIPT];
static QAction  *customActions[1][ADM_MAC_CUSTOM_SCRIPT];
static uint32_t ADM_nbCustom[2]={0,0};
/**
    \fn clearCustomMenu
*/
void MainWindow::clearCustomMenu(void)
{
    for(int pool=0;pool<2;pool++)
	if (ADM_nbCustom[pool])
	{
		for(int i = 0; i < ADM_nbCustom[pool]; i++)
		{
            if(pool==PY_CUSTOM)
                disconnect(customActions[pool][i], SIGNAL(triggered()), this, SLOT(customPy()));
            else
                disconnect(customActions[pool][i], SIGNAL(triggered()), this, SLOT(customJs()));
			delete customActions[pool][i];
			delete customNames[pool][i];
		}
		ADM_nbCustom[pool] = 0;
	}
    pyMenu->clear();
    jsMenu->clear();
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
    for(int pool=0;pool<2;pool++)
    {
        string myDir,subDir;
        /* Collect the name */
        if(JS_CUSTOM==pool) subDir=string("/js/");
        else                subDir=string("/py/");
        myDir=customFolder+subDir;
        if (! buildDirectoryContent(&(ADM_nbCustom[pool]), myDir.c_str(), customNames[pool], ADM_MAC_CUSTOM_SCRIPT,".js"))
        {
            printf("Failed to build custom dir content");
            return;
        }

        if(ADM_nbCustom[pool])
        {
            printf("Found %u custom script(s), adding them (%s)\n", ADM_nbCustom[pool],subDir.c_str());

            for(int i=0; i < ADM_nbCustom[pool]; i++)
            {
                customActions[pool][i] = new QAction(QString::fromUtf8(ADM_GetFileName(customNames[pool][i])), NULL);
                if(pool==JS_CUSTOM)
                {
                    jsMenu->addAction(customActions[pool][i]);
                    connect(customActions[pool][i], SIGNAL(triggered()), this, SLOT(customJs()));
                }
                else
                {
                    pyMenu->addAction(customActions[pool][i]);
                    connect(customActions[pool][i], SIGNAL(triggered()), this, SLOT(customPy()));
                }
            }
        }
        else
            printf("No custom scripts\n");
    }
	printf("Custom menu built\n");
}

/**
\fn     custom
\brief  Invoked when one of the custom script has been called
*/
void MainWindow::customJs(void)
{
	printf("[Custom] Js Invoked\n");
	QObject *ptr=sender();
	if(!ptr) return;
	for(int i=0;i<ADM_nbCustom[JS_CUSTOM];i++)
	{
		if(customActions[JS_CUSTOM][i]==ptr)
		{
			printf("[Custom] %u/%u scripts\n",i,ADM_nbCustom[JS_CUSTOM]);
			HandleAction( (Action)(ACT_CUSTOM_BASE_JS+i));
			return; 
		}
	}
	printf("[Custom] Not found\n");
}
void MainWindow::customPy(void)
{
	printf("[Custom] Python Invoked\n");
	QObject *ptr=sender();
	if(!ptr) return;
	for(int i=0;i<ADM_nbCustom[PY_CUSTOM];i++)
	{
		if(customActions[PY_CUSTOM][i]==ptr)
		{
			printf("[Custom] %u/%u scripts\n",i,ADM_nbCustom[PY_CUSTOM]);
			HandleAction( (Action)(ACT_CUSTOM_BASE_PY+i));
			return; 
		}
	}
	printf("[Custom] Not found\n");
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

//********************************************
//EOF
