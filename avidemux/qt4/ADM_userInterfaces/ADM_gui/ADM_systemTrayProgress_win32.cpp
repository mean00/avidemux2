/***************************************************************************
    copyright            : (C) 2015 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

#ifdef _MSC_VER
#define __STDC_LIMIT_MACROS
#endif

#ifdef QT_HAS_WINEXTRA
#include <QtWinExtras/QtWinExtras>
#endif
#include <QMainWindow>
#include <QStyle>

#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_systemTrayProgress.h"

/**
 */
class winTaskBarProgress : public admUITaskBarProgress
{
  public:
    winTaskBarProgress()
    {
#ifdef QT_HAS_WINEXTRA
        button = new QWinTaskbarButton();

        progress = button->progress();
#endif
    }
    virtual ~winTaskBarProgress()
    {
#ifdef QT_HAS_WINEXTRA

        progress = NULL;
        delete button;
        button = NULL;
#endif
    }
    virtual bool enable()
    {
#ifdef QT_HAS_WINEXTRA
        progress->show();
        progress->setVisible(true);
#endif
        return true;
    }
    virtual bool disable()
    {
#ifdef QT_HAS_WINEXTRA

        progress->hide();
        progress->setVisible(false);
#endif
        return true;
    }
    virtual bool setProgress(int percent)
    {
#ifdef QT_HAS_WINEXTRA

        progress->setValue(percent);
#endif
        return true;
    }
    virtual bool setParent(void *qwin)
    {
        QMainWindow *win = (QMainWindow *)qwin;
#ifdef QT_HAS_WINEXTRA
        button->setWindow(win->windowHandle());
#endif
        return true;
    }
#ifdef QT_HAS_WINEXTRA
    QWinTaskbarButton *button;

    QWinTaskbarProgress *progress;
#endif
};

/**
 */
admUITaskBarProgress *createADMTaskBarProgress()
{

    return new winTaskBarProgress();
}
// EOF
