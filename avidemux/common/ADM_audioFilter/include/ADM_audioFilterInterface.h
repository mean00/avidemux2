
/**************************************************************************
                          audioeng_buildfilters.h  -  description
                             -------------------
    begin                : Mon Dec 2 2002
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
#ifndef ADM_audioFilterInterface_H
#define ADM_audioFilterInterface_H

#include "ADM_audiodef.h"
#include "audiofilter_normalize_param.h"
#include "ADM_confCouple.h"
/* Encoder part */
uint8_t         audioCodecSetByIndex(int dex,int i);
void            audioCodecConfigure( int dex );
void            audioCodecConfigureCodecIndex( int dex,CONFcouple **conf  );

#endif
