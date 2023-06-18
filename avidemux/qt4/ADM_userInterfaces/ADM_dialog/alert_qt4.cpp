/**
    \file alert_qt4
   copyright            : (C) 2007 by mean
    email                : Mean/fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtCore/QCoreApplication>
#include <QDialog>
#include <QMessageBox>
#include <QWidget>
#include <QPushButton>

#include "ADM_inttype.h"
#include "DIA_coreToolkit.h"
#include "DIA_coreUI_internal.h"
#include "ADM_default.h"
#include "prefs.h"
#include "ADM_toolkitQt.h"

static int beQuiet=0;

namespace ADM_Qt4CoreUIToolkit
{

void GUI_Alert(const char *alertstring)
{
    QMessageBox::critical(qtLastRegisteredDialog(), QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Alert")),
    QString::fromUtf8(alertstring), QMessageBox::Ok );
}

void GUI_Info(const char *alertstring)
{
    QMessageBox::information(qtLastRegisteredDialog(), QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Info")),
    QString::fromUtf8(alertstring), QMessageBox::Ok );
}

void GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    QString alertString;

    prefs->get(MESSAGE_LEVEL,&msglvl);

    printf("Info message: \"%s\"\n", primary);

    if(msglvl<level)
    {
        printf("Silent mode, not showing info message\n");
        return;
    }

    if(! secondary_format)
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big>";
    else
    {
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big><br><br>" + QString::fromUtf8(secondary_format);
        alertString.replace("\n", "<br>");
    }

    QMessageBox::information(qtLastRegisteredDialog(), QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Info")),
            alertString, QMessageBox::Ok);
}

void GUI_Error_HIG(const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    QString alertString;

    prefs->get(MESSAGE_LEVEL,&msglvl);

    printf("Error message: \"%s\"\n", primary);

    if(msglvl==ADM_LOG_NONE)
    {
        printf("Silent mode, not showing error message\n");
        return;
    }

    if(! secondary_format)
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big>";
    else
    {
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big><br><br>" + QString::fromUtf8(secondary_format);
        alertString.replace("\n", "<br>");
    }

    QMessageBox::critical(qtLastRegisteredDialog(), QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Info")),
            alertString, QMessageBox::Ok);
}

int GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);
    QString alertString;

    printf("Confirmation: \"%s\"\n", primary);

    if (beQuiet || msglvl==ADM_LOG_NONE)
    {
        printf("Silent mode, confirmation dialog skipped.\n");
        return ADM_IGN;
    }

    if (!secondary_format)
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big>";
    else
    {
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big><br><br>" + QString::fromUtf8(secondary_format);
        alertString.replace("\n", "<br>");
    }

    QMessageBox box(qtLastRegisteredDialog());
    box.setWindowTitle(QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Confirmation")));
    box.setIcon(QMessageBox::Question);
    box.setText(alertString);
    QPushButton *yesButton = box.addButton(QString::fromUtf8(button_confirm), QMessageBox::YesRole);
    box.addButton(QMessageBox::No);
    box.setDefaultButton(yesButton);

    box.exec();

    bool reply = (box.clickedButton() == qobject_cast<QAbstractButton *>(yesButton));

    printf("Confirmation \"%s\": %s\n", primary, reply ? "Yes" : "No");

    return reply ? ADM_OK : ADM_ERR;
}

int GUI_YesNo(const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);
    QString alertString;

    printf("YesNo - \"%s\"\n", primary);

    if (beQuiet || msglvl==ADM_LOG_NONE)
    {
        printf("Silent mode, ignoring YesNo\n");
        return ADM_IGN;
    }

    if (!secondary_format)
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big>";
    else
    {
        alertString = "<big><b>" + QString::fromUtf8(primary) + "</b></big><br><br>" + QString::fromUtf8(secondary_format);
        alertString.replace("\n", "<br>");
    }

    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(qtLastRegisteredDialog(), QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Confirmation")),
                alertString, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    printf("YesNo \"%s\": %s\n", primary, (reply == QMessageBox::Yes) ? "Yes" : "No");

    return (reply == QMessageBox::Yes) ? ADM_OK : ADM_ERR;
}

int GUI_Question(const char *alertstring, bool insuppressible)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);
    QMessageBox::StandardButton reply;
    printf("Question: \"%s\"\n", alertstring);

    if ((beQuiet || msglvl == ADM_LOG_NONE) && !insuppressible)
    {
        printf("Silent mode, question ignored\n");
        return ADM_IGN;
    }

    reply = QMessageBox::question(qtLastRegisteredDialog(), QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Question")),
                QString::fromUtf8(alertstring), QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

    printf("Question \"%s\": %s\n", alertstring, (reply == QMessageBox::Yes) ? "Yes" : "No");

    return (reply == QMessageBox::Yes) ? ADM_OK : ADM_ERR;
}
/**
    \fn GUI_Alternate(char *title,char *choice1,char *choice2)
    \brief Popup a dialog box name title with 2 buttons (choice1/choice2)
    \return 0 if first is selected, 1 if second
*/
int GUI_Alternate(const char *title,const char *choice1,const char *choice2)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);
    printf("Alternate - \"%s\": \"%s\" or \"%s\"\n", title,choice1,choice2);

    if (beQuiet || msglvl==ADM_LOG_NONE)
    {
        printf("Silent mode, choosing \"%s\"\n", choice1);
        return 0;
    }

    QMessageBox box(qtLastRegisteredDialog());

    box.setWindowTitle(QString::fromUtf8(QT_TRANSLATE_NOOP("qtalert","Question?")));
    QPushButton *first = box.addButton(QString::fromUtf8(choice1),QMessageBox::YesRole);
    QPushButton *second = box.addButton(QString::fromUtf8(choice2),QMessageBox::NoRole);
    box.setDefaultButton(first);

    if (title)
        box.setText(QString::fromUtf8(title));
    else
        box.setText(QT_TRANSLATE_NOOP("qtalert","Question"));

    box.setIcon(QMessageBox::Question);

    box.exec();

    if (box.clickedButton() == qobject_cast<QAbstractButton *>(second))
    {
        printf("Alternate: the second option chosen\n");
        return 1;
    }
    printf("Alternate: the first option chosen\n");
    return 0;
}

uint8_t GUI_getDoubleValue(double *valye, float min, float max, const char *title)
{
    return 0;
}

uint8_t GUI_isQuiet(void)
{
    return beQuiet;
}

void GUI_Verbose(void)
{
    beQuiet=0;
}

void GUI_Quiet(void)
{
    beQuiet=1;
}

extern DIA_workingBase *createWorking(const char *title);
extern DIA_processingBase *createProcessing(const char *title, uint64_t totalToProcess);
extern DIA_encodingBase *createEncoding(uint64_t duration);
extern DIA_audioTrackBase *createAudioTrack( PoolOfAudioTracks *pool, ActiveAudioTracks *active );

void getVersion(uint32_t *maj,uint32_t *minor)
{
    *maj=ADM_CORE_TOOLKIT_MAJOR;
    *minor=ADM_CORE_TOOLKIT_MINOR;
}

void UI_purge( void )
{
    QCoreApplication::processEvents ();
}

} // namespace

static CoreToolkitDescriptor Qt4CoreToolkitDescriptor=
{
    &ADM_Qt4CoreUIToolkit::getVersion,
    &ADM_Qt4CoreUIToolkit::GUI_Info_HIG,
    &ADM_Qt4CoreUIToolkit::GUI_Error_HIG,
    &ADM_Qt4CoreUIToolkit::GUI_Confirmation_HIG,

    &ADM_Qt4CoreUIToolkit::GUI_YesNo,
    &ADM_Qt4CoreUIToolkit::GUI_Question,
    &ADM_Qt4CoreUIToolkit::GUI_Alternate,
    &ADM_Qt4CoreUIToolkit::GUI_Verbose,

    &ADM_Qt4CoreUIToolkit::GUI_Quiet,
    &ADM_Qt4CoreUIToolkit::GUI_isQuiet,
    &ADM_Qt4CoreUIToolkit::createWorking,
    &ADM_Qt4CoreUIToolkit::createEncoding,

    &ADM_Qt4CoreUIToolkit::createAudioTrack,
    &ADM_Qt4CoreUIToolkit::UI_purge,
    &ADM_Qt4CoreUIToolkit::createProcessing,
};

void InitCoreToolkit(void )
{
    DIA_toolkitInit(&Qt4CoreToolkitDescriptor);
}
//EOF
