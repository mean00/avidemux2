/**
    \file ADM_audioStreamConstantChunk
    \brief Base class

(C) Mean 2008
GPL-v2
*/
#include "ADM_default.h"
#include "ADM_audioStreamConstantChunk.h"
#include "DIA_working.h"
#include "ADM_vidMisc.h"
/**
    \fn ADM_audioStreamConstantChunk
    \brief constructor
*/
ADM_audioStreamConstantChunk::ADM_audioStreamConstantChunk(WAVHeader *header,ADM_audioAccess *access) 
    : ADM_audioStream(header,access)
{
    //
    chunkSize=header->blockalign;
    if(!chunkSize)
    {
        ADM_warning("[ADM_audioStreamConstantChunk] Blockalign is null expect problems\n");
        chunkSize=8192; // dummy value
    }
    ADM_info("[ADM_audioStreamConstantChunk] Chunk size %" PRIu32"\n",chunkSize);
    ADM_info("[ADM_audioStreamConstantChunk] Byterate   %" PRIu32"\n",header->byterate);
    // Compute sample per chunk from wavHeader...
    float f;
    f=chunkSize;
    f/=header->byterate; // F is in seconds
    f*=header->frequency; // in sample
    samplesPerChunk=(uint32_t)f;
    ADM_info("[ADM_audioStreamConstantChunk] About %" PRIu32" samples per chunk\n",samplesPerChunk);
    //samplesPerChunk=16;
    // If hinted..., compute the duration ourselves
    if(access->isCBR()==true && access->canSeekOffset()==true)
    {
        // We can compute the duration from the length
        float size=access->getLength();
              size/=header->byterate; // Result is in second
              size*=1000;
              size*=1000; // s->us
              durationInUs=(uint64_t)size;
              ADM_info("Computed duration %s\n",ADM_us2plain(durationInUs));
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
    *size=0;
    *nbSample=0;
    if(sizeMax>=chunkSize)
    {
        uint32_t mSize;
        uint64_t mDts;
        if(!access->getPacket(buffer,&mSize,sizeMax,&mDts)) 
        {
                ADM_warning("Cant get packet\n");
                return 0;
        }
        ADM_info("Got packet : chunk=%d size=%d dts=%s\n",chunkSize,mSize,ADM_us2plain(mDts));
        if(!*size)
            *dts=mDts;

        *size+=mSize;
        *nbSample+=samplesPerChunk;
        if(mSize!=chunkSize)
        {
            ADM_warning("Expected chunk of size =%d, got %d\n",chunkSize,mSize);
        }

        buffer+=mSize;
        sizeMax-=mSize;
     }
     if(!*size) return 0;
     return 1;
}

