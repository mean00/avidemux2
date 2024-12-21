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
#include "ADM_nativeAvs.h"
#include "ADM_demuxerInternal.h"
#include "fourcc.h"

ADM_DEMUXER_BEGIN(nativeAvsHeader, 50,
                    1,0,0,
                    "navs",
                    "native avs demuxer plugin (c) Mean 2018"
                );

/**
    \fn Probe
*/

extern "C"  uint32_t     ADM_PLUGIN_EXPORT    probe(uint32_t magic, const char *fileName)
{
	std::string st = std::string(fileName);
	std::string extension("avs");

	if (st.length() >= extension.length()) 
	{
        if (!st.compare(st.length() - extension.length(), extension.length(), extension))
			return 100;
	}
    ADM_warning (" [avsProxy] Cannot open that\n");
    return 0;
}
