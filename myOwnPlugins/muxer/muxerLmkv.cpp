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
        s[0].data=NULL;
        s[1].data=NULL;
        scale=1000;
        videoToggle=0;
        videoFrameDuration=40000;
};
/**
    \fn     muxerLmkv
    \brief  Destructor
*/

muxerLmkv::~muxerLmkv()
{
    ADM_info("[muxerLmkv] Destroying\n");
    close();
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerLmkv::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
       
        instance=mk_createWriter(file,1000LL,true);// unit is us so it's in us
        if(!instance)
        {
            ADM_warning("Cannot create instance\n");
            return false;
        }
        
        if(!setupVideo(s))
            goto fail;
        mk_writeHeader(instance,"adm26");
        return true;
fail:
        ADM_warning("Opening of libmkv failed\n");
        return false;
}
/**
    \fn save
*/
bool muxerLmkv::save(void)
{
   
    int toggle=0;
    bool result=true;
    
    
 
    printf("...\n");
    
    bool running=true;
    uint64_t videoDts=0;
    while(running)
    {
        if(!writeVideo(videoDts))            
            running=false;
    }

theEnd:

    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerLmkv::close(void)
{
    if(instance)
    {
        mk_close(instance);
        instance=NULL;
    }
    if(s[0].data)
    {
        delete [] s[0].data;
        s[0].data=NULL;
    }
    if(s[1].data)
    {
        delete [] s[1].data;
        s[1].data=NULL;
    }

    return true;
}
//EOF



