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
ADM_aviAudioAccess::ADM_aviAudioAccess(odmlIndex *idx,WAVHeader *hdr,
						uint32_t nbchunk,
						const char *name,
						uint32_t extraLen,
						uint8_t  *extraData)
{
    this->extraData=new uint8_t[extraLen];
    memcpy(this->extraData,extraData,extraLen);
    this->extraDataLen=extraLen;
    index=idx;
    nbIndex=nbchunk;
    length=0;
    for(int i=0;i<nbIndex;i++) length+=idx[i].size;
    fd=ADM_fopen(name,"r");
    ADM_assert(fd);
    pos=0;
    currentIndex=0;
    wavHeader=hdr;
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
uint64_t  ADM_aviAudioAccess::getPos(void)
{
 uint64_t total=0;
    if(currentIndex==0) return 0;
    if(currentIndex>=nbIndex) return length;
    for(int i=0;i<currentIndex;i++)
    {
        total+=index[i].size;
    }
    return total;
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
        if(pos>=total && pos<=total+index[i].size)
        {
            fseeko(fd,index[i].offset,SEEK_SET);
            currentIndex=i;
            return 1;
        }
        total+=index[i].size;
    }
    printf("[aviAudioAccess] Seek to pos %"LLU" failed\n",pos);
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
        fseeko(fd,index[currentIndex].offset,SEEK_SET);
        if(index[currentIndex].size>maxSize) 
        {
            ADM_error("Packet too large %d, maximum is %d\n",index[currentIndex].size,maxSize);
            *size=0;
            return false;
         }
        fread(buffer,1,index[currentIndex].size,fd);
        if(!currentIndex) 
            *dts=0;
        else
            *dts=ADM_AUDIO_NO_DTS;
        *size=index[currentIndex].size;
        currentIndex++;
        return 1;
}
//EOF
