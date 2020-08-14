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
        double size=access->getLength();
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
    return ADM_audioStreamBuffered::goToTime(nbUs);

}
/**
        \fn getPacket
*/
uint8_t ADM_audioStreamDCA::getPacket(uint8_t *obuffer,uint32_t *osize, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts)
{
#define ADM_LOOK_AHEAD DTS_HEADER_SIZE // Need 11 bytes...
    uint8_t data[ADM_LOOK_AHEAD];
    uint32_t offset;
    ADM_DCA_INFO info;

    while(1)
    {
        if(needBytes(ADM_LOOK_AHEAD)==false) 
        {
            ADM_warning("DCA: Not enough data to decode core header\n");
            return 0;
        }
        // Do we have sync ?
        if(false == ADM_DCAGetInfo(buffer.at(start), limit-start, &info, &offset))
        {
            ADM_warning("DCA: No sync within buffer\n");
            skipBytes(limit-start);
            continue;
        }
        ADM_assert(info.frameSizeInBytes<=sizeMax);
        skipBytes(offset);
        if(needBytes(info.frameSizeInBytes)==false)
        {
            ADM_warning("DCA: Not enough data\n");
            return 0;
        }

        uint32_t coreSize=info.frameSizeInBytes;
        *osize=coreSize;
        read(coreSize,obuffer);
        *nbSample=info.samples;
        *dts=lastDts;
        advanceDtsBySample(*nbSample);

        // Check for substream marker, the initial data length may have been too short
        if(false==needBytes(4+ADM_LOOK_AHEAD))
        {
            ADM_warning("DCA: Not enough data to check substream\n");
            return 1;
        }

        if(false==peek(ADM_LOOK_AHEAD,data))
            return 1;

        if(data[0]!=0x7F || data[1]!=0xFE || data[2]!=0x80 || data[3]!=0x01)
        {
            uint32_t align=(coreSize+3)&(~3);
            align-=coreSize;
            start+=align;
            if(false==peek(ADM_LOOK_AHEAD,data))
                return 1;

            if(data[0]!=0x64 || data[1]!=0x58 || data[2]!=0x20 || data[3]!=0x25)
                return 1;

            if(false == ADM_DCAGetInfo(buffer.at(start), limit-start, &info, &offset, true))
            {
                ADM_warning("DCA: Cannot get substream size.\n");
                skipBytes(limit-start);
                return 1;
            }
            if(coreSize >= info.frameSizeInBytes) // should not happen
                return 1;
            // Read the remainder of the frame i.e. the substream
            *osize=info.frameSizeInBytes;
            read(info.frameSizeInBytes-coreSize, obuffer+coreSize);
        }
        return 1;
    }
}

// EOF
