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
#include "ADM_vs.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"
#include <algorithm>

ADM_DEMUXER_BEGIN( vsHeader, 50,
                    1,0,0,
                    "flv",
                    "VapourSynth demuxer plugin (c) Mean 2015"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t         probe(uint32_t magic, const char *fileName)
{
        std::string fname=std::string(fileName);
        if(fname.length()<3) return 0;
        if (fname.substr(fname.length()-3) == std::string(".py")) 
        {
                ADM_info("[Probe] This is .py, might be vapourSynth\n");
                // todo : probe deeper
                return 100;
        }
        return 0;
}
