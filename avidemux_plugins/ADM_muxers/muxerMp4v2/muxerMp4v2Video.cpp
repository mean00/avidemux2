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
#include "ADM_imageFlags.h"
#include "ADM_videoInfoExtractor.h"
#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif

/**
    \fn loadNextVideoFrame
    \brief Load buffer, convert to annexB if needed
*/
bool muxerMp4v2::loadNextVideoFrame(ADMBitstream *bs)
{
    if(true==needToConvertFromAnnexB)
        {
            ADMBitstream tmp;
            tmp.data=scratchBuffer;
            tmp.bufferSize=videoBufferSize;
            if(false==vStream->getPacket(&tmp))
                return false;
            bs->dts=tmp.dts;
            bs->pts=tmp.pts;
            bs->flags=tmp.flags;
            bs->len=ADM_convertFromAnnexBToMP4(scratchBuffer,tmp.len, bs->data,videoBufferSize);
            return true;
        }
    if(false==vStream->getPacket(bs))
        {
            return false;
        }

    
    return true;
}
/**
    \fn setEsdsAtom
    \brief extract esds atom from extradata or for first frame. In all cases read the first frame
*/
bool muxerMp4v2::initMpeg4(void)
{
    bool removeVol=false;
    // Preload first image
    if(false==loadNextVideoFrame(&(in[0])))
        {
            ADM_error("Cannot read 1st video frame\n");
            return false;
        }
        nextWrite=1;

    videoTrackId=MP4AddVideoTrack(handle,90000,MP4_INVALID_DURATION,
    vStream->getWidth(),vStream->getHeight(),MP4_MPEG4_VIDEO_TYPE);
    if(MP4_INVALID_TRACK_ID==videoTrackId)
        {
            ADM_error("Cannot add mpeg4 video Track \n");
            return false;
        }
    ADM_info("Setting mpeg4 (a)SP ESDS...\n");
    if(0) //false==vStream->getPacket(&in) )
     {
        ADM_error("Cannot read first frame\n");
        return false;
     }
     uint8_t *esdsData=NULL;
     uint32_t esdsLen=0;
        if(false==vStream->getExtraData(&esdsLen,&esdsData))
        {
            ADM_info("No extradata, geting ESDS from first frame...\n");
        }else
        {
            ADM_info("Got esds from extradata\n");
        }
        if(!esdsLen) // We dont have extraData, look into the 1st frame
        {
            ADM_info("Trying to get VOL header from first frame...\n");
            if(!extractVolHeader(in[0].data,in[0].len,&esdsData,&esdsLen))
            {
                ADM_error("Cannot get ESDS, aborting\n");
                return false;
            }
            // Remove VOL Header from Fist frame...
            removeVol=true;
        }
        //
        if(!esdsLen)
        {
            ADM_error("ESDS not found, aborting\n");
            return false;
        }
        if(!esdsData[0] && !esdsData[1] && esdsData[2]==1)
        {
            // Remove startcode
            if(esdsLen<4)
            {
                ADM_error("ESDS too short\n");
                return false;
            }
            esdsData+=4;
            esdsLen-=4;
        }

        ADM_info("Esds:\n"); mixDump(esdsData,esdsLen);ADM_info("\n");            
        if(false==MP4SetTrackESConfiguration(handle,videoTrackId,esdsData,esdsLen))
        {
            ADM_error("SetTracEsConfiguration failed\n");
            return false;
        }
        ADM_info("ESDS atom set\n");
        if(removeVol)
        {
            uint32_t size=(uint32_t)((in[0].data+in[0].len)-(esdsData+esdsLen));
            memmove(in[0].data,esdsData+esdsLen,size);
            in[0].len=size;
        }
        return true;
}
/**
       \fn initH264
       \brief format header for H264
*/
bool muxerMp4v2::initH264(void)
{
//
            bool result=false;
            uint32_t spsLen;
            uint8_t  *spsData=NULL;
            uint32_t ppsLen;
            uint8_t  *ppsData=NULL;
            // Extract sps & pps            
            uint8_t *extra=NULL;
            uint32_t extraLen=0;
            if(false==vStream->getExtraData(&extraLen,&extra))
            {
                ADM_error("Cannot get extradata\n");
                return false;
            }
            if(extraLen)
                mixDump(extra,extraLen);
            ADM_info("\n");
            if(false==ADM_getH264SpsPpsFromExtraData(extraLen,extra,&spsLen,&spsData,&ppsLen,&ppsData))
            {
                ADM_error("Wrong extra data for h264\n");
                return false;
            }
            
            // if we dont have extraData, it is annexB 100 % sire
            needToConvertFromAnnexB=true;
            if(extraLen)
                if(extra[0]==1) needToConvertFromAnnexB=false;
            if(false==loadNextVideoFrame(&(in[0])))
            {
                ADM_error("Cannot read 1st video frame\n");
                return false;
            }
            nextWrite=1;
            //
            videoTrackId=MP4AddH264VideoTrack(handle,90000,MP4_INVALID_DURATION,
                    vStream->getWidth(),vStream->getHeight(),spsData[1],spsData[2],spsData[3],3);
            if(MP4_INVALID_TRACK_ID==videoTrackId)
            {
                ADM_error("Cannot add h264 video Track \n");
                return false;
            }
            ADM_info("SPS (%d) :",spsLen);
            mixDump(spsData,spsLen);
            ADM_info("PPS (%d) :",ppsLen);
            mixDump(ppsData,ppsLen);
            ADM_info("\n");

            MP4AddH264SequenceParameterSet(handle,videoTrackId, spsData,spsLen );
            MP4AddH264PictureParameterSet( handle,videoTrackId, ppsData,ppsLen);
            // MP4AddIPodUUID
            result=true;
clnup:
            if(spsData) delete [] spsData;
            if(ppsData) delete [] ppsData;
            spsData=NULL;
            ppsData=NULL;
            return result;
}
/**
    \fn initVideo
*/
bool muxerMp4v2::initVideo(void)
{
        uint32_t fcc=vStream->getFCC();
       
        ADM_info("Setting video..\n");
        if(isMpeg4Compatible(fcc))
        {
           
            if(false==initMpeg4())
            {
                ADM_error("Cannot set ESDS atom\n");
                return false;
            }
        }
        if(isH264Compatible(fcc))
        {
            if(false==initH264())
            {
                ADM_error("Cannot add h264 track\n");
                return false;
            }
        }
        double inc=vStream->getAvgFps1000();
        inc=inc/1000;
        if(inc>0.005) inc=1/inc;
                else inc=0.005;
        ADM_info("Frame increment =%d ms\n",(int)(inc*1000));
        inc*=90000;
        setMaxDurationPerChunk(videoTrackId, inc);
        ADM_info("[MP4V2] Video correctly initalized\n");
        return true;
}
//EOF



