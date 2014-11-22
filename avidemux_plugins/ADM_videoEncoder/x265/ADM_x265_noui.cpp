/***************************************************************************
                          \fn ADM_x265
                          \brief Front end for x265 Mpeg4 asp encoder
                             -------------------
    
    copyright            : (C) 2002/2014 by mean/gruntster
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

#include "ADM_default.h"
#include "ADM_x265.h"
#undef ADM_MINIMAL_UI_INTERFACE // we need the full UI
#include "DIA_factory.h"


bool x265_ui(x265_settings *settings)
{
        return true;
}
// EOF


