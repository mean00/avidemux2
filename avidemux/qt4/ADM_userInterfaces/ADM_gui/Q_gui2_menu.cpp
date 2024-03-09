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
#include "ADM_QSettings.h"
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
                MenuEntry separatorEntry = {MENU_SEPARATOR, "-", NULL, ACT_DUMMY, NULL, NULL, true};

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

                MenuEntry dummyEntry = {MENU_SUBMENU, itemName, NULL, ACT_DUMMY, NULL, NULL, true};
                it = fileMenu.insert(it, dummyEntry);

                MenuEntry runProjectEntry = {MENU_SUBACTION, QT_TRANSLATE_NOOP("qgui2menu","&Run Project..."), NULL, firstMenuId, NULL, NULL, true};
                it = fileMenu.insert(it + 1, runProjectEntry);

                if ((this->_scriptEngines[engineIndex]->capabilities() & IScriptEngine::Debugger) == IScriptEngine::Debugger)
                {
                    MenuEntry debugEntry = {MENU_SUBACTION, QT_TRANSLATE_NOOP("qgui2menu","&Debug Project..."), NULL, (Action)(firstMenuId + 1), NULL, NULL, true};
                    it = fileMenu.insert(it + 1, debugEntry);
                    i++;
                }

                MenuEntry saveAsProjectEntry = {MENU_SUBACTION, QT_TRANSLATE_NOOP("qgui2menu","Save &As Project..."), NULL, (Action)(firstMenuId + 2), NULL, NULL, true};
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

        MenuEntry entry = {MENU_ACTION, itemName, NULL, (Action)(ACT_SCRIPT_ENGINE_SHELL_FIRST + engineIndex), NULL, NULL, true};

        it = toolMenu.insert(it, entry) + 1;
    }

    if (it == toolMenu.begin()) // no script engines available
        return;

    // sneak in here for now:
    MenuEntry entry = {MENU_ACTION, QT_TRANSLATE_NOOP("qgui2menu","Script Shortcuts"), NULL, (Action)(ACT_ScriptShortcutConfig), NULL, NULL, true};
    toolMenu.insert(it, entry);
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

void MainWindow::addScriptDirToMenu(QMenu* scriptMenu, const QString& dir, const QStringList& fileExts, bool addShortcuts)
{
    QFileInfoList scriptFileList = QDir(dir).entryInfoList(
                                       fileExts, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::DirsFirst | QDir::Name);

    QSettings *qset = NULL;
    if (addShortcuts)
    {
        qset = qtSettingsCreate();
        if (qset)
        {
            qset->beginGroup("GuiScriptShortcuts");
        }
    }
    for (int index = 0; index < scriptFileList.size(); index++)
    {
		QFileInfo fileInfo = scriptFileList.at(index);

        if (fileInfo.isDir())
        {
            if (fileInfo.completeBaseName() != "lib")
            {
                QMenu *dirMenu = new QMenu(fileInfo.completeBaseName(), scriptMenu);

                scriptMenu->addMenu(dirMenu);
                this->addScriptDirToMenu(dirMenu, fileInfo.absoluteFilePath(), fileExts, false);
            }
        }
        else
        {
            FileAction *action = new FileAction(fileInfo.baseName(), fileInfo.absoluteFilePath(), scriptMenu);
            action->setObjectName(fileInfo.baseName());
            if (addShortcuts && qset)
            {
                if (fileInfo.fileName().length() > 0)
                {
                    #define SET_TOOLBAR(X) \
                            if (qset->value("alt" #X "toolbar").toBool()) \
                            { \
                                ui.actionScript ##X ->setVisible(true); \
                                ui.actionScript ##X ->setToolTip(fileInfo.fileName() + "\n[" + action->shortcut().toString() + "]"); \
                                connect(ui.actionScript ##X, &QAction::triggered, this, [=](){action->activate(QAction::Trigger);}); \
                            }

                    if (qset->value("altHome").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_Home);          else
                    if (qset->value("altEnd").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_End);            else
                    if (qset->value("altLeft").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_Left);          else
                    if (qset->value("altUp").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_Up);              else
                    if (qset->value("altRight").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_Right);        else
                    if (qset->value("altDown").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_Down);          else
                    if (qset->value("altPageUp").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_PageUp);      else
                    if (qset->value("altPageDown").toString() == fileInfo.fileName()) action->setShortcut(Qt::ALT | Qt::Key_PageDown);  else
                    if (qset->value("alt0").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_0); SET_TOOLBAR(0);}  else
                    if (qset->value("alt1").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_1); SET_TOOLBAR(1);}  else
                    if (qset->value("alt2").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_2); SET_TOOLBAR(2);}  else
                    if (qset->value("alt3").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_3); SET_TOOLBAR(3);}  else
                    if (qset->value("alt4").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_4); SET_TOOLBAR(4);}  else
                    if (qset->value("alt5").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_5); SET_TOOLBAR(5);}  else
                    if (qset->value("alt6").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_6); SET_TOOLBAR(6);}  else
                    if (qset->value("alt7").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_7); SET_TOOLBAR(7);}  else
                    if (qset->value("alt8").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_8); SET_TOOLBAR(8);}  else
                    if (qset->value("alt9").toString() == fileInfo.fileName()) {action->setShortcut(Qt::ALT | Qt::Key_9); SET_TOOLBAR(9);}  else
                    {}
                    #undef SET_TOOLBAR
                }
            }
            scriptMenu->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(scriptFileActionHandler()));
        }
    }
    if (qset)
    {
        qset->endGroup();
        delete qset;
        qset = NULL;
    }
}

void MainWindow::scriptFileActionHandler()
{
    QString filePath = ((FileAction*)sender())->filePath();
    QString fileExt = QFileInfo(filePath).suffix();
    static int block = 0;
    
    if (block) return;
    block++;

    for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
    {
        if (fileExt == this->_scriptEngines[engineIndex]->defaultFileExtension().c_str())
        {
            printf("Executing %s with %s engine\n", filePath.toUtf8().constData(), this->_scriptEngines[engineIndex]->name().c_str());

            A_parseScript(this->_scriptEngines[engineIndex], filePath.toUtf8().constData());
            A_Resync();
            setMenuItemsEnabledState();
        }
    }
    block--;
}

/**
    \fn buildCustomMenu
*/
void MainWindow::buildCustomMenu(void)
{
    ui.menuCustom->clear();
    ui.menuAuto->clear();

    if (this->_scriptEngines.empty())
    {
        buildActionLists();
        return;
    }

    QStringList fileExts;

    for (int engineIndex = 0; engineIndex < this->_scriptEngines.size(); engineIndex++)
    {
        fileExts << QString("*.") + this->_scriptEngines[engineIndex]->defaultFileExtension().c_str();
    }

    #define X_NUMS X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) X(8) X(9)
    #define X(key) ui.actionScript ## key ->setVisible(false); ui.actionScript ## key ->disconnect();
        X_NUMS
    #undef X
    #undef X_NUMS

    this->addScriptDirToMenu(ui.menuCustom, ADM_getCustomDir().c_str(), fileExts, true);
    this->addScriptDirToMenu(ui.menuAuto, ADM_getAutoDir().c_str(), fileExts, false);
    buildActionLists(); // since we change the menu, the list of stuff needs to be refreshed else it points to deleted items

}

void MainWindow::buildRecentMenu(QMenu *menu, std::vector<std::string>files, QAction **actions)
{
	menu->clear();

	for (int i = 0; i < NB_LAST_FILES; i++)
	{
		if (files[i].size())
		{
			QString act;
			act.setNum(i+1);
			act += ". ";
			act += QString::fromUtf8(files[i].c_str());
			actions[i] = menu->addAction(act);
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
	buildActionLists();
}

void MainWindow::buildRecentProjectMenu(void)
{
	this->buildRecentMenu(this->recentProjects, prefs->get_lastprojectfiles(), this->recentProjectAction);
	buildActionLists();
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

/**
 * \fn addSessionRestoreToRecentMenu
 */
void MainWindow::addSessionRestoreToRecentMenu(vector<MenuEntry>& menu)
{
    for (int i = 0; i < menu.size(); i++)
    {
        if (menu[i].type == MENU_SEPARATOR)
        {
            if (this->_scriptEngines.size())
            {
                vector<MenuEntry>::iterator it = menu.begin() + i;
                // add session restore menu entry
                string restoreSessionEntryName = QT_TRANSLATE_NOOP("qgui2menu","Restore previous session");
                MenuEntry restoreSessionEntry = {MENU_ACTION, restoreSessionEntryName, NULL, ACT_RESTORE_SESSION, NULL, NULL, true};
                it = menu.insert(it + 1, restoreSessionEntry);
                // add separator
                MenuEntry separatorEntry = {MENU_SEPARATOR, "-", NULL, ACT_DUMMY, NULL, NULL, true};
                menu.insert(it + 1, separatorEntry);
            }
            break;
        }
    }
}
//EOF
