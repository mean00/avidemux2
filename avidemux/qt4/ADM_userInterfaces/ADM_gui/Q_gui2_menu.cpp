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

#include <string>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

#include "config.h"
#include "Q_gui2.h"
#include "FileAction.h"
#include "A_functions.h"

using std::string;

void MainWindow::addScriptEnginesToFileMenu(vector<MenuEntry>& fileMenu)
{
    for (int i = 0; i < fileMenu.size(); i++)
    {
        if (fileMenu[i].type == MENU_SEPARATOR)
        {
            if (this->_scriptEngines.size() > 0)
            {
                MenuEntry separatorEntry = {MENU_SEPARATOR, "-", NULL, ACT_DUMMY, NULL, NULL};

                fileMenu.insert(fileMenu.begin() + i, separatorEntry);
                i++;
            }

            for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
            {
                vector<MenuEntry>::iterator it = fileMenu.begin() + i;
                Action firstMenuId = (Action)(ACT_SCRIPT_ENGINE_FIRST + (engineIndex * 3));
                string itemName;

                if (this->_scriptEngines.size() == 1)
                {
                    itemName = QT_TRANSLATE_NOOP("qgui2menu","Project Script");
                }
                else
                {
                    itemName = this->_scriptEngines[engineIndex]->name() + QT_TRANSLATE_NOOP("qgui2menu"," Project");
                }

                MenuEntry dummyEntry = {MENU_SUBMENU, itemName, NULL, ACT_DUMMY, NULL, NULL};
                it = fileMenu.insert(it, dummyEntry);

                MenuEntry runProjectEntry = {MENU_SUBACTION, QT_TRANSLATE_NOOP("qgui2menu","&Run Project..."), NULL, firstMenuId, NULL, NULL};
                it = fileMenu.insert(it + 1, runProjectEntry);

                if ((this->_scriptEngines[engineIndex]->capabilities() & IScriptEngine::Debugger) == IScriptEngine::Debugger)
                {
                    MenuEntry debugEntry = {MENU_SUBACTION, QT_TRANSLATE_NOOP("qgui2menu","&Debug Project..."), NULL, (Action)(firstMenuId + 1), NULL, NULL};
                    it = fileMenu.insert(it + 1, debugEntry);
                    i++;
                }

                MenuEntry saveAsProjectEntry = {MENU_SUBACTION, QT_TRANSLATE_NOOP("qgui2menu","Save &As Project..."), NULL, (Action)(firstMenuId + 2), NULL, NULL};
                it = fileMenu.insert(it + 1, saveAsProjectEntry);
                i += 3;
            }

            break;
        }
    }
}

void MainWindow::addScriptShellsToToolsMenu(vector<MenuEntry>& toolMenu)
{
    vector<MenuEntry>::iterator it = toolMenu.begin();

    for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
    {
        string itemName;

        if (this->_scriptEngines.size() == 1)
        {
            itemName = QT_TRANSLATE_NOOP("qgui2menu","Scripting Shell");
        }
        else
        {
            itemName = this->_scriptEngines[engineIndex]->name() + QT_TRANSLATE_NOOP("qgui2menu"," Shell");
        }

        MenuEntry entry = {MENU_ACTION, itemName, NULL, (Action)(ACT_SCRIPT_ENGINE_SHELL_FIRST + engineIndex), NULL, NULL};

        it = toolMenu.insert(it, entry) + 1;
    }
}

void MainWindow::addScriptReferencesToHelpMenu()
{
    bool referenceAdded = false;
    QAction *beforeAction = ui.menuHelp->actions()[0];

    for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
    {
        string itemName;

        if (this->_scriptEngines[engineIndex]->referenceUrl().length() > 0)
        {
            if (this->_scriptEngines.size() == 1)
            {
                itemName = QT_TRANSLATE_NOOP("qgui2menu","Scripting Reference");
            }
            else
            {
                itemName = this->_scriptEngines[engineIndex]->name() + QT_TRANSLATE_NOOP("qgui2menu"," Reference");
            }

            FileAction *action = new FileAction(
                itemName.c_str(), (this->_scriptEngines[engineIndex]->name() + "/" +
                                   this->_scriptEngines[engineIndex]->referenceUrl()).c_str(), ui.menuHelp);

            referenceAdded = true;
            ui.menuHelp->insertAction(beforeAction, action);
            connect(action, SIGNAL(triggered()), this, SLOT(scriptReferenceActionHandler()));
        }
    }

    if (referenceAdded)
    {
        ui.menuHelp->insertSeparator(beforeAction);
    }
}

void MainWindow::scriptReferenceActionHandler()
{
#ifdef _WIN32
    QString referenceFile = QCoreApplication::applicationDirPath() + "/help/" + ((FileAction*)sender())->filePath();
#else
    QString referenceFile = ADM_getInstallRelativePath(
                                "share", "avidemux6/help", ((FileAction*)sender())->filePath().toUtf8().constData());
#endif

    QDesktopServices::openUrl(QUrl("file:///" + referenceFile, QUrl::TolerantMode));
}

void MainWindow::addScriptDirToMenu(QMenu* scriptMenu, const QString& dir, const QStringList& fileExts)
{
    QFileInfoList scriptFileList = QDir(dir).entryInfoList(
                                       fileExts, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::DirsFirst | QDir::Name);

    for (int index = 0; index < scriptFileList.size(); index++)
    {
		QFileInfo fileInfo = scriptFileList.at(index);

        if (fileInfo.isDir())
        {
            if (fileInfo.completeBaseName() != "lib")
            {
                QMenu *dirMenu = new QMenu(fileInfo.completeBaseName(), scriptMenu);

                scriptMenu->addMenu(dirMenu);
                this->addScriptDirToMenu(dirMenu, fileInfo.absoluteFilePath(), fileExts);
            }
        }
        else
        {
            FileAction *action = new FileAction(fileInfo.baseName(), fileInfo.absoluteFilePath(), scriptMenu);

            scriptMenu->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(scriptFileActionHandler()));
        }
    }
}

void MainWindow::scriptFileActionHandler()
{
    QString filePath = ((FileAction*)sender())->filePath();
    QString fileExt = QFileInfo(filePath).suffix();

    for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
    {
        if (fileExt == this->_scriptEngines[engineIndex]->defaultFileExtension().c_str())
        {
            printf("Executing %s with %s engine\n", filePath.toUtf8().constData(), this->_scriptEngines[engineIndex]->name().c_str());

            A_parseScript(this->_scriptEngines[engineIndex], filePath.toUtf8().constData());
            A_Resync();
        }
    }
}

/**
    \fn buildCustomMenu
*/
void MainWindow::buildCustomMenu(void)
{
    QStringList fileExts;

    for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
    {
        fileExts << QString("*.") + this->_scriptEngines[engineIndex]->defaultFileExtension().c_str();
    }

    ui.menuCustom->clear();
    ui.menuAuto->clear();

    this->addScriptDirToMenu(ui.menuCustom, ADM_getCustomDir(), fileExts);
    this->addScriptDirToMenu(ui.menuAuto, ADM_getAutoDir(), fileExts);
}

void MainWindow::buildRecentMenu(QMenu *menu, std::vector<std::string>files, QAction **actions)
{
	menu->clear();

	for (int i = 0; i < NB_LAST_FILES; i++)
	{
		if (files[i].size())
		{
			actions[i] = menu->addAction(QString('0' + i) + QString(":") + QString::fromUtf8(files[i].c_str()));
		}
		else
		{
			actions[i] = NULL;
		}
	}
}

/**
    \fn buildRecentMenu
*/
void MainWindow::buildRecentMenu(void)
{
	this->buildRecentMenu(this->recentFiles, prefs->get_lastfiles(), this->recentFileAction);
}

void MainWindow::buildRecentProjectMenu(void)
{
	this->buildRecentMenu(this->recentProjects, prefs->get_lastprojectfiles(), this->recentProjectAction);
}

void MainWindow::searchRecentFiles(QAction *action, QAction **actionList, int firstEventId)
{
	for (int i = 0; i < NB_LAST_FILES; i++)
	{
		QAction *a = actionList[i];

		if (!a) continue;

		if (a == action)
		{
			sendAction((Action)(firstEventId + i));
			return;
		}
	}
}

/**
\fn searchRecentFiles
*/
void MainWindow::searchRecentFiles(QAction * action)
{
	this->searchRecentFiles(action, this->recentFileAction, ACT_RECENT0);
}

void MainWindow::searchRecentProjects(QAction * action)
{
	this->searchRecentFiles(action, this->recentProjectAction, ACT_RECENT_PROJECT0);
}
