/***************************************************************************
    \file  Q_titleBar
    \brief Custom QDockWidget title bar
    \author Matthew White <mehw.is.me@inventati.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Q_titleBar.h"

TitleBar::TitleBar(QString title /* = "" */)
{
    QString tt;

    ui.setupUi(this);

    // Can interpret text mnemonic (&) when a buddy is set (setBuddy).
    ui.titleLabel->setText(title);

    tt = QT_TRANSLATE_NOOP("qgui2","Dock");
    ui.dockButton->setToolTip(tt);
    ui.dockButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMinButton));
    connect(ui.dockButton,SIGNAL(clicked()),this,SLOT(dockParent()));

    tt = QT_TRANSLATE_NOOP("qgui2","Float");
    ui.floatButton->setToolTip(tt);
    ui.floatButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    connect(ui.floatButton,SIGNAL(clicked()),this,SLOT(floatParent()));

    tt = QT_TRANSLATE_NOOP("qgui2","Close");
    ui.closeButton->setToolTip(tt);
    ui.closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(ui.closeButton,SIGNAL(clicked()),this,SLOT(closeParent()));
}

void TitleBar::showEvent(QShowEvent *e)
{
    QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());
    bool floating = dockWidget->isFloating();
    QWidget::showEvent(e);
    ui.dockButton->setVisible(floating);
    ui.floatButton->setVisible(!floating);
    // Interpret text mnemonic (&).
    if(floating)
        ui.titleLabel->setBuddy(ui.dockButton);
    else
        ui.titleLabel->setBuddy(ui.floatButton);
}

void TitleBar::dockParent(void)
{
    QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());
    dockWidget->setFloating(false);
}

void TitleBar::floatParent(void)
{
    QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());
    dockWidget->setFloating(true);
}

void TitleBar::closeParent(void)
{
    QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());
    dockWidget->setVisible(false);
}

TitleBar::~TitleBar()
{
}
