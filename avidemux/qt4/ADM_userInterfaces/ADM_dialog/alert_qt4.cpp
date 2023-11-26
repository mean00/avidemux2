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
#include "GUI_ui.h"

static int beQuiet=0;

static QString convertAccels(const char *in)
{
    QString out = QString::fromUtf8(in);
    out.replace("&", "&&");
    out.replace("_", "&");
    return out;
}

static void alertCommon(enum QMessageBox::Icon icon, const char *title, const char *alert, const char *desc)
{
    QMessageBox box(qtLastRegisteredDialog());
    box.setWindowTitle(QString::fromUtf8(title));
    box.setIcon(icon);
    box.setTextInteractionFlags(Qt::TextSelectableByMouse);

    QString alertString = QString::fromUtf8(alert);
    alertString.replace("\n", "<br>");
    if(desc)
        alertString = "<b>" + alertString + "</b>";

#ifndef __APPLE__
    alertString += "<br><br>";
    if(desc)
        alertString += QString::fromUtf8(desc);

    box.setText(alertString);
#else
    box.setText(alertString);
    if(desc)
        box.setInformativeText(QString::fromUtf8(desc));
#endif
    box.exec();
}

static bool questionCommon(const char *title, const char *question, const char *desc,
    const char *confirm, enum QMessageBox::StandardButton alternative)
{
    QMessageBox box(qtLastRegisteredDialog());
    box.setWindowTitle(QString::fromUtf8(title));
    box.setIcon(QMessageBox::Question);
    box.setTextFormat(Qt::RichText);
    box.setTextInteractionFlags(Qt::TextSelectableByMouse);

    QString alertString = QString::fromUtf8(question);
    alertString.replace("\n", "<br>");
    if(desc)
        alertString = "<b>" + alertString + "</b>";

#ifndef __APPLE__
    alertString += "<br><br>";
    if(desc)
        alertString += QString::fromUtf8(desc);

    box.setText(alertString);
#else
    box.setText(alertString);
    if(desc)
        box.setInformativeText(QString::fromUtf8(desc));
#endif

    QPushButton *yesButton = confirm ?
        box.addButton(convertAccels(confirm), QMessageBox::YesRole) :
        box.addButton(QMessageBox::Yes);
    box.addButton(alternative);
    box.setDefaultButton(yesButton);

    box.exec();

    return box.clickedButton() == qobject_cast<QAbstractButton *>(yesButton);
}

/**********************************************/

namespace ADM_Qt4CoreUIToolkit
{
/**
    \fn GUI_Info_HIG
*/
void GUI_Info_HIG(const ADM_LOG_LEVEL level,const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);

    if(secondary_format)
        printf("Info message: \"%s\" \"%s\"\n", primary, secondary_format);
    else
        printf("Info message: \"%s\"\n", primary);

    if(msglvl<level)
    {
        printf("Silent mode, not showing info message\n");
        return;
    }

    alertCommon(QMessageBox::Information,
        QT_TRANSLATE_NOOP("qtalert","Info"),
        primary, secondary_format);
}
/**
    \fn GUI_Error_HIG
*/
void GUI_Error_HIG(const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);

    if(secondary_format)
        printf("Error message: \"%s\" \"%s\"\n", primary, secondary_format);
    else
        printf("Error message: \"%s\"\n", primary);

    if(msglvl==ADM_LOG_NONE)
    {
        printf("Silent mode, not showing error message\n");
        return;
    }

    alertCommon(QMessageBox::Critical,
        QT_TRANSLATE_NOOP("qtalert","Error"),
        primary, secondary_format);
}
/**
    \fn GUI_Confirmation_HIG
*/
int GUI_Confirmation_HIG(const char *button_confirm, const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);

    if (beQuiet || msglvl==ADM_LOG_NONE)
    {
        if(secondary_format)
            printf("Confirmation: \"%s\" \"%s\"\n", primary, secondary_format);
        else
            printf("Confirmation: \"%s\"\n", primary);
        printf("Silent mode, confirmation dialog skipped.\n");
        return ADM_IGN;
    }

    bool reply = questionCommon(QT_TRANSLATE_NOOP("qtalert","Confirmation"),
        primary, secondary_format,
        button_confirm, QMessageBox::No);

    if(secondary_format)
        printf("Confirmation \"%s\" \"%s\" : %s\n", primary, secondary_format, reply ? "Yes" : "No");
    else
        printf("Confirmation \"%s\" : %s\n", primary, reply ? "Yes" : "No");

    return reply ? ADM_OK : ADM_ERR;
}
/**
    \fn GUI_YesNo
*/
int GUI_YesNo(const char *primary, const char *secondary_format)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);

    if (beQuiet || msglvl==ADM_LOG_NONE)
    {
        if(secondary_format)
            printf("YesNo: \"%s\" \"%s\"\n", primary, secondary_format);
        else
            printf("YesNo: \"%s\"\n", primary);
        printf("Silent mode, ignoring YesNo\n");
        return ADM_IGN;
    }

    bool reply = questionCommon(QT_TRANSLATE_NOOP("qtalert","Confirmation"),
        primary, secondary_format, NULL, QMessageBox::No);

    if(secondary_format)
        printf("YesNo \"%s\" \"%s\" : %s\n", primary, secondary_format, reply ? "Yes" : "No");
    else
        printf("YesNo \"%s\" : %s\n", primary, reply ? "Yes" : "No");

    return reply ? ADM_OK : ADM_ERR;
}
/**
    \fn GUI_Question
*/
int GUI_Question(const char *alertstring, bool insuppressible)
{
    uint32_t msglvl=2;
    prefs->get(MESSAGE_LEVEL,&msglvl);

    if ((beQuiet || msglvl == ADM_LOG_NONE) && !insuppressible)
    {
        printf("Question: \"%s\"\n", alertstring);
        printf("Silent mode, question ignored\n");
        return ADM_IGN;
    }

    bool reply = questionCommon(QT_TRANSLATE_NOOP("qtalert","Question"),
        alertstring, NULL, NULL, QMessageBox::Cancel);

    printf("Question \"%s\": %s\n", alertstring, reply ? "Yes" : "No");

    return reply ? ADM_OK : ADM_ERR;
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
    QPushButton *first = box.addButton(convertAccels(choice1), QMessageBox::YesRole);
    QPushButton *second = box.addButton(convertAccels(choice2), QMessageBox::NoRole);
    box.setDefaultButton(first);

    if (title)
        box.setText(QString::fromUtf8(title));
    else
        box.setText(QT_TRANSLATE_NOOP("qtalert","Question"));

    box.setIcon(QMessageBox::Question);
    box.setTextInteractionFlags(Qt::TextSelectableByMouse);

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

void GUI_tweaks(const char * op, const char * paramS, int paramN)
{
    UI_tweaks(op, paramS, paramN);
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
    &ADM_Qt4CoreUIToolkit::GUI_tweaks
};

void InitCoreToolkit(void )
{
    DIA_toolkitInit(&Qt4CoreToolkitDescriptor);
}
//EOF
