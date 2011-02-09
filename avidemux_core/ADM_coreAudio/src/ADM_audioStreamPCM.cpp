/**
    \file ADM_audioStreamPCM
    \brief AC3 Handling class

*/
#include "ADM_default.h"
#include "ADM_audioStreamPCM.h"

/**
    \fn ADM_audioStreamAC3
    \brief constructor
*/
ADM_audioStreamPCM::ADM_audioStreamPCM(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStream(header,access)
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
ADM_audioStreamPCM::~ADM_audioStreamPCM()
{
   
}
/**
    \fn goToTime
    \brief goToTime
*/
bool         ADM_audioStreamPCM::goToTime(uint64_t nbUs)
{
    if(access->canSeekTime()==true)
    {
        if( access->goToTime(nbUs)==true)
        {
           setDts(nbUs);
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
uint8_t ADM_audioStreamPCM::getPacket(uint8_t *obuffer,uint32_t *osize, 
                                      uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
uint64_t thisDts=0;
    if(!access->getPacket(obuffer,osize,sizeMax,&thisDts)) return 0;
    uint32_t bytesPerSample=wavHeader.channels*2; 
#warning fixme handle mono
    *nbSample=(uint32_t)(*osize/bytesPerSample);
    if(thisDts!=ADM_NO_PTS) 
            setDts(thisDts);
    *dts=lastDts;
    advanceDtsBySample(*nbSample);
    return 1;
}
// EOF
