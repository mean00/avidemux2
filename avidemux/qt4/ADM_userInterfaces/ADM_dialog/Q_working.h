/***************************************************************************
    copyright            : (C) 2001 by mean
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
#include "ui_working.h"
/**
 * \class workWindow
 */
class workWindow : public QDialog
{
    Q_OBJECT
    
public:
                        workWindow(QWidget *parent);
    Ui_workingDialog    *ui;
    bool                active;
public slots:
    void                reject(void);
protected:
    void                closeEvent(QCloseEvent *event)
    {
        reject();
    }
};

