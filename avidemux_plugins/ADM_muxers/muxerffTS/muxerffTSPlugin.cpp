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
#include "muxerffTS.h"
#include "ts_muxer_desc.cpp"
#include "fourcc.h"
 bool ffTSConfigure(void);

extern "C" {
static void *defaultConfig = NULL;

static void snapshotDefaultConfiguration(void)
{
    if (defaultConfig)
        return;
    size_t confSize = sizeof(ts_muxer);
    defaultConfig = ADM_alloc(confSize);
    memcpy(defaultConfig, &tsMuxerConfig, confSize);
}

ADM_PLUGIN_EXPORT ADM_muxer *create(void)
{
    return new muxerffTS;
}
ADM_PLUGIN_EXPORT void destroy(ADM_muxer *m)
{
    muxerffTS *z = (muxerffTS *)m;
    delete z;
}
ADM_PLUGIN_EXPORT uint8_t getVersion(uint32_t *major, uint32_t *minor, uint32_t *patch)
{
    *major = 1;
    *minor = 0;
    *patch = 1;
    return 1;
}
ADM_PLUGIN_EXPORT uint32_t getApiVersion(void)
{
    return ADM_MUXER_API_VERSION;
}
ADM_PLUGIN_EXPORT const char *getName(void)
{
    return "ffTS";
}
ADM_PLUGIN_EXPORT const char *getDescriptor(void)
{
    return "ffMpeg TS muxer plugin (c) Mean 2009";
}
ADM_PLUGIN_EXPORT const char *getDisplayName(void)
{
    return "Mpeg TS Muxer (ff)";
}
ADM_PLUGIN_EXPORT const char *getDefaultExtension(void)
{
    return tsMuxerConfig.m2TsMode ? "m2ts" : "ts";
}
ADM_PLUGIN_EXPORT void clearDefaultConfig(void)
{
    ADM_dealloc(defaultConfig);
    defaultConfig = NULL;
}
ADM_PLUGIN_EXPORT bool getConfiguration(CONFcouple **conf)
{
    return ADM_paramSave(conf, ts_muxer_param, &tsMuxerConfig);
}
ADM_PLUGIN_EXPORT bool setConfiguration(CONFcouple *conf)
{
    snapshotDefaultConfiguration();
    return ADM_paramLoad(conf, ts_muxer_param, &tsMuxerConfig);
}
ADM_PLUGIN_EXPORT bool resetConfiguration(void)
{
    snapshotDefaultConfiguration();
    if (defaultConfig)
        memcpy(&tsMuxerConfig, defaultConfig, sizeof(ts_muxer));
    return true;
}
ADM_PLUGIN_EXPORT bool configure(void)
{
    snapshotDefaultConfiguration();
    return ffTSConfigure();
}
}

