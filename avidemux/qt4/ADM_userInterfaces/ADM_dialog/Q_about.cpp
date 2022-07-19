/***************************************************************************
                                Q_about.cpp
                                -----------

    begin                : Fri May 5 2008
    copyright            : (C) 2008 by gruntster
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
#include "Q_about.h"
#include "ADM_inttype.h"
#include "ADM_toolkitQt.h"

extern uint8_t DIA_license(void);

Ui_aboutWindow::Ui_aboutWindow(QWidget* parent) : QDialog(parent)
{
    Q_INIT_RESOURCE(about);

    ui.setupUi(this);

    connect(ui.licenseButton, SIGNAL(clicked(bool)), this, SLOT(licenseButton_clicked(bool)));

#define SBUFLEN 128
    char subversion[SBUFLEN] = {0};
    const char *compiler="";
#ifdef WIN32
        #ifdef _MSC_VER
            compiler="-VC++";
        #else
            compiler="-Gcc";
        #endif
#endif
#if defined(ADM_SUBVERSION)
    snprintf(subversion, SBUFLEN, " %s <br><small>(%s-fflibs %s%s)</small>", ADM_VERSION, ADM_SUBVERSION, ADM_FFMPEG_VERSION, compiler);
#else
    snprintf(subversion, SBUFLEN, " %s - Release", ADM_VERSION);
#endif
    QString sv(subversion);
    ui.labelVersion->setTextFormat(Qt::RichText);
    ui.labelVersion->setText(ui.labelVersion->text() + sv);
#if !defined(COPYRIGHT_YEAR)
    #define COPYRIGHT_YEAR 2022
#endif
    subversion[0] = '\0';
    snprintf(subversion, SBUFLEN, "© 2001 - %d  Mean / eumagga0x2a", COPYRIGHT_YEAR); // doesn't need to be translatable
    ui.labelCopy->setText(QString::fromUtf8(subversion));
    ui.labelUrl->setText("<a href=\"http://www.avidemux.org\">http://www.avidemux.org</a>");
}

void Ui_aboutWindow::licenseButton_clicked(bool)
{
    DIA_license();
}

uint8_t DIA_about(void)
{
    Ui_aboutWindow dialog(qtLastRegisteredDialog());
    qtRegisterDialog(&dialog);

    dialog.exec();

    qtUnregisterDialog(&dialog);

    return 1;
}
