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
#include "ADM_coreUtils.h"

#if 0
    #define aprintf printf
#else
    #define aprintf(...) {}
#endif

/**
    \fn ADM_tsAccess
    \param name   [in] Name of the file to take audio from
    \param pid    [in] Pid of the audio track
    \param append [in] Flag to auto append files
    \param aacAdts[in] Set to true if the file is aac/adts
    \param myLen/myExtra[in] ExtraData if any
*/
ADM_tsAccess::ADM_tsAccess(const char *name,uint32_t pid,int append,ADM_TS_MUX_TYPE muxing,int myLen,uint8_t  *myExtra)
{
        this->pid=pid;
        if(!demuxer.open(name,append)) ADM_assert(0);
        packet=new TS_PESpacket(pid);
        this->muxing=muxing;
        ADM_info("Creating audio track, pid=%x, muxing =%d\n",pid,muxing);
        lastDts=ADM_NO_PTS;
        wrapCount=0;
        if(myLen && myExtra)
        {
            extraDataLen=myLen;
            extraData=new uint8_t [myLen];
            memcpy(extraData,myExtra,extraDataLen);
            ADM_info("Creating ts audio access with %u bytes of extradata.",extraDataLen);
            mixDump(extraData,extraDataLen);
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
    if(seekPoints.size())
        return (seekPoints[seekPoints.size()-1].size);
    return 0;
}
/**
    \fn getDurationInUs
    \brief Rememember seekPoint.dts time is already scaled and in us
*/
uint64_t  ADM_tsAccess::getDurationInUs(void)
{
    if(!seekPoints.size()) return 0;
    // Take last seek point; should be accurate enough
    int offset=seekPoints.size()-1;
    while(offset)
    {
        uint64_t dts=seekPoints[offset].dts;
        if(dts==ADM_NO_PTS)
        {
            offset--;
            continue;
        }
        return dts;
    }
    return 0; // ?
}
/**
    \fn updateExtraData
    \brief Re-extract extradata from audio stream at given position
*/
bool ADM_tsAccess::updateExtraData(uint64_t start)
{
    int retries=20;
    demuxer.setPos(start);
    switch(muxing)
    {
        case ADM_TS_MUX_ADTS:
        {
            int insize=0;
            int dummySize;
            uint8_t *ptr=NULL;
            while(retries)
            {
                if(false==demuxer.getNextPES(packet)) return false;
                int avail=packet->payloadSize-packet->offset;
                insize=avail;
                ptr=packet->payload+packet->offset;
                if(ADM_adts2aac::ADTS_OK==adts.convert2(insize,ptr,&dummySize,NULL))
                {
                    uint32_t len=0;
                    uint8_t *data=NULL;
                    adts.getExtraData(&len,&data);
                    if(len!=2) return false;
                    if(extraDataLen!=len)
                    {
                        delete [] extraData;
                        extraData=new uint8_t[len];
                        extraDataLen=len;
                    }
                    memcpy(extraData,data,len);
                    ADM_info("AAC ADTS extradata:\n");
                    mixDump(extraData,extraDataLen);
                    adts.reset();
                    return true;
                }
                retries--;
            }
        }
            break;
        case ADM_TS_MUX_LATM:
        {
            uint64_t time=ADM_NO_PTS;
            while(retries--)
            {
                if(false==demuxer.getNextPES(packet)) return false;
                int avail=packet->payloadSize-packet->offset;
                if(false==latm.pushData(avail,packet->payload+packet->offset))
                    return false;
                ADM_latm2aac::LATM_STATE outcome=latm.convert(packet->pts);
                if(outcome==ADM_latm2aac::LATM_ERROR)
                    continue;
                if(outcome==ADM_latm2aac::LATM_MORE_DATA_NEEDED)
                    continue;
                uint32_t len=0;
                uint8_t *data=NULL;
                if(latm.getExtraData(&len,&data) && len && data)
                {
                    if(extraDataLen!=len)
                    {
                        delete [] extraData;
                        extraData=new uint8_t[len];
                        extraDataLen=len;
                    }
                    memcpy(extraData,data,len);
                    ADM_info("AAC LATM extradata:\n");
                    mixDump(extraData,extraDataLen);
                    latm.flush();
                    return true;
                }
             }
        }
            break;
        default:
            return true;
    }
    return false;
}
/**
    \fn goToTime
    \brief Rememember seekPoint.dts time is already scaled and in us
*/                              
bool      ADM_tsAccess::goToTime(uint64_t timeUs)
{
    latm.flush();
    adts.reset();
    if(!seekPoints.size())
        return false;

    if(timeUs<seekPoints[0].dts)
    {
            aprintf("[PsAudio] Requested %" PRIu32" tick before 1st seek point at :%" PRIu32"\n",(uint32_t)timeUs/1000,(uint32_t)seekPoints[0].dts/1000);
            updateExtraData(seekPoints[0].position);
            demuxer.setPos(seekPoints[0].position);
            wrapCount=0;
            lastDts=ADM_NO_PTS;
            return true;
    }

    for(int i=1;i<seekPoints.size();i++)
    {
        if(seekPoints[i].dts >=timeUs )
        {
            aprintf("[PsAudio] Requested %" PRIu32" tick seeking to  at :%" PRIu32" us (next is %" PRIu32"ms \n",(uint32_t)timeUs/1000,
                    (uint32_t)seekPoints[i-1].dts/1000,
                    (uint32_t)seekPoints[i].dts/1000);
            updateExtraData(seekPoints[i-1].position);
            demuxer.setPos(seekPoints[i-1].position);
            uint64_t st=seekPoints[i-1].dts;
            if(st!=ADM_NO_PTS)
            {
                st /= 100;
                st *= 9; // now in ticks
                st >>= 32;
                wrapCount=(uint32_t)st;
            }
            lastDts=ADM_NO_PTS;
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
    const uint64_t wrapLen=1LL<<32;
    if(x<dtsOffset)
        x+=wrapLen;
    x-=dtsOffset;
    if(lastDts!=ADM_NO_PTS)
    {
        if(lastDts>x && lastDts-x >= wrapLen/2)
            wrapCount++;
        if(wrapCount && x>lastDts && x-lastDts > wrapLen/2)
            wrapCount--;
    }
    lastDts=x;
    x+=wrapLen*wrapCount;
    double f=x*100.;
    f/=9.;
    f+=0.49;
    return (uint64_t)f;

}
/**
    \fn getPacket
*/
bool      ADM_tsAccess::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    // If it is adts, ask ffmpeg to unwrap it...
    switch(muxing)
    {
        case ADM_TS_MUX_ADTS:
            {
                    int outsize=0;
                    *size=0;
                    bool gotPacket=false;
                    int insize=0;
                    uint8_t *ptr=NULL;
                    while(1)
                    {
                        // Manage several packet in packet
                        if(ADM_adts2aac::ADTS_OK==adts.convert2(insize,ptr,&outsize,buffer))
                        {
                            *size=outsize;
                            if(gotPacket)
                                *dts=timeConvert(packet->pts);
                            else
                                *dts=ADM_NO_PTS;
                            return true;
                        }
                        if(false==demuxer.getNextPES(packet)) return false;
                        int avail=packet->payloadSize-packet->offset;
                        if(avail>maxSize) ADM_assert(0);
                        insize=avail;
                        ptr=packet->payload+packet->offset;
                        gotPacket=true;
                    }
                    break;
            }
        case ADM_TS_MUX_NONE:
            {
                if(false==demuxer.getNextPES(packet)) return false;
                int avail=packet->payloadSize-packet->offset;
                if(avail>maxSize) ADM_assert(0);
                *size=avail;
                memcpy(buffer,packet->payload+packet->offset,avail);
                *dts=timeConvert(packet->pts);
                break;
            }
        case ADM_TS_MUX_LATM:
            {
                // Try to get one...
                int retries=40; // arbitrary
                bool gotPacket=false;
                uint64_t time=ADM_NO_PTS;
                while(latm.empty()) // fetch next LOAS frame, it will contain several frames
                {
                    if(!retries)
                    {
                        ADM_error("Cannot get AAC packet from LATM\n");
                        return false;
                    }
                    if(gotPacket) time=packet->pts;
                    ADM_latm2aac::LATM_STATE outcome = latm.convert(time);
                    if(outcome != ADM_latm2aac::LATM_OK)
                    {
                        if(outcome == ADM_latm2aac::LATM_ERROR)
                        {
                            ADM_warning("Error demuxing LATM frame, %d attempts remaining.\n",retries);
                            if(--retries > 10 /* arbitrary cap */) continue; // consume pushed data first
                            latm.flush();
                        }
                        if(false==demuxer.getNextPES(packet)) return false;
                        int avail=packet->payloadSize-packet->offset;
                        if(avail>maxSize) ADM_assert(0);
                        gotPacket=true;
                        if(false==latm.pushData(avail,packet->payload+packet->offset))
                            latm.flush();
                    }
                    retries--;
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
        aprintf("[psAudio] getPacket dts = %" PRIu32" ms\n",(uint32_t)*dts/1000);
    }
    return true;
}


//EOF
