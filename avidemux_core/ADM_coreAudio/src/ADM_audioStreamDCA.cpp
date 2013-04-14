/**
    \file ADM_audioStreamDCA
    \brief DCA Handling class

*/
#include "ADM_default.h"
#include "ADM_audioStreamDCA.h"
#include "ADM_dcainfo.h"

/**
    \fn ADM_audioStreamDCA
    \brief constructor
*/
ADM_audioStreamDCA::ADM_audioStreamDCA(WAVHeader *header,ADM_audioAccess *access) : ADM_audioStreamBuffered(header,access)
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
ADM_audioStreamDCA::~ADM_audioStreamDCA()
{

}
/**
    \fn goToTime
    \brief goToTime
*/
bool         ADM_audioStreamDCA::goToTime(uint64_t nbUs)
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
uint8_t ADM_audioStreamDCA::getPacket(uint8_t *obuffer,uint32_t *osize, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
#define ADM_LOOK_AHEAD DTS_HEADER_SIZE // Need 10 bytes...
uint8_t data[ADM_LOOK_AHEAD];
uint32_t offset;
ADM_DCA_INFO info;
    while(1)
    {
        // Do we have sync ?
        if(needBytes(ADM_LOOK_AHEAD)==false) 
        {
            ADM_warning("DCA: Not sync found in buffer\n");
            return false;
        }
            
        // Peek
        peek(ADM_LOOK_AHEAD,data);
        // Search start seq
        if(buffer[start]!=0x7F || buffer[start+1]!=0xFE)
        {
            read8();
            continue;
        }
        if(buffer[start+2]!=0x80 || buffer[start+3]!=0x1)
        {
            read8();
            read8();
            continue;
        }

        if(false== ADM_DCAGetInfo(buffer.at(start), limit-start,&info,&offset))
        {
            read8();
            read8();
            read8();
            read8();
            continue;
        }
        ADM_assert(info.frameSizeInBytes<=sizeMax);
        if(needBytes(info.frameSizeInBytes)==false)
        {
            ADM_warning("DCA: Not enough data\n");
            return false;
        }
        *osize=info.frameSizeInBytes;
        read(*osize,obuffer);
        *nbSample=info.samples;
        *dts=lastDts;
        advanceDtsBySample(*nbSample);
        return 1;

    }
}

// EOF
