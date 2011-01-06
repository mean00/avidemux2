/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GAIN_PARAM_H
#define GAIN_PARAM_H

typedef enum 
{
  ADM_NO_GAIN,
  ADM_GAIN_AUTOMATIC,
  ADM_GAIN_MANUAL,
  ADM_GAIN_MAX // dont use!
  
}ADM_GAINMode;

typedef struct GAINparam
{
  ADM_GAINMode mode;
  int32_t gain10;
}GAINparam;

#endif
