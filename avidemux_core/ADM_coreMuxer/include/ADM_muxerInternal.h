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

#ifndef ADM_muxerInternal_H
#define ADM_muxerInternal_H

#define ADM_MUXER_API_VERSION 9
#include "ADM_dynamicLoading.h"
#include "ADM_muxer.h"
#include "ADM_paramList.h"
#include <stddef.h>
class ADM_dynMuxer : public ADM_LibWrapper
{
  public:
    int initialised;
    ADM_muxer *(*createmuxer)();
    void (*deletemuxer)(ADM_muxer *muxer);
    uint8_t (*getVersion)(uint32_t *major, uint32_t *minor, uint32_t *patch);
    const char *name;
    const char *displayName;
    const char *descriptor;
    const char *defaultExtension;
    uint32_t apiVersion;
    bool (*configure)(void);
    bool (*getConfiguration)(CONFcouple **conf);
    bool (*resetConfiguration)();
    bool (*setConfiguration)(CONFcouple *conf);
    void (*clearDefaultConfig)(void);
    const char *(*getDefaultExtension)(void);

    ADM_dynMuxer(const char *file) : ADM_LibWrapper()
    {
        const char *(*getDescriptor)();
        uint32_t (*getApiVersion)();
        const char *(*getMuxerName)();
        const char *(*getDisplayName)();

        initialised =
            (loadLibrary(file) &&
             getSymbols(8 + 5, &createmuxer, "create", &deletemuxer, "destroy", &getMuxerName, "getName",
                        &getDisplayName, "getDisplayName", &getApiVersion, "getApiVersion", &getVersion, "getVersion",
                        &getDescriptor, "getDescriptor", &configure, "configure", &setConfiguration, "setConfiguration",
                        &getConfiguration, "getConfiguration", &resetConfiguration, "resetConfiguration",
                        &getDefaultExtension, "getDefaultExtension", &clearDefaultConfig, "clearDefaultConfig"));
        if (initialised)
        {
            name = getMuxerName();
            displayName = getDisplayName();
            apiVersion = getApiVersion();
            descriptor = getDescriptor();
            defaultExtension = getDefaultExtension();
            ADM_info("[Muxer]Name :%s ApiVersion :%d Description :%s\n", name, apiVersion, descriptor);
        }
        else
        {
            ADM_info("[Muxer]Symbol loading failed for %s\n", file);
        }
    }
    virtual ~ADM_dynMuxer()
    {
        if (initialised)
            clearDefaultConfig();
    }
};

#define ADM_MUXER_NOEXT(Class, maj, mn, pat, name, desc, displayName, configureFunc, confTemplate, confVar, confSize)  \
    static void *defaultConfig = NULL;                                                                                 \
                                                                                                                       \
    static void snapshotDefaultConfiguration()                                                                         \
    {                                                                                                                  \
        if (confVar != NULL && defaultConfig == NULL)                                                                  \
        {                                                                                                              \
            defaultConfig = ADM_alloc(confSize);                                                                       \
            memcpy(defaultConfig, confVar, confSize);                                                                  \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    ADM_PLUGIN_EXPORT ADM_muxer *create(void)                                                                          \
    {                                                                                                                  \
        return new Class;                                                                                              \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT void destroy(ADM_muxer *h)                                                                       \
    {                                                                                                                  \
        Class *z = (Class *)h;                                                                                         \
        delete z;                                                                                                      \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT uint8_t getVersion(uint32_t *major, uint32_t *minor, uint32_t *patch)                            \
    {                                                                                                                  \
        *major = maj;                                                                                                  \
        *minor = mn;                                                                                                   \
        *patch = pat;                                                                                                  \
        return 1;                                                                                                      \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT uint32_t getApiVersion(void)                                                                     \
    {                                                                                                                  \
        return ADM_MUXER_API_VERSION;                                                                                  \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT const char *getName(void)                                                                        \
    {                                                                                                                  \
        return name;                                                                                                   \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT const char *getDescriptor(void)                                                                  \
    {                                                                                                                  \
        return desc;                                                                                                   \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT const char *getDisplayName(void)                                                                 \
    {                                                                                                                  \
        return displayName;                                                                                            \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT void clearDefaultConfig(void)                                                                    \
    {                                                                                                                  \
        ADM_dealloc(defaultConfig);                                                                                    \
        defaultConfig = NULL;                                                                                          \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT bool getConfiguration(CONFcouple **conf)                                                         \
    {                                                                                                                  \
        if (confTemplate == NULL)                                                                                      \
        {                                                                                                              \
            *conf = NULL;                                                                                              \
            return true;                                                                                               \
        }                                                                                                              \
        return ADM_paramSave(conf, confTemplate, confVar);                                                             \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT bool setConfiguration(CONFcouple *conf)                                                          \
    {                                                                                                                  \
        snapshotDefaultConfiguration();                                                                                \
        return ADM_paramLoad(conf, confTemplate, confVar);                                                             \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT bool resetConfiguration()                                                                        \
    {                                                                                                                  \
        snapshotDefaultConfiguration();                                                                                \
        if (defaultConfig != NULL)                                                                                     \
            memcpy(confVar, defaultConfig, confSize);                                                                  \
        return true;                                                                                                   \
    }                                                                                                                  \
    ADM_PLUGIN_EXPORT bool configure(void)                                                                             \
    {                                                                                                                  \
        snapshotDefaultConfiguration();                                                                                \
        if (configureFunc == NULL)                                                                                     \
            return true;                                                                                               \
        return configureFunc();                                                                                        \
    }

#define ADM_MUXER_BEGIN(Ext, Class, maj, mn, pat, name, desc, displayName, configureFunc, confTemplate, confVar,       \
                        confSize)                                                                                      \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        ADM_MUXER_NOEXT(Class, maj, mn, pat, name, desc, displayName, configureFunc, confTemplate, confVar, confSize)  \
        ADM_PLUGIN_EXPORT const char *getDefaultExtension(void)                                                        \
        {                                                                                                              \
            return Ext;                                                                                                \
        }                                                                                                              \
    }

#define ADM_MUXER_DYN_EXT(extFunc, Class, maj, mn, pat, name, desc, displayName, configureFunc, confTemplate, confVar, \
                          confSize)                                                                                    \
    extern "C"                                                                                                         \
    {                                                                                                                  \
        ADM_MUXER_NOEXT(Class, maj, mn, pat, name, desc, displayName, configureFunc, confTemplate, confVar, confSize)  \
        ADM_PLUGIN_EXPORT const char *getDefaultExtension(void)                                                        \
        {                                                                                                              \
            if (extFunc)                                                                                               \
                return extFunc();                                                                                      \
            return NULL;                                                                                               \
        }                                                                                                              \
    }

#endif
// EOF
