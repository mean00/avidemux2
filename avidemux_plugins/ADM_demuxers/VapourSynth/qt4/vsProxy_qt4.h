/**
         \file Simple proxy for vsProxy
         \brief external job control
         \author mean fixounet@free.fr (c) 2015
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include <QDialog>
#include "ui_vs.h"
/**
    \class vsWindow
*/
class vsWindow   : public QDialog 
{
Q_OBJECT

public:
                        vsWindow(QWidget *parent);	                
    virtual             ~vsWindow();
protected:    
    uint32_t            localPort;
    Ui_VapourSynthProxy dialog;

};


