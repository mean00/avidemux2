/**
    \file ADM_audioStreamBuffered
    \brief Byte oriented audioStream class

*/
#include "ADM_default.h"
#include "ADM_audioStreamBuffered.h"

#define ADM_MAX_SKEW 40000

/**
    \fn ADM_audioStreamBuffered
    \brief constructor
*/
ADM_audioStreamBuffered::ADM_audioStreamBuffered(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStream(header,access)
{
    limit=0;
    start=0;
}
/**
        \fn refill
*/
bool ADM_audioStreamBuffered::refill(void)
{
        // Shrink buffer...
        if(limit>ADM_AUDIOSTREAM_BUFFER_SIZE && start> 10*1024)
        {
            //printf("[Shrink]\n");
            memmove(buffer, buffer+start,limit-start);
            limit-=start;
            start=0;
        }
        uint64_t newDts;
        uint32_t size;
        ADM_assert(limit<(2*ADM_AUDIOSTREAM_BUFFER_SIZE-16));
        uint32_t toRead=2*ADM_AUDIOSTREAM_BUFFER_SIZE-limit-16;
        if(true!=access->getPacket(buffer+limit, &size, toRead,&newDts))
                return false;
        // We introduce a small error as there might be some bytes left in the buffer
        // By construction, the error should be minimal
        if(newDts!=ADM_AUDIO_NO_DTS)
        {
            if( abs(newDts-lastDts)>ADM_MAX_SKEW)
            {
                printf("[AudioStream] Warning skew in dts %"LLD"\n",(int64_t)newDts-(uint64_t)lastDts);
                setDts(newDts);
            }
            // If we have a DTS and the buffer is empty, set the dts inconditionnaly
            if(!start) setDts(newDts); // Fixme allow a bit of error, not accumulating
        }
        limit+=size;
        ADM_assert(limit<ADM_AUDIOSTREAM_BUFFER_SIZE*2);
        return true;
}
/**
    \fn read8
*/
uint32_t   ADM_audioStreamBuffered::read8()
{
    ADM_assert(start!=limit);
    return buffer[start++];
}
/**
    \fn read16
*/
uint32_t   ADM_audioStreamBuffered::read16()
{
uint32_t r;
    ADM_assert(start+1<limit);
    r=(buffer[start]<<8)+buffer[start+1];
    start+=2;
    return r;
}
/**
    \fn read32
*/
uint32_t   ADM_audioStreamBuffered::read32()
{
uint32_t r;
    ADM_assert(start+3<limit);
    r=(buffer[start]<<24)+(buffer[start+1]<<16)+(buffer[start+2]<<8)+buffer[start+3];
    start+=4;
    return r;
}
/**
        \fn read

*/
bool      ADM_audioStreamBuffered::read(uint32_t n,uint8_t *d)
{
        if(start+n>limit) refill();
        if(start+n>limit) return false;
        memcpy(d,buffer+start,n);
        start+=n;
        return true;
}
/**
        \fn peek

*/
bool      ADM_audioStreamBuffered::peek(uint32_t n,uint8_t *d)
{
        if(start+n>=limit) refill();
        if(start+n>=limit) return false;
        memcpy(d,buffer+start,n);
        return true;
}
/**
        \fn goToTime
*/
bool      ADM_audioStreamBuffered::goToTime(uint64_t nbUs)
{
    if(true==ADM_audioStream::goToTime(nbUs))
    {
        limit=start=0;
        return true;
    }
    return false;
}
/**
    \fn needBytes
*/
bool      ADM_audioStreamBuffered::needBytes(uint32_t nbBytes)
{
    while(1)
    {
        if((limit-start)>=nbBytes) return true;
        if(false==refill()) return false;
    }
}
/**
    \fn skipBytes
*/
bool      ADM_audioStreamBuffered::skipBytes(uint32_t nbBytes)
{
    if((limit-start)>=nbBytes)
    {
        start+=nbBytes;
        return true;
    }
    return false;
}
// EOF
