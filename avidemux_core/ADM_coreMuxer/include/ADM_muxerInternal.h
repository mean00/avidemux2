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

#ifndef  ADM_muxerInternal_H
#define  ADM_muxerInternal_H

#define ADM_MUXER_API_VERSION 9
#include <stddef.h>
#include "ADM_dynamicLoading.h"
#include "ADM_muxer.h"
#include "ADM_paramList.h"
class ADM_dynMuxer :public ADM_LibWrapper
{
public:
        int         initialised;
        ADM_muxer    *(*createmuxer)();
        void         (*deletemuxer)(ADM_muxer *muxer);
        uint8_t      (*getVersion)(uint32_t *major,uint32_t *minor,uint32_t *patch);
        const char    *name;
        const char    *displayName;
        const char    *descriptor;
        const char    *defaultExtension;
        uint32_t      apiVersion;
        bool  (*configure)(void);
        bool  (*getConfiguration)(CONFcouple **conf);
        bool  (*resetConfiguration)();
        bool  (*setConfiguration)(CONFcouple *conf);

        ADM_dynMuxer(const char *file) : ADM_LibWrapper()
        {
        const char   *(*getDescriptor)();
        uint32_t     (*getApiVersion)();
        const char  *(*getMuxerName)();
        const char  *(*getDisplayName)();
        const char  *(*getDefaultExtension)();



			initialised = (loadLibrary(file) && getSymbols(8+4,
				&createmuxer, "create",
				&deletemuxer, "destroy",
				&getMuxerName, "getName",
                &getDisplayName, "getDisplayName",
				&getApiVersion,  "getApiVersion",
				&getVersion,     "getVersion",
				&getDescriptor,  "getDescriptor",
                &configure,"configure",
                &setConfiguration,"setConfiguration",
                &getConfiguration,"getConfiguration",
				&resetConfiguration,"resetConfiguration",
                &getDefaultExtension,"getDefaultExtension"
                ));
                if(initialised)
                {
                    name=getMuxerName();
                    displayName=getDisplayName();
                    apiVersion=getApiVersion();
                    descriptor=getDescriptor();
                    defaultExtension=getDefaultExtension();
                    printf("[Muxer]Name :%s ApiVersion :%d Description :%s\n",name,apiVersion,descriptor);
                }else
                {
                    printf("[Muxer]Symbol loading failed for %s\n",file);
                }
        }
};

#define ADM_MUXER_BEGIN( Ext,Class,maj,mn,pat,name,desc,displayName,configureFunc,confTemplate,confVar,confSize) \
extern "C" {\
	static void *defaultConfig = NULL; \
\
	static void snapshotDefaultConfiguration() \
	{ \
		if (confVar != NULL && defaultConfig == NULL) \
		{ \
			defaultConfig = ADM_alloc(confSize); \
			memcpy(defaultConfig, confVar, confSize); \
		} \
	} \
\
ADM_muxer   *create(void){ return new Class; } \
void         destroy(ADM_muxer *h){ Class *z=(Class *)h;delete z;} \
uint8_t      getVersion(uint32_t *major,uint32_t *minor,uint32_t *patch) {*major=maj;*minor=mn;*patch=pat;return 1;} \
uint32_t     getApiVersion(void) {return ADM_MUXER_API_VERSION;} \
const char  *getName(void) {return name;} \
const char  *getDescriptor(void) {return desc;} \
const char  *getDisplayName(void) { return displayName;} \
const char  *getDefaultExtension(void) { return Ext;} \
bool        getConfiguration(CONFcouple **conf) \
{\
         if(confTemplate==NULL) {*conf=NULL;return true;} \
         return ADM_paramSave(conf,confTemplate,confVar); \
}\
bool        setConfiguration(CONFcouple *conf)\
{\
	snapshotDefaultConfiguration(); \
                  return ADM_paramLoad(conf,confTemplate,confVar); \
} \
bool        resetConfiguration() \
{ \
	snapshotDefaultConfiguration(); \
	if (defaultConfig != NULL) memcpy(confVar, defaultConfig, confSize); \
	return true; \
} \
 bool  configure(void) \
{ \
	snapshotDefaultConfiguration(); \
 if(configureFunc==NULL) return true;\
 return configureFunc();}\
}

#endif
//EOF

