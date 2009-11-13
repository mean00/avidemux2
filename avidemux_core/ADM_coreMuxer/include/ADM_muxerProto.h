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
class ADM_muxer;
class CONFcouple;
bool        ADM_mux_configure(int index);
uint32_t    ADM_mx_getNbMuxers(void);
const char *ADM_mx_getDisplayName(uint32_t i);
const char *ADM_mx_getName(uint32_t i);
bool        ADM_mx_getMuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
bool        ADM_mx_getExtraConf(int index,CONFcouple **c);
bool        ADM_mx_setExtraConf(int index,CONFcouple *c);
int         ADM_MuxerIndexFromName(const char *name);
ADM_muxer   *ADM_MuxerSpawn(const char *name);
ADM_muxer   *ADM_MuxerSpawnFromIndex(int index);



#endif
