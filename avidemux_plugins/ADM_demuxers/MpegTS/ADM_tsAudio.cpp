/***************************************************************************
    \file ADM_tsAudio.cpp

    copyright            : (C) 2006/2009 by mean
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

#include "ADM_default.h"
#include "ADM_Video.h"
#include "ADM_aacinfo.h"
#include <string.h>
#include <math.h>

#include "ADM_ts.h"

#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif
void        mixDump(uint8_t *ptr, uint32_t len);
/**
    \fn ADM_tsAccess
    \param name   [in] Name of the file to take audio from
    \param pid    [in] Pid of the audio track
    \param append [in] Flag to auto append files (ignored for now)
    \param aacAdts[in] Set to true if the file is aac/adts
    \param myLen/myExtra[in] ExtraData if any
*/
ADM_tsAccess::ADM_tsAccess(const char *name,uint32_t pid,bool append,ADM_TS_MUX_TYPE muxing,int myLen,uint8_t  *myExtra)
{
FP_TYPE fp=FP_DONT_APPEND;
        if(append) fp=FP_APPEND;
        this->pid=pid;
        if(!demuxer.open(name,fp)) ADM_assert(0);
        packet=new TS_PESpacket(pid);
        this->muxing=muxing;
        ADM_info("Creating audio track, pid=%x, muxing =%d\n",pid,muxing);
        if(myLen && myExtra)
        {   
            extraData=new uint8_t [myLen];
            extraDataLen=myLen;
            memcpy(extraData,myExtra,extraDataLen);
            ADM_info("Creating ts audio access with %d bytes of extradata.",myLen);
            mixDump(extraData,extraDataLen);
            ADM_info("\n");
        }
}

/**
    \fn ~ADM_tsAccess
*/
ADM_tsAccess::~ADM_tsAccess()
{
    demuxer.close();
    if(packet) delete packet;
    packet=NULL;
    if(extraData) delete [] extraData;
    extraData=NULL;
}
/**
    \fn push
    \brief add a seek point.
*/
bool      ADM_tsAccess::push(uint64_t at, uint64_t dts,uint32_t size)
{
ADM_mpgAudioSeekPoint s;
            s.position=at;
            s.dts=dts;
            s.size=size;
            seekPoints.push_back(s);
            return true;
}
/**
    \fn getLength
*/
uint32_t  ADM_tsAccess::getLength(void)
{
  return (seekPoints[seekPoints.size()-1].size);

}
/**
    \fn getDurationInUs
*/
uint64_t  ADM_tsAccess::getDurationInUs(void)
{
    if(!seekPoints.size()) return 0;
    // Take last seek point; should be accurate enough
    return timeConvert(seekPoints[seekPoints.size()-1].dts);
}
/**
    \fn goToTime
    \brief Rememember seekPoint.dts time is already scaled and in us
*/                              
bool      ADM_tsAccess::goToTime(uint64_t timeUs)
{
    // Convert time in us to scaled 90 kHz tick
    latm.flush();
    if(timeUs<seekPoints[0].dts)
    {
            aprintf("[PsAudio] Requested %"LU" tick before 1st seek point at :%"LU"\n",(uint32_t)timeUs/1000,(uint32_t)seekPoints[0].dts/1000);
            demuxer.setPos(seekPoints[0].position);
            return true;
    }

    for(int i=1;i<seekPoints.size();i++)
    {
        if(seekPoints[i].dts >=timeUs )
        {
            aprintf("[PsAudio] Requested %"LU" tick seeking to  at :%"LU" us (next is %"LU"ms \n",(uint32_t)timeUs/1000,
                    (uint32_t)seekPoints[i-1].dts/1000,
                    (uint32_t)seekPoints[i].dts/1000);
            demuxer.setPos(seekPoints[i-1].position);
            return true;
        }
    }
    return false;
}
/**
    \fn timeConvert
    \brief Convert time in ticks raw from the stream to avidemux time in us starting from the beginning of the file
*/
uint64_t ADM_tsAccess::timeConvert(uint64_t x)
{
    if(x==ADM_NO_PTS) return ADM_NO_PTS;
    if(x<dtsOffset)
    {
        x+=1LL<<32;
    }
    x=x-dtsOffset;
    x=x*1000;
    x/=90;
    return x;

}
/**
    \fn getPacket
*/
bool      ADM_tsAccess::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
int retries=10;
again:
uint64_t p,d,start;
    if(false==demuxer.getNextPES(packet)) return false;
    int avail=packet->payloadSize-packet->offset;
    if(avail>maxSize) ADM_assert(0);
    *size=avail;
    // If it is adts, ask ffmpeg to unwrap it...
    switch(muxing)
    {
        case ADM_TS_MUX_ADTS:
            {
                    bool r=false;
                    int outsize=0;
                    *size=0;
                    r=adts.convert(avail,packet->payload+packet->offset,&outsize,buffer);
                    if(false==r) return false;
                    *size=outsize;
                    *dts=timeConvert(packet->pts);
                    break;
            }
        case ADM_TS_MUX_NONE:
            {
                memcpy(buffer,packet->payload+packet->offset,avail);
                *dts=timeConvert(packet->pts);
                break;
            }
        case ADM_TS_MUX_LATM:
            {
                // Try to get one...
                
                if(!retries)
                {
                    ADM_warning("No packet out of latm\n");
                    return false;   
                }
                if(latm.empty()==true)
                {
                        latm.pushData(avail,packet->payload+packet->offset,packet->pts);
                        retries--;
                        goto again;
                 }
                 uint64_t myPts;
                 latm.getData(&myPts,size,buffer,maxSize);
                 *dts=timeConvert(myPts);
                 break;
            }
        default:
            ADM_assert(0);
     }
    if(*dts!=ADM_NO_PTS) 
    {
        aprintf("[psAudio] getPacket dts = %"LU" ms\n",(uint32_t)*dts/1000);
    }
    return true;
}


//EOF
