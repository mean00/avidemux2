/***************************************************************************
    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr

      See lavformat/flv[dec/env].c for detail
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
#include "ADM_muxerInternal.h"
#include "ADM_dummy.h"

#include "fourcc.h"

extern "C" bool confDummy(void ){return true;}

ADM_MUXER_BEGIN( "dummy",muxerDummy,
                    1,0,0,
                    "dummy",    // Internal name
                    "dummy2 muxer plugin (c) Mean 2008",
                    "Dummy Muxer", // Display name
                    confDummy, //conf func
                    NULL, // template
                    NULL,  // conf data
                    0
                );

