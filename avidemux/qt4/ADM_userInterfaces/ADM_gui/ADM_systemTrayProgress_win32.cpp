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
#include <QtWinExtras/QtWinExtras>
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
    winTaskBarProgress(QMainWindow *w)
    {
          button = new QWinTaskbarButton(w);
          progress = button->progress();
          button->setOverlayIcon(w->style()->standardIcon(QStyle::SP_MediaPlay));
    }
    virtual ~winTaskBarProgress()
    {
        progress=NULL;
        delete button;
        button=NULL;
    }    
    virtual bool enable() 
    {
        progress->show();
        progress->setVisible(true);
        return true;
    }
    virtual bool disable() 
    {
        progress->hide();
        progress->setVisible(false);
        return true;
    }
    virtual bool setProgress(int percent) 
    {
        progress->setValue(percent);
    } 
    QWinTaskbarButton *button;
    QWinTaskbarProgress *progress;
};

/**
 */
admUITaskBarProgress *createADMTaskBarProgress(void *parent)
{
        QMainWindow *win=( QMainWindow *)parent;
        return new winTaskBarProgress(win);
}
// EOF
