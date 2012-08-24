/***************************************************************************
    \file   ADM_coreDemuxer.h
    \brief  Common part for demuxer
    \author (C) 2010 by mean  : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_CORE_DEMUXER_H 
#define ADM_CORE_DEMUXER_H

#include "ADM_coreDemuxer6_export.h"
#include "ADM_Video.h"

ADM_COREDEMUXER6_EXPORT bool ADM_dm_cleanup(void);
ADM_COREDEMUXER6_EXPORT vidHeader *ADM_demuxerSpawn(uint32_t magic,const char *name);
ADM_COREDEMUXER6_EXPORT uint8_t ADM_dm_loadPlugins(const char *path);
ADM_COREDEMUXER6_EXPORT uint32_t ADM_dm_getNbDemuxers(void);
ADM_COREDEMUXER6_EXPORT bool     ADM_dm_getDemuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);

#endif
