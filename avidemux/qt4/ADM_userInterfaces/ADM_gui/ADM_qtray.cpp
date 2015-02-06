/***************************************************************************
                               ADM_qtray.cpp
                               -------------

    begin                : Tue Sep 2 2008
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

#include <string.h>
#include <stdio.h>
#include <QDialog>
#include <QMenu>

#include "ADM_default.h"
#include "ADM_qtray.h"

#include "tray/film1.xpm"
#include "tray/film3.xpm"
#include "tray/film5.xpm"
#include "tray/film7.xpm"
#include "tray/film9.xpm"
#include "tray/film11.xpm"
#include "tray/film13.xpm"
#include "tray/film15.xpm"
#include "tray/film17.xpm"
#include "tray/film19.xpm"
#include "tray/film21.xpm"
#include "tray/film23.xpm"

extern void UI_deiconify(void);

void ADM_qtray_signalReceiver::restore(void)
{
	UI_deiconify();
	parent->showNormal();
}

void ADM_qtray_signalReceiver::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::DoubleClick)
		restore();
}

ADM_qtray::ADM_qtray(const void* parent) : ADM_tray(parent)
{
	_parent = parent;
	lastIcon = 0;
	maxIcons = 12;

	pixmap = new QIcon[maxIcons];
	pixmap[0] = QIcon(QPixmap(xpm_film1));
	pixmap[1] = QIcon(QPixmap(xpm_film3));
	pixmap[2] = QIcon(QPixmap(xpm_film5));
	pixmap[3] = QIcon(QPixmap(xpm_film7));
	pixmap[4] = QIcon(QPixmap(xpm_film9));
	pixmap[5] = QIcon(QPixmap(xpm_film11));
	pixmap[6] = QIcon(QPixmap(xpm_film13));
	pixmap[7] = QIcon(QPixmap(xpm_film15));
	pixmap[8] = QIcon(QPixmap(xpm_film17));
	pixmap[9] = QIcon(QPixmap(xpm_film19));
	pixmap[10] = QIcon(QPixmap(xpm_film21));
	pixmap[11] = QIcon(QPixmap(xpm_film23));

	signalReceiver = new ADM_qtray_signalReceiver();
	signalReceiver->parent = (QDialog*)parent;
	QSystemTrayIcon* trayIcon = new QSystemTrayIcon(pixmap[0], (QObject*)parent);

	sys = trayIcon;

	trayIcon->setToolTip("Avidemux");

	openAction = new QAction(QT_TR_NOOP("Open Avidemux"), (QObject*)parent);
	QObject::connect(openAction, SIGNAL(triggered()), signalReceiver, SLOT(restore()));
	QObject::connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), signalReceiver, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	trayIconMenu = new QMenu((QWidget*)parent);
	trayIconMenu->addAction(openAction);

	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->show();
}

ADM_qtray::~ADM_qtray()
{
    ADM_info("Deleting tray\n");
    if(sys)
    {
        QSystemTrayIcon *t=(QSystemTrayIcon *)sys;
        t->hide();
        delete t;
        sys=NULL;
    }
    if(signalReceiver)
    {
        delete signalReceiver;
        signalReceiver=NULL;
    }
    if(pixmap)
    {
        delete [] pixmap;
        pixmap=NULL;
    }
}

uint8_t ADM_qtray::setPercent(int percent)
{
	char percentS[40];

	sprintf(percentS, "Avidemux [%d%%]", percent);

	lastIcon++;

	if (lastIcon >= maxIcons)
		lastIcon = 0;

	((QSystemTrayIcon*)sys)->setIcon(pixmap[lastIcon]);
	((QSystemTrayIcon*)sys)->setToolTip(percentS);

	return 1;
}

uint8_t ADM_qtray::setStatus(int working)
{
	return 1;
}
/**
    \fn DIA_createTray
*/
ADM_tray *DIA_createTray(void const*parent)
{
    return new ADM_qtray(parent);
}
