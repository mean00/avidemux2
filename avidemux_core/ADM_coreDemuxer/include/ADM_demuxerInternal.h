/***************************************************************************
                          ADM_videoInternal.h  -  description
    begin                : Thu Apr 18 2008
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

#ifndef  ADM_videoInternal_H
#define  ADM_videoInternal_H

#define ADM_DEMUXER_API_VERSION 3
#include "ADM_dynamicLoading.h"
#include "ADM_Video.h"
class ADM_demuxer :public ADM_LibWrapper
{
public:
        int         initialised;
        vidHeader   *(*createdemuxer)();
        void         (*deletedemuxer)(vidHeader *demuxer);
        uint8_t      (*getVersion)(uint32_t *major,uint32_t *minor,uint32_t *patch);
        /// Return true if that demuxer can handle that file, lower value means
        /// less likely to be a good demuxer for that
        uint32_t         (*probe)(uint32_t magic, const char *fileName);
        // Only initialized once
        const char    *name;
        const char    *descriptor;
        uint32_t      apiVersion;
        uint32_t      priority;

        ADM_demuxer(const char *file) : ADM_LibWrapper()
        {
        const char   *(*getDescriptor)();
        uint32_t     (*getApiVersion)();
        uint32_t     (*getPriority)();
        const char  *(*getDemuxerName)();

			initialised = (loadLibrary(file) && getSymbols(8,
				&createdemuxer, "create",
				&deletedemuxer, "destroy",
				&probe,         "probe",

				&getDemuxerName, "getName",
				&getApiVersion,  "getApiVersion",
				&getVersion,     "getVersion",
                &getPriority,    "getPriority",
				&getDescriptor,  "getDescriptor"));
                if(initialised)
                {
                    name=getDemuxerName();
                    priority=getPriority();
                    apiVersion=getApiVersion();
                    descriptor=getDescriptor();
                    printf("[Demuxer]Name :%s ApiVersion :%d Description :%s\n",name,apiVersion,descriptor);
                }else
                {
                    printf("[Demuxer]Symbol loading failed for %s\n",file);
                }
        }
};

#define ADM_DEMUXER_BEGIN( Class,prio,maj,mn,pat,name,desc) \
extern "C" {\
ADM_PLUGIN_EXPORT vidHeader   *create(void){ return new Class; } \
ADM_PLUGIN_EXPORT void         destroy(vidHeader *h){ Class *z=(Class *)h;delete z;} \
ADM_PLUGIN_EXPORT uint8_t      getVersion(uint32_t *major,uint32_t *minor,uint32_t *patch) {*major=maj;*minor=mn;*patch=pat;return 1;} \
ADM_PLUGIN_EXPORT uint32_t     getApiVersion(void) {return ADM_DEMUXER_API_VERSION;} \
ADM_PLUGIN_EXPORT const char  *getName(void) {return name;} \
ADM_PLUGIN_EXPORT const char  *getDescriptor(void) {return desc;} \
ADM_PLUGIN_EXPORT uint32_t     getPriority(void) {return prio;} \
}

#endif
//EOF

