/***************************************************************************
    copyright            : (C) 2007 by mean
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
#include "ADM_muxerInternal.h"
#include "fourcc.h"
#include "muxerLmkv.h"
#include "lmkv_muxer_desc.cpp"
static bool dummy(void)
{
    return true;
}
ADM_MUXER_BEGIN( "mkv",muxerLmkv,
                    1,0,0,
                    "Lmkv",    // Internal name
                    "Libmkv muxer plugin (c) Mean 2012",
                    "libmkv Muxer", // DIsplay name
                    dummy,
                    NULL, //template
                    NULL,
                    0
                );


