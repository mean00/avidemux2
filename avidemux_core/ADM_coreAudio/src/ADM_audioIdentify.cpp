/**
    \file  ADM_audioIdentify
    \brief Identify a codec used in a file

    \author copyright            : (C) 2012 by mean
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
#include "ADM_audiodef.h"
#include "ADM_mp3info.h"
#include "ADM_a52info.h"
#include "ADM_audioCodecEnum.h"
#include "ADM_audioIdentify.h"
/**
    \fn idMP2
    \brief return true if the tracks is mp2
*/
static bool idMP2(int bufferSize,const uint8_t *data,WAVHeader &info)
{
        const uint8_t *mp2Buffer=data;
        const int limit=bufferSize;
        // Now read buffer until we have 3 correctly decoded packet
        int probeIndex=0;
        while(probeIndex<limit)
        {

            const uint8_t *ptr=mp2Buffer+probeIndex;
            int     len=limit-probeIndex;
            if(len<4)
            {
                    ADM_info("\t no sync(3)\n");
                    return false;
            }
            uint32_t syncoff,syncoff2;
            MpegAudioInfo mp2info,confirm;
            if( !getMpegFrameInfo(ptr,len,&mp2info,NULL,&syncoff))
            {
                    ADM_info("\t no sync\n");
                    return false;
            }
          // Skip this packet
            int next=probeIndex+syncoff+mp2info.size;
            len=limit-next;
            if(len<4)
            {
                    ADM_info("\t no sync(2)\n");
                    return false;
            }
            if(getMpegFrameInfo(mp2Buffer+next,len,&confirm,&mp2info,&syncoff2))
            {
                    if(!syncoff2)
                    {
                            ADM_warning("\tProbably MP2 : Fq=%d br=%d chan=%d\n", (int)mp2info.samplerate,
                                                                (int)mp2info.bitrate,
                                                                (int)mp2info.mode);
                            // fill in info
                            info.frequency=mp2info.samplerate;
                            info.byterate=(mp2info.bitrate>>3)*1000;
                            if(mp2info.layer==1) info.encoding=WAV_MP2;
                                else             info.encoding=WAV_MP3;
                            if(mp2info.mode==1)
                                            info.channels=1;
                            else
                                            info.channels=2;
            
                            return true;
                    }
            }
            probeIndex+=syncoff+1;
        }
        return false;
}
/**
    \fn idAC3
    \brief return true if the tracks is mp2
*/
static bool idAC3(int bufferSize,const uint8_t *data,WAVHeader &info)
{
    uint32_t fq,br,chan,syncoff;
    uint32_t fq2,br2,chan2,syncoff2;

   if( !ADM_AC3GetInfo(data,bufferSize, &fq, &br, &chan,&syncoff))
    {
            ADM_info("Not ac3\n");
            return false;
    }
    // Need a 2nd packet...
    const uint8_t *second=data+syncoff;
    int size2=bufferSize-syncoff;
    ADM_assert(size2>0);
    ADM_info("Maybe AC3... \n");
    // Try on 2nd packet...
    if( ADM_AC3GetInfo(second,size2, &fq2, &br2, &chan2,&syncoff2))
    {
        if((fq==fq2) && (br2==br) && (chan==chan2))
        {
            ADM_warning("\tProbably AC3 : Fq=%d br=%d chan=%d\n",(int)fq,(int)br,(int)chan);
            info.encoding=WAV_AC3;
            info.channels=chan;
            info.byterate=(br>>3)*1000;
            info.frequency=fq;
            return true;
        }
    }
    ADM_info("Cannot confirm ac3\n");
    return false;
}
/**
    \fn ADM_identifyAudioStream
*/
bool ADM_identifyAudioStream(int bufferSize,const uint8_t *buffer,WAVHeader &info)
{
    memset(&info,0,sizeof(info));
    if(idMP2(bufferSize,buffer,info)) return true;
    if(idAC3(bufferSize,buffer,info)) return true;
    return false;
}
// EOF
