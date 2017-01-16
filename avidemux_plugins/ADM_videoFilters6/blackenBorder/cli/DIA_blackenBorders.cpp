/***************************************************************************
                          DIA_crop.cpp  -  description
                             -------------------

			    GUI for cropping including autocrop
			    +Revisted the Gtk2 way
			     +Autocrop now in RGB space (more accurate)

    begin                : Fri May 3 2002
    copyright            : (C) 2002/2007 by mean
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
#include <stdint.h>
#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"

#include "../blackenBorder.h"
/**
      \fn     DIA_getCropParams
      \brief  Handle crop dialog
*/
bool DIA_getBlackenParams(	blackenBorder *param,ADM_coreVideoFilter *in)
{
    return false;
}
//____________________________________
// EOF
