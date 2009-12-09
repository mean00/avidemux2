/**
    \file ADM_JSDebug.h
    \brief Debug oriented functions for avidemux JS/JS shell
    \author mean (c) 2009 fixounet@free.fr


*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADMJS_DEBUG_H
#define ADMJS_DEBUG_H
extern "C"
{
void jsPopupError(const char *s);
void jsPopupInfo(const char *s);
};


#endif