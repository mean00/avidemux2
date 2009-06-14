/***************************************************************************
                          adm_encdivx.cpp  -  description
                             -------------------
    begin                : Sun Jul 14 2002
    copyright            : (C) 2002 by mean
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

#ifndef ADM_TRAY_H
#define ADM_TRAY_H

class ADM_tray
{
protected:
        void    *sys;
		void *_parent;
public:
                ADM_tray(void *parent);
                ~ADM_tray();
        uint8_t setPercent(int percent);
        uint8_t setStatus(int working);

};

#endif
