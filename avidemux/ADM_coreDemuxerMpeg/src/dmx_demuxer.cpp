/***************************************************************************
                         Base demuxer class
    
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

#include "ADM_default.h"
#include <string.h>
#include <math.h>

#include "dmx_demuxer.h"

dmx_demuxer::dmx_demuxer(void)
{
        _size=0;
        _lastErr=0;
}
dmx_demuxer::~dmx_demuxer(void)
{
}
