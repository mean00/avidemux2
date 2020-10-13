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
#include "ADM_vsInternal.h"

#if !defined(__APPLE__) && !defined(_WIN32)
 #include <dlfcn.h>
#endif


vsDynaLoader dynaLoader;
static bool loaded=false;
ADM_DEMUXER_BEGIN( vsHeader, 50,
                    1,0,0,
                    "vs",
                    "VapourSynth demuxer plugin (c) Mean 2015"
                );


/**
    \fn Probe
*/

extern "C"  uint32_t     ADM_PLUGIN_EXPORT    probe(uint32_t magic, const char *fileName)
{
        
        // Check if we have the lib loaded
        if(!loaded)
            dynaLoader.vsInit(DLL_TO_LOAD,PYTHONLIB);
        loaded=true;
        if(!dynaLoader.isOperational())
            return 0;
        std::string fname=std::string(fileName);
        if(fname.length()<4) return 0;
        if(fname.substr(fname.length()-4) == ".vpy")
        {
                ADM_info("This is .vpy, might be VapourSynth\n");
                // todo : probe deeper
                return 100;
        }
        return 0;
}
