

/***************************************************************************
                         DIA_resizeWiz
                             -------------------

                           Ui for hue & sat
    copyright            : (C) 2004/5 by mean
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
#ifndef DIA_RES_WIZ_H
#define DIA_RES_WIZ_H

typedef enum 
{
        RESWIZ_VCD,
        RESWIZ_SVCD,
        RESWIZ_DVD,
        RESWIZ_DVD_HD1,
        RESWIZ_PSP,
        RESWIZ_PSP_FULLRES,
        RESWIZ_IPOD,
        RESWIZ_IPOD640
        
}RESWIZ_FORMAT;

typedef enum 
{
        RESWIZ_AR_1_1,
        RESWIZ_AR_4_3,
        RESWIZ_AR_16_9
}RESWIZ_AR;

uint8_t DIA_resizeWiz(RESWIZ_FORMAT *format, RESWIZ_AR *source, RESWIZ_AR *destination);

#endif
