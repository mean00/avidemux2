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
/**
	\class ADM_tray
*/
class ADM_tray
{
protected:
        void    *sys;
		const void *_parent;
public:
                ADM_tray(const void *parent) {};
     virtual  ~ADM_tray() {};
     virtual  uint8_t setPercent(int percent) {return 0;};
     virtual  uint8_t setStatus(int working)  {return 0;};

};
ADM_tray *DIA_createTray(const void *parent);
#endif
