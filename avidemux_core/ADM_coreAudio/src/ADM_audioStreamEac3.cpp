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
        double size=access->getLength();
        size/=header->byterate; // Result is in second
        size*=1000;
        size*=1000; // s->us
        durationInUs=(uint64_t)size;
    }
    lookahead = ADM_AC3_HEADER_SIZE; // Start with the bare minimum and increase later.
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
    lookahead = ADM_AC3_HEADER_SIZE;
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
    return ADM_audioStreamBuffered::goToTime(nbUs);
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
    // In EAC3, an independent frame can be followed by dependent frame(s).
    // A packet is complete when we encounter the next independent frame.
    uint32_t offset;
    bool isPlainAC3, gotFrame = false;
    ADM_EAC3_INFO info;

    while(1)
    {
        if(!gotFrame && !needBytes(lookahead))
            return 0;
        // Search start seq
        if(buffer[start]!=0x0b || buffer[start+1]!=0x77)
        {
            read8();
            continue;
        }
        // Do we have an independent E-AC3 frame in the buffer?
        if(false == ADM_EAC3GetInfo(buffer.at(start), limit-start, &offset, &info, &isPlainAC3)
            || isPlainAC3
            || (info.flags & ADM_EAC3_FLAG_PKT_DAMAGED))
        {
            printf("[EAC3 Stream] Syncing...\n");
            gotFrame = false;
            read8();
            continue;
        }
        gotFrame = true;
        if(!(info.flags & ADM_EAC3_FLAG_PKT_COMPLETE))
        {
            skipBytes(offset); // The start sequence may belong to a dependent frame we need to skip.
            offset = 0;
            lookahead = info.frameSizeInBytes + ADM_AC3_HEADER_SIZE;
            if(needBytes(lookahead))
                continue;
        }
        ADM_assert(info.frameSizeInBytes <= sizeMax);
        if(!needBytes(offset + info.frameSizeInBytes))
            return 0;

        *osize = info.frameSizeInBytes;
        skipBytes(offset);
        read(info.frameSizeInBytes, obuffer);
        *nbSample = info.samples;
        *dts=lastDts;
        advanceDtsBySample(info.samples);
        return 1;
    }
}

// EOF
