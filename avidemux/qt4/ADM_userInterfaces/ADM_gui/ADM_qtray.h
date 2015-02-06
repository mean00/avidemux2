/***************************************************************************
                                 ADM_qtray.h
                                 -----------

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

#ifndef ADM_QTRAY_H
#define ADM_QTRAY_H

#include <QAction>
#include <QIcon>
#include <QSystemTrayIcon>
#include "ADM_default.h"
#include "ADM_tray.h"

class ADM_qtray_signalReceiver : public QObject
{
     Q_OBJECT

public:
	QDialog *parent;

public slots:
	void restore(void);
	void iconActivated(QSystemTrayIcon::ActivationReason reason);
};

class ADM_qtray : public ADM_tray
{
protected:
	QAction *openAction;
	QMenu *trayIconMenu;
	QIcon *pixmap;
	int lastIcon;
	int maxIcons;
	ADM_qtray_signalReceiver *signalReceiver;

public:
	ADM_qtray(const void *parent);
	~ADM_qtray();
	uint8_t setPercent(int percent);
	uint8_t setStatus(int working);
};
#endif
