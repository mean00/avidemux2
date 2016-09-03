/***************************************************************************
                           Fly-Ui for hue & sat

    copyright            : (C) 2004/5/7 by mean
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
#include "ADM_image.h"
#include "DIA_flyDialog.h"


#include "ADM_vidEq2.h"

#include "DIA_flyEq2.h"

/**
 * 		\fn DIA_getEQ2Param
 * 		\brief flyDialogGtk handling the mplayer EQ2 user Interface dialog.
 */
uint8_t DIA_getEQ2Param(eq2 *param, ADM_coreVideoFilter *in)
{
    return false;
}
uint8_t    flyEq2::download(void) {return 1;}
uint8_t    flyEq2::upload(void) {return 1;}

