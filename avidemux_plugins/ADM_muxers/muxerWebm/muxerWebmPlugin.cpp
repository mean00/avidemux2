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
#include "muxerWebm.h"
#include "webm_muxer_desc.cpp"
#include "fourcc.h"
 bool WebmConfigure(void);

ADM_MUXER_BEGIN( "webm",muxerWebm,
                    1,0,0,
                    "Webm",    // Internal name
                    "Webm muxer plugin (c) Mean 2009",
                    "Webm Muxer", // DIsplay name
                    WebmConfigure,
                    Webm_muxer_param, //template
                    &WebmMuxerConfig, //config
                    sizeof(Webm_muxer)
                );

