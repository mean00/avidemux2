/**
    \file ADM_audioStreamBuffered
    \brief Byte oriented audioStream class

*/
#include "ADM_default.h"
#include "ADM_audioStreamBuffered.h"
#include "ADM_vidMisc.h"

#ifdef _MSC_VER
#define abs(x) _abs64(x)
#endif

#define ADM_MAX_SKEW 40000

/**
    \fn ADM_audioStreamBuffered
    \brief constructor
*/
ADM_audioStreamBuffered::ADM_audioStreamBuffered(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStream(header,access)
{
    limit=0;
    start=0;
    buffer.setSize(2*ADM_AUDIOSTREAM_BUFFER_SIZE);
}
/**
        \fn refill
*/
bool ADM_audioStreamBuffered::refill(void)
{
        int nbTry=0;
#define MAX_TRIES 50 // ~ 1 sec
again:
        // Shrink buffer...
        if(limit>ADM_AUDIOSTREAM_BUFFER_SIZE && start> 10*1024)
        {
            //printf("[Shrink]\n");
            memmove(buffer.at(0), buffer.at(start),limit-start);
            limit-=start;
            start=0;
        }
        uint64_t newDts;
        uint32_t size;
        ADM_assert(limit<(2*ADM_AUDIOSTREAM_BUFFER_SIZE-16));
        uint32_t toRead=2*ADM_AUDIOSTREAM_BUFFER_SIZE-limit-16;
        if(true!=access->getPacket(buffer.at(limit), &size, toRead,&newDts))
                return false;
        // We introduce a small error as there might be some bytes left in the buffer
        // By construction, the error should be minimal
        if(newDts!=ADM_AUDIO_NO_DTS)
        {
            if( labs((int64_t)newDts-(int64_t)lastDts)>ADM_MAX_SKEW)
            {
                if(newDts<lastDts || newDts>(lastDts+60LL*1000000LL)) // If the jump is absurd we ignore it
                {
                        nbTry++;
                        if(nbTry<MAX_TRIES)
                        {
                            ADM_warning("Trying to ignore the discontinuous timestamp (%d try)\n",nbTry);
                            goto again;
                        }
                }
                uint64_t delta=labs((int64_t)newDts-(int64_t)lastDts);
                const char *sign="+";
                if(newDts<lastDts) sign="-";
                printf("[AudioStream] Warning skew in dts =%s %lu \n",sign,delta);
                printf("[AudioStream] Warning skew lastDts=%s \n",ADM_us2plain(lastDts));
                printf("[AudioStream] Warning skew newDts=%s  \n",ADM_us2plain(newDts));
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
        memcpy(d,buffer.at(start),n);
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
        memcpy(d,buffer.at(start),n);
        return true;
}
/**
        \fn goToTime
*/
bool      ADM_audioStreamBuffered::goToTime(uint64_t nbUs)
{
    limit=start=0;
    if(true==ADM_audioStream::goToTime(nbUs))
    {
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
