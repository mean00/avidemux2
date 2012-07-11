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
				string itemName = "Project";

				if (this->_scriptEngines.size() > 1)
				{
					itemName = this->_scriptEngines[engineIndex]->name() + " " + itemName;
				}

				MenuEntry dummyEntry = {MENU_SUBMENU, itemName, NULL, ACT_DUMMY, NULL, NULL};
				it = fileMenu.insert(it, dummyEntry);

				MenuEntry runProjectEntry = {MENU_SUBACTION, "&Run Project...", NULL, firstMenuId, NULL, NULL};
				it = fileMenu.insert(it + 1, runProjectEntry);

				if ((this->_scriptEngines[engineIndex]->capabilities() & IScriptEngine::Debugger) == IScriptEngine::Debugger)
				{
					MenuEntry debugEntry = {MENU_SUBACTION, "&Debug Project...", NULL, (Action)(firstMenuId + 1), NULL, NULL};
					it = fileMenu.insert(it + 1, debugEntry);
					i++;
				}

				MenuEntry saveAsProjectEntry = {MENU_SUBACTION, "Save &As Project...", NULL, (Action)(firstMenuId + 2), NULL, NULL};
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
		string itemName = "Shell";

		if (this->_scriptEngines.size() > 1)
		{
			itemName = this->_scriptEngines[engineIndex]->name() + " " + itemName;
		}

		MenuEntry entry = {MENU_ACTION, itemName, NULL, (Action)(ACT_SCRIPT_ENGINE_SHELL_FIRST + engineIndex), NULL, NULL};

		it = toolMenu.insert(it, entry) + 1;
	}
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
		if (fileExt == _scriptEngines[engineIndex]->defaultFileExtension().c_str())
		{
			printf("Executing %s with %s engine\n", filePath.toUtf8().constData(), _scriptEngines[engineIndex]->name().c_str());

			A_parseScript(_scriptEngines[engineIndex], filePath.toUtf8().constData());
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
		fileExts << QString("*.") + _scriptEngines[engineIndex]->defaultFileExtension().c_str();
	}

	ui.menuCustom->clear();
	ui.menuAuto->clear();

	const char *customDir = ADM_getCustomDir();
	const char *autoDir = ADM_getAutoDir();

	this->addScriptDirToMenu(ui.menuCustom, customDir, fileExts);
	this->addScriptDirToMenu(ui.menuAuto, autoDir, fileExts);

	delete [] customDir;
	delete [] autoDir;
}
