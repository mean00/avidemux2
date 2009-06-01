
/***************************************************************************
  \file DIA_plugins.cpp
  (C) 2008 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#include "config.h"
#include "ADM_default.h"
#include "DIA_factory.h"

/* Functions we need to get infos */
uint32_t ADM_ad_getNbFilters(void);
bool     ADM_ad_getFilterInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t ADM_av_getNbDevices(void);
bool     ADM_av_getDeviceInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t ADM_ve_getNbEncoders(void);
bool     ADM_ve_getEncoderInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t ADM_ae_getPluginNbEncoders(void);
bool     ADM_ae_getAPluginEncoderInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t ADM_dm_getNbDemuxers(void);
bool     ADM_dm_getDemuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t ADM_mx_getNbMuxers(void);
bool     ADM_mx_getMuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
bool     ADM_ve6_getDemuxerInfo(int filter, const char **name, uint32_t *major,uint32_t *minor,uint32_t *patch);
uint32_t ADM_ve6_getNbDemuxers(void);
#define QT_TR_NOOP(x) x

/* /Functions */
/**
        \fn DIA_pluginsInfo
        \brief Display loaded plugin infos        

*/
uint8_t DIA_pluginsInfo(void)
{
    uint32_t aNbPlugin=ADM_ad_getNbFilters();
    uint32_t avNbPlugin=ADM_av_getNbDevices();
    uint32_t aeNbPlugin=ADM_ae_getPluginNbEncoders();
    uint32_t dmNbPlugin=ADM_dm_getNbDemuxers();
    uint32_t mxNbPlugin=ADM_mx_getNbMuxers();
    uint32_t ve6NbPlugin=ADM_ve6_getNbDemuxers();

    // Audio Plugins

    printf("[Audio Plugins] Found %u plugins\n",aNbPlugin);
    diaElemReadOnlyText *aText[aNbPlugin];
    diaElemFrame frameAudio(QT_TR_NOOP("Audio Plugins"));
        
       
    for(int i=0;i<aNbPlugin;i++)
    {
        const char *name;
        uint32_t major,minor,patch;
        char versionString[256];
        char infoString[256];
        char *end;
            ADM_ad_getFilterInfo(i, &name,&major,&minor,&patch);
            snprintf(versionString,255,"%02d.%02d.%02d",major,minor,patch);
            strncpy(infoString,name,255);
            if(strlen(infoString))
            {
                end=strlen(infoString)+infoString-1;
                // Remove trailing line feed
                while(*end==0x0a || *end==0x0d) *end--=0;
            }
            aText[i]=new diaElemReadOnlyText(infoString,versionString);
            frameAudio.swallow(aText[i]);
    }
    diaElem *diaAudio[]={&frameAudio};
    diaElemTabs tabAudio(QT_TR_NOOP("Audio"),1,diaAudio);
    // /Audio

    // Encoder
    printf("[VideoEncoder6 Plugins] Found %u plugins\n",ve6NbPlugin);
    diaElemReadOnlyText *veText[ve6NbPlugin];
    diaElemFrame frameVE(QT_TR_NOOP("Video Encoder Plugins"));
        
       
    for(int i=0;i<ve6NbPlugin;i++)
    {
        const char *name;
        uint32_t major,minor,patch;
        char versionString[256];
        char infoString[256];
        char *end;
            ADM_ve6_getDemuxerInfo(i, &name,&major,&minor,&patch);
            snprintf(versionString,255,"%02d.%02d.%02d",major,minor,patch);
            strncpy(infoString,name,255);
            if(strlen(infoString))
            {
                end=strlen(infoString)+infoString-1;
                // Remove trailing line feed
                while(*end==0x0a || *end==0x0d) *end--=0;
            }
            veText[i]=new diaElemReadOnlyText(infoString,versionString);
            frameVE.swallow(veText[i]);
    }

    diaElem *diaVE[]={&frameVE};
    diaElemTabs tabVE(QT_TR_NOOP("Video Encoder"),1,diaVE);

    // Audio Device
    printf("[AudioDevice Plugins] Found %u plugins\n",avNbPlugin);
    diaElemReadOnlyText *avText[avNbPlugin];
    diaElemFrame frameAV(QT_TR_NOOP("Audio Device Plugins"));
    
 for(int i=0;i<avNbPlugin;i++)
    {
        const char *name;
        uint32_t major,minor,patch;
        char versionString[256];
        char infoString[256];
        char *end;
            ADM_av_getDeviceInfo(i, &name,&major,&minor,&patch);
            snprintf(versionString,255,"%02d.%02d.%02d",major,minor,patch);
            strncpy(infoString,name,255);
            if(strlen(infoString))
            {
                end=strlen(infoString)+infoString-1;
                // Remove trailing line feed
                while(*end==0x0a || *end==0x0d) *end--=0;
            }
            avText[i]=new diaElemReadOnlyText(infoString,versionString);
            frameAV.swallow(avText[i]);
    }
    diaElem *diaAV[]={&frameAV};
    diaElemTabs tabAV(QT_TR_NOOP("Audio Device"),1,diaAV);

    // /Encoder

  // Audio Encoder
    printf("[AudioEncoder Plugins] Found %u plugins\n",aeNbPlugin);
    diaElemReadOnlyText *aeText[aeNbPlugin];
    diaElemFrame frameAE(QT_TR_NOOP("Audio Encoder Plugins"));
    
 for(int i=0;i<aeNbPlugin;i++)
    {
        const char *name;
        uint32_t major,minor,patch;
        char versionString[256];
        char infoString[256];
        char *end;
            ADM_ae_getAPluginEncoderInfo(i, &name,&major,&minor,&patch);
            snprintf(versionString,255,"%02d.%02d.%02d",major,minor,patch);
            strncpy(infoString,name,255);
            if(strlen(infoString))
            {
                end=strlen(infoString)+infoString-1;
                // Remove trailing line feed
                while(*end==0x0a || *end==0x0d) *end--=0;
            }
            aeText[i]=new diaElemReadOnlyText(infoString,versionString);
            frameAE.swallow(aeText[i]);
    }
    diaElem *diaAE[]={&frameAE};
    diaElemTabs tabAE(QT_TR_NOOP("Audio Encoders"),1,diaAE);

    // /Audio Encoder

 // Demuxer Encoder
    printf("[Demuxers Plugins] Found %u plugins\n",dmNbPlugin);
    diaElemReadOnlyText *dmText[dmNbPlugin];
    diaElemFrame frameDM(QT_TR_NOOP("Demuxer Plugins"));
    
 for(int i=0;i<dmNbPlugin;i++)
    {
        const char *name;
        uint32_t major,minor,patch;
        char versionString[256];
        char infoString[256];
        char *end;
            ADM_dm_getDemuxerInfo(i, &name,&major,&minor,&patch);
            snprintf(versionString,255,"%02d.%02d.%02d",major,minor,patch);
            strncpy(infoString,name,255);
            if(strlen(infoString))
            {
                end=strlen(infoString)+infoString-1;
                // Remove trailing line feed
                while(*end==0x0a || *end==0x0d) *end--=0;
            }
            dmText[i]=new diaElemReadOnlyText(infoString,versionString);
            frameDM.swallow(dmText[i]);
    }
    diaElem *diaDM[]={&frameDM};
    diaElemTabs tabDM(QT_TR_NOOP("Demuxers"),1,diaDM);

    // /Demuxer Encoder


 // muxer Encoder
    printf("[Muxers Plugins] Found %u plugins\n",mxNbPlugin);
    diaElemReadOnlyText *mxText[mxNbPlugin];
    diaElemFrame frameMX(QT_TR_NOOP("Muxer Plugins"));
    
 for(int i=0;i<mxNbPlugin;i++)
    {
        const char *name;
        uint32_t major,minor,patch;
        char versionString[256];
        char infoString[256];
        char *end;
            ADM_mx_getMuxerInfo(i, &name,&major,&minor,&patch);
            snprintf(versionString,255,"%02d.%02d.%02d",major,minor,patch);
            strncpy(infoString,name,255);
            if(strlen(infoString))
            {
                end=strlen(infoString)+infoString-1;
                // Remove trailing line feed
                while(*end==0x0a || *end==0x0d) *end--=0;
            }
            mxText[i]=new diaElemReadOnlyText(infoString,versionString);
            frameMX.swallow(mxText[i]);
    }
    diaElem *diaMX[]={&frameMX};
    diaElemTabs tabMX(QT_TR_NOOP("Muxers"),1,diaMX);

    // /muxer Encoder

    diaElemTabs *tabs[]={&tabAudio,&tabVE,&tabAV,&tabAE,&tabDM,&tabMX};
    diaFactoryRunTabs(QT_TR_NOOP("Plugins Info"),6,tabs);

    for(int i=0;i<aNbPlugin;i++)
        delete aText[i];
    for(int i=0;i<ve6NbPlugin;i++)
        delete veText[i];
    for(int i=0;i<avNbPlugin;i++)
        delete avText[i];
    for(int i=0;i<aeNbPlugin;i++)
        delete aeText[i];
    for(int i=0;i<dmNbPlugin;i++)
        delete dmText[i];
    for(int i=0;i<mxNbPlugin;i++)
        delete mxText[i];

    return 1;
}
