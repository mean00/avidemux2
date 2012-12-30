/**********************************************************************
            \file            muxerMp4v2
            \brief           libmp4v2 muxer
                             -------------------
    
    copyright            : (C) 2011 by mean
    email                : fixounet@free.fr
    Strongly inspired by handbrake code

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdarg.h>
#include "ADM_default.h"
#include "ADM_cpp.h"
#include "fourcc.h"
#include "muxerLmkv.h"
#include "ADM_codecType.h"
#include "ADM_imageFlags.h"

#if 1
#define aprintf(...) {}
#define cprintf(...) {}
#else
#define aprintf printf
#endif


/**
    \fn     muxerLmkv
    \brief  Constructor
*/
muxerLmkv::muxerLmkv()
{
        ADM_info("[muxerLmkv] Creating\n");
        instance=NULL;
        videoTrack=NULL;
};
/**
    \fn     muxerLmkv
    \brief  Destructor
*/

muxerLmkv::~muxerLmkv()
{
    ADM_info("[muxerLmkv] Destroying\n");
    if(instance)
    {
        mk_close(instance);
        instance=NULL;
    }
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerLmkv::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
        videoStream=s;
        instance=mk_createWriter(file,1000000000LL,false);// ns
        if(!instance)
        {
            ADM_warning("Cannot create instance\n");
            return false;
        }
        
        // Create video track
        mk_TrackConfig videoConf;
        memset(&videoConf,0,sizeof(videoConf));
        videoConf.trackType=MK_TRACK_VIDEO;
        videoConf.flagEnabled=true;
        videoConf.codecID=MK_VCODEC_MP4AVC;
        uint8_t *extraData;
        uint32_t extraDataLen;
        s->getExtraData(&extraDataLen,&extraData);
        
        videoConf.codecPrivate=extraData;
        videoConf.codecPrivateSize=extraDataLen;
        
        videoTrack=mk_createTrack(instance,&videoConf);
        if(!videoTrack)
        {
            ADM_warning("Cannot create video track\n");
            goto fail;
        }
        mk_writeHeader(instance,"adm26");
        return true;
fail:
        mk_close(instance);
        instance=NULL;
        return false;
}
/**
    \fn save
*/
bool muxerLmkv::save(void)
{
    uint32_t size=videoStream->getWidth()*videoStream->getHeight()*3;
    uint8_t *buffer=new uint8_t[size];
    ADMBitstream s(size);
    s.data=buffer;
    
    printf("...\n");
    while(videoStream->getPacket(&s))
    {
        int r;
        r=mk_startFrame(instance,videoTrack);
        cprintf("Start :%d\n",r);
        
        r=mk_addFrameData(instance,videoTrack, s.data,s.len);
        cprintf("addData :%d\n",r);
        
        int key=0;
        if(s.flags & AVI_KEY_FRAME)
        {
            key=1;
        }
        r= mk_setFrameFlags(instance,videoTrack,0,key,0); // us -> ns
	cprintf("setFlags :%d\n",r);				 
        
        r=mk_flushFrame(instance,videoTrack);
        cprintf("Flush :%d\n",r);
    }
    delete [] buffer;
    buffer=NULL;
    return false;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerLmkv::close(void)
{

    return true;
}
//EOF



