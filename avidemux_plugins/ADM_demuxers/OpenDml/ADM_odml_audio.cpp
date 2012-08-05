/***************************************************************************
                          aviaudio.cpp  -  description
                             -------------------
    begin                : Fri Nov 23 2001
    copyright            : (C) 2001 by mean
    email                : fixounet@free.fr

This class deals with a chunked / not chunked stream
It is an fopen/fwrite lookalike interface to chunks



 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"
#include "fourcc.h"
#include "ADM_openDML.h"

#include "ADM_odml_audio.h"
//#include "ADM_audiocodec/ADM_audiocodec.h"
#include "ADM_audioStream.h"

/**
    \fn ADM_aviAudioAccess

*/
#define ODML_MAX_AUDIO_CHUNK (10*1024)
ADM_aviAudioAccess::ADM_aviAudioAccess(odmlIndex *idx,WAVHeader *hdr,
						uint32_t nbChunk,
						const char *name,
						uint32_t extraLen,
						uint8_t  *extraData)
{
    this->extraData=new uint8_t[extraLen];
    memcpy(this->extraData,extraData,extraLen);
    this->extraDataLen=extraLen;
    length=0;
    uint32_t mx=0;
    for(int i=0;i<nbChunk;i++) 
    {
        length+=idx[i].size;    
        if(idx[i].size>mx) mx=idx[i].size;
    }
    if((hdr->encoding==WAV_PCM || hdr->encoding==WAV_LPCM)&& mx>ODML_MAX_AUDIO_CHUNK)
    {
        // Split the huge chunk into smaller ones
        for(int i=0;i<nbChunk;i++)
        {
            uint64_t start=idx[i].offset;
            uint32_t size=idx[i].size;      
            uint32_t ONE_CHUNK=(ODML_MAX_AUDIO_CHUNK)/(2*hdr->channels);
            ONE_CHUNK*=2*hdr->channels;
            while(size>ONE_CHUNK)
            {
                odmlIndex current;
                current.offset=start;
                current.size=ONE_CHUNK;
                current.dts=ADM_NO_PTS;
                myIndex.append(current);
                start+=ONE_CHUNK;
                size-=ONE_CHUNK;
            }
                odmlIndex current;
                current.offset=start;
                current.size=size;
                current.dts=ADM_NO_PTS;
                myIndex.append(current);
        }

    }else
    {       // Duplicate index as is
        for(int i=0;i<nbChunk;i++)
          myIndex.append(idx[i]);
    }
    fd=ADM_fopen(name,"r");
    ADM_assert(fd);
    pos=0;
    currentIndex=0;
    wavHeader=hdr;
    currentPosition=0;
    nbIndex=myIndex.size(); // will not change
}
/**
    \fn isCBR

*/
bool      ADM_aviAudioAccess::isCBR(void) 
{
    if(wavHeader->encoding==WAV_MP3 && wavHeader->blockalign==1152) return false;
    return true;

}
/**
    \fn getPos

*/
bool  ADM_aviAudioAccess::updatePos(void)
{
 uint64_t total=0;
    if(currentIndex==0) 
    {
        currentPosition=0;
        return true;
    }
    
    if(currentIndex>=nbIndex) return length;
    for(int i=0;i<currentIndex;i++)
    {
        total+=myIndex[i].size;
    }
    currentPosition=total;
    return true;
}
bool ADM_aviAudioAccess::nextIndex(void)
{
        currentPosition+=myIndex[currentIndex].size;
        currentIndex++;
        return true;
}
uint64_t  ADM_aviAudioAccess::getPos(void)
{
    return currentPosition;
}

/**
        \fn ~ADM_aviAudioAccess
*/
 ADM_aviAudioAccess::~ADM_aviAudioAccess()
{
    if(fd)fclose(fd);
    fd=NULL;
    if(extraData) delete[] extraData;
    extraData=NULL;
    myIndex.clear();
}
/**
    \fn setPos
*/
 bool     ADM_aviAudioAccess::setPos(uint64_t pos)
{
    // go to the index just after the wanted one
    uint64_t total=0;
    for(int i=0;i<nbIndex-1;i++)
    {
        if(pos>=total && pos<=total+myIndex[i].size)
        {
            fseeko(fd,myIndex[i].offset,SEEK_SET);
            currentIndex=i;
            updatePos();
            return 1;
        }
        total+=myIndex[i].size;
    }
    printf("[aviAudioAccess] Seek to pos %"PRIu64" failed\n",pos);
    return 0;
}
/**
        \fn getPacket
        \brief
*/
bool   ADM_aviAudioAccess::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
        if(currentIndex>=nbIndex) 
        {
            printf("[OpenDmlDemuxer] Index Exceeded %d/%d\n",currentIndex,nbIndex);
            return 0;
        }
        uint32_t blockSize=myIndex[currentIndex].size;
        if(blockSize>maxSize) 
        {
            ADM_error("Packet too large %d, maximum is %d\n",blockSize,maxSize);
            *size=0;
            return false;
         }
        fseeko(fd,myIndex[currentIndex].offset,SEEK_SET);
        fread(buffer,1,blockSize,fd);
        if(!currentIndex) 
            *dts=0;
        else
            *dts=ADM_AUDIO_NO_DTS;
        *size=blockSize;
        nextIndex();
        return 1;
}
//EOF
