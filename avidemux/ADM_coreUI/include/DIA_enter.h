/***************************************************************************
                          DIA_enter.h
  Handles univeral dialog
  (C) Mean 2006 fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DIA_ENTER_H
#define DIA_ENTER_H

//  Get a float value
uint8_t  DIA_GetFloatValue(float *value, float min, float max, const char *title, const char *legend);
//  Get an integer value
uint8_t  DIA_GetIntegerValue(int *value, int min, int max, const char *title, const char *legend);
//uint8_t  		GUI_getDoubleValue(double *valye, float min, float max, const char *title);


#endif
