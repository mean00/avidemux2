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
#include "ADM_codecType.h"
#include "ADM_imageFlags.h"
#include "ADM_videoInfoExtractor.h"
#include "muxerLmkv.h"

#define cprintf(...) {}

#if 0
#define aprintf(...) {}
#else
#define aprintf printf
#endif

static char *idFromFourcc(uint32_t fcc)
{
    if(isH264Compatible(fcc))
        return MK_VCODEC_MP4AVC;
    if(isMpeg4Compatible(fcc))
        return MK_VCODEC_MP4ASP;
    if(isMpeg12Compatible(fcc))
        return MK_VCODEC_MPEG2;
    if(fourCC::check(fcc,(uint8_t *)"DIV3"))
        return MK_VCODEC_MSMP4V3;
    return NULL;

}

/**
 * \fn setupVideo
 * @param vid
 * @return 
 */
bool muxerLmkv::setupVideo(ADM_videoStream *vid)
{
        videoStream=vid;
       // Create video track
        mk_TrackConfig videoConf;
        memset(&videoConf,0,sizeof(videoConf));
        
        
        // Main setup ...
        
        videoConf.trackType=MK_TRACK_VIDEO;
        videoConf.flagEnabled=true;
        videoConf.flagDefault=1;
        
        double fps=vid->getAvgFps1000();
        if(!fps) fps=25000;
                
        fps=1000000000000./fps;
        videoConf.defaultDuration=(uint64_t)fps;
        videoFrameDuration=(uint64_t)(fps/1000LL); // ns -> us
        aprintf("Default video frame duration =%d ns\n",(int)fps);
        aprintf("Default video frame duration =%d us\n",(int)videoFrameDuration);
        videoConf.codecID=idFromFourcc(vid->getFCC());
        if(!videoConf.codecID)
        {
            ADM_warning("Unsupported video codec\n");
            return false;
        }
        
        // Fill in description of video
        
        videoConf.extra.video.aspectRatioType=MK_ASPECTRATIO_FREE;
        videoConf.extra.video.pixelWidth=vid->getWidth();
        videoConf.extra.video.pixelHeight=vid->getHeight();
        
        videoConf.extra.video.displayWidth=videoConf.extra.video.pixelWidth;
        videoConf.extra.video.displayHeight=videoConf.extra.video.pixelHeight;
        
        videoConf.extra.video.displayUnit=0; // in pixels
        
        // Get extradata if needed...
        uint8_t *extraData;
        uint32_t extraDataLen;
        vid->getExtraData(&extraDataLen,&extraData);        
        videoConf.codecPrivate=extraData;
        videoConf.codecPrivateSize=extraDataLen;
        
        videoTrack=mk_createTrack(instance,&videoConf);
        if(!videoTrack)
        {
            ADM_warning("Cannot create video track\n");
            return false;
        }
        uint32_t size=vid->getWidth()*videoStream->getHeight()*3;
        uint8_t *buffer=new uint8_t[size];
        uint8_t *buffer2=new uint8_t[size];
        
        s[0].data=buffer;
        s[1].data=buffer2;
        s[0].bufferSize=size;
        s[1].bufferSize=size;
        
        // Preload first image..
        if(!videoStream->getPacket(s))
        {
            ADM_warning("Cannot get 1st frame\n");
            return false;
        }
        videoToggle=1;
        return true;
}
/**
 * \fn writeVideo
 * \brief write one video frame, videoDts is the output DTS of the written frame
 * @param videoDts
 * @return 
 */
bool muxerLmkv::writeVideo(uint64_t &videoDts)
{
    
        if(!videoStream->getPacket(s+videoToggle))
        {
            ADM_warning("[LMKV] Cant get video frame\n");
            return false;
        }
        int r;
        
        ADMBitstream *thisOne=s+(videoToggle^1);
        ADMBitstream *nextOne=s+videoToggle;
        videoToggle^=1;
        
        r=mk_startFrame(instance,videoTrack);
        cprintf("Start :%d\n",r);
        
        r=mk_addFrameData(instance,videoTrack, thisOne->data,thisOne->len);
        cprintf("addData :%d\n",r);
        
        int key=0;
        if(thisOne->flags & AVI_KEY_FRAME)
        {
            key=1;
        }
        uint64_t timeStamp=thisOne->pts;
        printf("Incoming pts= %d\n",(int)timeStamp);
        if(timeStamp==ADM_NO_PTS) 
                timeStamp=0;
        else
                timeStamp*=scale; // us -> ns
        int64_t duration=0;
        if(thisOne->dts!=ADM_NO_PTS && nextOne->dts!=ADM_NO_PTS)
            duration=(nextOne->dts-thisOne->dts)*scale;
        else
            duration=0;
        r= mk_setFrameFlags(instance,videoTrack,timeStamp,key,duration); // us -> ns
	cprintf("setFlags :%d\n",r);				 
       // printf("Writting frame with pts=%d duration=%d\n",(int)timeStamp,(int)duration);
        r=mk_flushFrame(instance,videoTrack);
        cprintf("Flush :%d\n",r);
    
        videoDts=thisOne->dts;
        return true;
}

// EOF
