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
    \fn setEsdsAtom
    \brief extract esds atom from extradata or for first frame. In all cases read the first frame
*/
bool muxerMp4v2::setMpeg4Esds(void)
{
    bool removeVol=false;
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
    \fn initVideo
*/
bool muxerMp4v2::initVideo(void)
{
        uint32_t fcc=vStream->getFCC();

        if(false==vStream->getPacket(&(in[0])))
        {
            ADM_error("Cannot read 1st video frame\n");
            return false;
        }
        nextWrite=1;
        ADM_info("Setting video..\n");
        if(isMpeg4Compatible(fcc))
        {
            videoTrackId=MP4AddVideoTrack(handle,90000,MP4_INVALID_DURATION,
                    vStream->getWidth(),vStream->getHeight(),MP4_MPEG4_VIDEO_TYPE);
            if(MP4_INVALID_TRACK_ID==videoTrackId)
            {
                ADM_error("Cannot add mpeg4 video Track \n");
                return false;
            }
            if(false==setMpeg4Esds())
            {
                ADM_error("Cannot set ESDS atom\n");
                return false;
            }
        }
        if(isH264Compatible(fcc))
        {
#if 0
            // Extract sps & pps
            uint8_t *sps,*pps;
            //
            videoTrackId=MP4AddH264VideoTrack(handle,90000,MP4_INVALID_DURATION,
                    vStream->getWidth(),vStream->getHeight(),sps[0],sps[1],sps[2],3);
            if(MP4_INVALID_TRACK_ID==videoTrackId)
            {
                ADM_error("Cannot add h264 video Track \n");
                goto er;
            }
            if(false==MP4SetTrackESConfiguration(handle,videoTrackId,esdsData,esdsLen))
            {
                ADM_error("SetTracEsConfiguration failed\n");
                return false;
            }
#endif
        }
        return true;
}
//EOF



