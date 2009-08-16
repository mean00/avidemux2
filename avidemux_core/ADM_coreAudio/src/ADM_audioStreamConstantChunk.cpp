/**
    \file ADM_audioStreamConstantChunk
    \brief Base class

(C) Mean 2008
GPL-v2
*/
#include "ADM_default.h"
#include "ADM_audioStreamConstantChunk.h"
#include "DIA_working.h"
/**
    \fn ADM_audioStreamConstantChunk
    \brief constructor
*/
ADM_audioStreamConstantChunk::ADM_audioStreamConstantChunk(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStreamBuffered(header,access)
{
    //
    chunkSize=header->blockalign;
    if(!chunkSize)
    {
        printf("[ADM_audioStreamConstantChunk] Blockalign is null expect problems\n");
        chunkSize=8192; // dummy value
    }
    printf("[ADM_audioStreamConstantChunk] Chunk size %"LU"\n",chunkSize);
    printf("[ADM_audioStreamConstantChunk] Byterate   %"LU"\n",header->byterate);
    // Compute sample per chunk from wavHeader...
    float f;
    f=chunkSize;
    f/=header->byterate; // F is in seconds
    f*=header->frequency; // in sample
    samplesPerChunk=(uint32_t)f;
    printf("[ADM_audioStreamConstantChunk] About %"LU" samples per chunk\n",samplesPerChunk);

    // If hinted..., compute the duration ourselves
    if(access->isCBR()==true && access->canSeekOffset()==true)
    {
        // We can compute the duration from the length
        float size=access->getLength();
              size/=header->byterate; // Result is in second
              size*=1000;
              size*=1000; // s->us
              durationInUs=(uint64_t)size;
              return;
    }
// Time based
    durationInUs=access->getDurationInUs();
    

}

/**
    \fn ADM_audioStream
    \brief destructor
*/
ADM_audioStreamConstantChunk::~ADM_audioStreamConstantChunk()
{
  
}

/**
        \fn getPacket
*/
uint8_t ADM_audioStreamConstantChunk::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{

        // Do we have enough ? Refill if needed ?
        if(needBytes(chunkSize)==false) return 0;
        if(sizeMax<chunkSize)
        {
            printf("[ADM_audioStreamConstantChunk] Buffer too small %"LU", need %"LU"\n",sizeMax,chunkSize);
            return 0;
        }
        *size=chunkSize;
        read(chunkSize,buffer);
        start+=chunkSize;
        *nbSample=samplesPerChunk;
        *dts=lastDts;
        advanceDtsBySample(samplesPerChunk);
        return 1;
            
}

