/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "Q_scriptShortcutConfig.h"

#include "ADM_default.h"
#include "ADM_toolkitQt.h"
#include "ADM_QSettings.h"
#include "ADM_script2/include/ADM_script.h"
#include <string>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>

scriptShortcutConfigDialog::scriptShortcutConfigDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    
    QSettings *qset = NULL;
    qset = qtSettingsCreate();
    if (qset)
    {
        qset->beginGroup("GuiScriptShortcuts");
    #define X(key) ui.lineEditAlt ## key ->setText(qset->value("alt" #key).toString());
        SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO
    #undef X
    #define X(key) ui.checkBoxAlt ## key ->setChecked(qset->value("alt" #key "toolbar").toBool());
        SCRIPT_NUMERIC_SHORTCUT_CONFIG_LIST_XMACRO
    #undef X

        qset->endGroup();
        delete qset;
        qset = NULL;
    }
    
    QString dir = QString(ADM_getCustomDir().c_str());
    QStringList fileExts;
    QStringList customScriptFiles;
    customScriptFiles << QString(QT_TRANSLATE_NOOP("qscriptshortcutconfig","clear"));
    for(int i=0; i < getScriptEngines().size(); i++)
    {
        IScriptEngine *tempEngine = getScriptEngines()[i];
        std::string dext = tempEngine->defaultFileExtension();
        fileExts << QString("*.") + dext.c_str();
    }
    QFileInfoList scriptFileList = QDir(dir).entryInfoList(fileExts, QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::DirsFirst | QDir::Name);
    for (int index = 0; index < scriptFileList.size(); index++)
    {
        QFileInfo fileInfo = scriptFileList.at(index);
        if (!fileInfo.isDir())
        {
            customScriptFiles << fileInfo.fileName();
        }
    }
    
#define X(key) ui.comboBoxAlt ## key -> addItems(customScriptFiles);
    SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO
#undef X

    QKeySequence tmpKS;
#define X(key) tmpKS = QKeySequence(Qt::ALT | Qt::Key_ ## key); ui.labelAlt ## key ->setText(tmpKS.toString());
    SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO
#undef X

#define X(key) connect(ui.comboBoxAlt ## key,  SIGNAL(activated(int)), this, SLOT(comboBoxAlt ## key ## _activated(int)));
    SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO
#undef X

}

scriptShortcutConfigDialog::~scriptShortcutConfigDialog()
{
}

void scriptShortcutConfigDialog::showEvent(QShowEvent *event)
{
    this->adjustSize();
}

#define X(key) void scriptShortcutConfigDialog::comboBoxAlt ## key ## _activated(int index) { \
        if (index == 0) ui.lineEditAlt ## key ->clear(); else \
        ui.lineEditAlt ## key ->setText(ui.comboBoxAlt ## key ->itemText(index)); \
        }
SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO
#undef X


void DIA_ScriptShortcutConfig()
{
    scriptShortcutConfigDialog dialog(qtLastRegisteredDialog());
    qtRegisterDialog(&dialog);

    if (dialog.exec() == QDialog::Accepted)
    {
        QSettings *qset = NULL;
        qset = qtSettingsCreate();
        if (qset)
        {
            qset->beginGroup("GuiScriptShortcuts");
        #define X(key) qset->setValue("alt" #key, dialog.ui.lineEditAlt ## key ->text());
            SCRIPT_SHORTCUT_CONFIG_LIST_XMACRO
        #undef X
        #define X(key) qset->setValue("alt" #key "toolbar", dialog.ui.checkBoxAlt ## key ->isChecked());
            SCRIPT_NUMERIC_SHORTCUT_CONFIG_LIST_XMACRO
        #undef X
            qset->endGroup();
            delete qset;
            qset = NULL;
        }
    }

    qtUnregisterDialog(&dialog);

}
