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

#include "ADM_default.h"
#include "fourcc.h"
#include "muxerMp4v2.h"
#include "ADM_codecType.h"
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif



/**
    \fn     muxerMp4v2
    \brief  Constructor
*/
muxerMp4v2::muxerMp4v2()
{
        handle=NULL;
};
/**
    \fn     muxerMp4v2
    \brief  Destructor
*/

muxerMp4v2::~muxerMp4v2()
{
    printf("[Mp4v2Muxer] Destructing\n");
    if(handle)
        ADM_error("MP4V2: File still opened\n");
}

/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerMp4v2::open(const char *file, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{

//        audioDelay=s->getVideoDelay();
        vStream=s;
        nbAStreams=nbAudioTrack;
        aStreams=a;
// Verify everything is ok : Accept Mp4 & H264 for video, AAC for audio
        uint32_t fcc=vStream->getFCC();
        if(!isH264Compatible(fcc) && !isMpeg4Compatible(fcc))
        {
            ADM_error("[mp4v2] Only h264 and mp4 video track!\n");
            return false;
        }
        for(int i=0;i<nbAStreams;i++)
        {
            if(aStreams[i]->getInfo()->encoding!=WAV_AAC)
            {
                ADM_error("[mp4v2] Only AAC audio!\n");
                return false;
            }
        }
        // Create file
        handle=MP4Create( file, MP4_DETAILS_ERROR, 0 ); // FIXME MP4_CREATE_64BIT_DATA
        if(MP4_INVALID_FILE_HANDLE==handle)
        {
            ADM_error("[mp4v2]Cannot create output file %s\n",file);
            return false;
        }
        if (!(MP4SetTimeScale( handle, 1000*1000 ))) // Us, optimize
        {
            ADM_error("[mp4v2]Cannot set timescale to us\n");
            return false;
        }
        
        return true;
}

/**
    \fn save
*/
bool muxerMp4v2::save(void)
{
    bool result=true;
    printf("[Mp4v2Muxer] Saving\n");
    
    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerMp4v2::close(void)
{
    if(handle)
    {
            MP4Close(handle);
#warning run MP4Optimize
    }
    handle=NULL;
    ADM_info("[Mp4v2Muxer] Closing\n");
    return true;
}
//EOF



