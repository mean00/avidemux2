/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_VIDFADE_PARAM_H
#define ADM_VIDFADE_PARAM_H

typedef struct VIDFADE_PARAM
{
  uint32_t startFade;
  uint32_t endFade;
  uint32_t inOut; //0 Out 1 In
  uint32_t toBlack; // =1 else fade to endFrame
}VIDFADE_PARAM;



#endif
