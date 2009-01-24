/**
    \file ADM_audioStream
    \brief Base class

(C) Mean 2008
GPL-v2
*/
#include "ADM_default.h"
#include "ADM_audioStreamMP3.h"
#include "../../ADM_audio/ADM_mp3info.h"

/**
    \fn ADM_audioStreamMP3
    \brief constructor
*/
ADM_audioStreamMP3::ADM_audioStreamMP3(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStreamBuffered(header,access)
{
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
    // and built vbr map if needed
    // The 2 conditions below means there is no gap i.e. avi style stream
    // else not needed
    if(access->isCBR()==false && access->canSeekTime()==false)
    {
        ADM_assert(access->canSeekOffset()==true);
        buildTimeMap();
    }

}

/**
    \fn ADM_audioStream
    \brief destructor
*/
ADM_audioStreamMP3::~ADM_audioStreamMP3()
{
   
}
/**
    \fn goToTime
    \brief goToTime
*/
bool         ADM_audioStreamMP3::goToTime(uint64_t nbUs)
{
    if(access->canSeekTime()==true)
    {
        if( access->goToTime(nbUs)==true)
        {
           lastDts=nbUs;
           limit=start=0;
           refill();
           return 1;
        }
        return 1;
    }
    // If CBR we can use the default way
    if(access->isCBR()==true)
        return ADM_audioStream::goToTime(nbUs);
    // if VBR use our time map
}
/**
        \fn getPacket
*/
uint8_t ADM_audioStreamMP3::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
#define ADM_LOOK_AHEAD 4 // Need 4 bytes...
uint8_t data[ADM_LOOK_AHEAD];
MpegAudioInfo info;
uint32_t offset;
    while(1)
    {
        // Do we have enough ? Refill if needed ?
        if(needBytes(ADM_LOOK_AHEAD)==false) return 0;
        // Peek
        peek(ADM_LOOK_AHEAD,data);
        if(getMpegFrameInfo(data,ADM_LOOK_AHEAD, &info,NULL,&offset))
        {
            ADM_assert(info.size<=sizeMax);
            if(needBytes(info.size)==true)
            {
                *size=info.size;
                read(*size,buffer);
                *nbSample=info.samples;
                advanceDts(*nbSample);
                *dts=lastDts;
                return 1;
            }
            
        }
        //discard one byte
        read8();
    }
}
/**
    \fn buildTimeMap
    \brief Build a timeMap i.e. the table making the relation between offset and time
           it is only used for VBR with offset access, time access does not it it 

*/
bool ADM_audioStreamMP3::buildTimeMap(void)
{
    //access->goToOffset(0);
    

}

// EOF
