/**
    \file ADM_audioStreamAC3
    \brief AC3 Handling class

*/
#include "ADM_default.h"
#include "ADM_audioStreamEac3.h"
#include "ADM_eac3info.h"

/**
    \fn ADM_audioStreamAC3
    \brief constructor
*/
ADM_audioStreamEAC3::ADM_audioStreamEAC3(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStreamBuffered(header,access)
{
    if(access->canGetDuration()==false)
    {
        // We can compute the duration from the length
        float size=access->getLength();
              size/=header->byterate; // Result is in second
              size*=1000;
              size*=1000; // s->us
              durationInUs=(uint64_t)size;
    }
}

/**
    \fn ADM_audioStream
    \brief destructor
*/
ADM_audioStreamEAC3::~ADM_audioStreamEAC3()
{
   
}
/**
    \fn goToTime
    \brief goToTime
*/
bool         ADM_audioStreamEAC3::goToTime(uint64_t nbUs)
{
    if(access->canSeekTime()==true)
    {
        if( access->goToTime(nbUs)==true)
        {
           setDts(nbUs);
           limit=start=0;
           refill();
           return 1;
        }
        return 1;
    }
    // If CBR we can use the default way
    return ADM_audioStream::goToTime(nbUs);
    
}
/**
        \fn getPacket
*/
uint8_t ADM_audioStreamEAC3::getPacket(uint8_t *obuffer,
                                       uint32_t *osize, 
                                       uint32_t sizeMax,
                                       uint32_t *nbSample,
                                       uint64_t *dts)
{
#define ADM_LOOK_AHEAD 6 // Need 6 bytes...
uint8_t data[ADM_LOOK_AHEAD];
uint32_t offset;
int size;
ADM_EAC3_INFO info;
    while(1)
    {
        // Do we have sync ?
        if(needBytes(ADM_LOOK_AHEAD)==false) return 0;
        // Peek
        peek(ADM_LOOK_AHEAD,data);
        // Search start seq
        if(buffer[start]!=0x0b || buffer[start+1]!=0x77)
        {
            read8();
            continue;
        }
        if(!ADM_EAC3GetInfo(buffer.at(+start), limit-start, &offset,&info))
        {
            printf("[EAC3 Stream] Syncing...\n");
            read8();
            continue;
        }
        // 
        size= info.frameSizeInBytes;
        ADM_assert(size<=sizeMax);
        if(needBytes(size)==false)
        {
            return false;
        }
        *osize=size;
        read(size,obuffer);
        *nbSample=256*6;
        *dts=lastDts;
        advanceDtsBySample(*nbSample);
        return 1;
    }
}

// EOF
