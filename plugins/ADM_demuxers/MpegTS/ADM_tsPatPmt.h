/** *************************************************************************
    \file ADM_tsPatPmt.cpp
    \brief Analyze pat & pmt
    copyright            : (C) 2007 by mean
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
#ifndef ADM_TS_PAT_PMT_H
#define ADM_TS_PAT_PMT_H
/**
    \fn scanForPrograms
*/

bool scanForPrograms(const char *file);

/**
    \fn scanPmt
*/
bool scanPmt(tsPacket *t,uint32_t pid);
#endif
//EOF
