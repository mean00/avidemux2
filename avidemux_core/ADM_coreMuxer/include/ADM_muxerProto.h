/***************************************************************************
    \file ADM_muxerProto.h
    \brief Prototypes for muxers plugins functions
    \author Mean (C) 2009, fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_MUXER_PROTO_H
#define ADM_MUXER_PROTO_H

#include "ADM_coreMuxer6_export.h"
#include "BVector.h"
#include "ADM_muxerInternal.h"

class ADM_muxer;
class CONFcouple;

extern ADM_COREMUXER6_EXPORT BVector <ADM_dynMuxer *> ListOfMuxers;

ADM_COREMUXER6_EXPORT void        ADM_lavFormatInit(void);
ADM_COREMUXER6_EXPORT bool        ADM_mux_configure(int index);
ADM_COREMUXER6_EXPORT bool        ADM_mx_cleanup(void);
ADM_COREMUXER6_EXPORT uint32_t    ADM_mx_getNbMuxers(void);
ADM_COREMUXER6_EXPORT const char *ADM_mx_getDisplayName(uint32_t i);
const char *ADM_mx_getName(uint32_t i);
ADM_COREMUXER6_EXPORT bool        ADM_mx_getMuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
bool        ADM_mx_getExtraConf(int index,CONFcouple **c);
ADM_COREMUXER6_EXPORT bool        ADM_mx_setExtraConf(int index,CONFcouple *c);
ADM_COREMUXER6_EXPORT uint8_t     ADM_mx_loadPlugins(const char *path);
ADM_COREMUXER6_EXPORT int         ADM_MuxerIndexFromName(const char *name);
ADM_muxer   *ADM_MuxerSpawn(const char *name);
ADM_COREMUXER6_EXPORT ADM_muxer   *ADM_MuxerSpawnFromIndex(int index);
ADM_COREMUXER6_EXPORT const char  *ADM_MuxerGetDefaultExtension(int i);


#endif
