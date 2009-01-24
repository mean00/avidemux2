/** *************************************************************************
                          \file ADM_videoFilter_iface.h
                             -------------------
    begin                : Tue Mar 19 2002
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
 
#ifndef ADM_VIDEOFILTER_IFACE
#define ADM_VIDEOFILTER_IFACE
 typedef enum 
 {
 	APM_NUM,
 	APM_HEXNUM,
 	APM_STRING,
 	APM_QUOTED,
 	APM_FLOAT,
 	APM_BOOL,
 	APM_LAST

 }APM_TYPE;
 
 typedef union
 {
 	int 	integer;
 	float	real;
 	char	*string;

 }ArgI;
 
 typedef struct 
 {
 	APM_TYPE type;
 	ArgI	 arg;

 }Arg;
 
 
#define MAXPARAM 40
 
#endif

