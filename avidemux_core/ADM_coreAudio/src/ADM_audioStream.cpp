/**
    \file ADM_audioStream
    \brief Base class

*/
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "ADM_audioStreamMP3.h"
#include "ADM_audioStreamAC3.h"
#include "ADM_audioStreamEac3.h"
#include "ADM_audioStreamConstantChunk.h"

/**
    \fn ADM_audioStream
    \brief constructor
*/
ADM_audioStream::ADM_audioStream(WAVHeader *header,ADM_audioAccess *access)
{
    if(header)
        wavHeader=*header;
    else    
        memset(&wavHeader,0,sizeof(wavHeader));
    this->access=access;
    lastDts=ADM_AUDIO_NO_DTS;
    lastDtsBase=0;
    sampleElapsed=0;
    if(access)
        if(access->canGetDuration()==true)
                durationInUs=access->getDurationInUs();
        else    
                durationInUs=0;
}
/**
    \fn goToTime
    \brief goToTime
*/
bool  ADM_audioStream::goToTime(uint64_t nbUs)
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
    ADM_assert(true==access->canSeekOffset());
    // Convert time to offset in bytes
    float f=nbUs*wavHeader.byterate;
    f/=1000;
    f/=1000; // in bytes
    if(access->setPos( (uint32_t)(f+0.5)))
    {
        // The seek might not be accurate, recompute the Dts
        // it is better to undershoot in most case
        uint64_t pos=access->getPos();
        // compute dts from pos & byterate
        float r=pos;
            r*=1000*1000;
            r/=wavHeader.byterate;
            setDts(r);
        return 1;
    }
    return false;
}
/**
        \fn getPacket
*/
uint8_t ADM_audioStream::getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *odts)
{
uint64_t dts=0;
    if(!access->getPacket(buffer,size,sizeMax,&dts)) return 0;
    // We got the packet
    // Try to guess the nbSample
    if(dts==ADM_AUDIO_NO_DTS)
    {
        if(wavHeader.encoding==WAV_AAC) 
            *nbSample=1024;
        else        
        {
            *nbSample=512;
            printf("[audioStream] Cant guess nb sample, setting 512\n");
        }
        *odts=ADM_AUDIO_NO_DTS;
        return 1;
    }
    //printf("[ADM_audioStream::get Packet : Size %u dts:%lu\n",size,dts);
    float f=dts-lastDts;
    f*=wavHeader.frequency;
    f/=1000;
    f/=1000;
    setDts(dts);
    *nbSample=(uint32_t)(f+0.5);
    *odts=dts;
    return 1;
}
/**
        \fn getExtraData
*/

bool         ADM_audioStream::getExtraData(uint32_t *l, uint8_t **d)
{
    return access->getExtraData(l,d);
}
/**
    \fn setDts
    \brief set a new Dts
*/
void  ADM_audioStream::setDts(uint64_t newDts)
{

    lastDts=newDts;
    sampleElapsed=0;
    lastDtsBase=newDts;
}

/**
        \fn advanceDtsBySample
*/
bool    ADM_audioStream::advanceDtsBySample(uint32_t samples)
{
        sampleElapsed+=samples;
        float f=sampleElapsed*1000;
            f/=wavHeader.frequency;
            f*=1000;
            lastDts=lastDtsBase+(uint64_t)(f+0.5);
        return true;
}
/**
                Create the appropriate audio stream
*/
ADM_audioStream  *ADM_audioCreateStream(WAVHeader *wavheader, ADM_audioAccess *access)
{
uint8_t *data;
uint32_t size;
    switch(wavheader->encoding)
    {
        case WAV_EAC3:
            return new ADM_audioStreamEAC3(wavheader,access);    
        case WAV_AC3:
            return new ADM_audioStreamAC3(wavheader,access);
        case WAV_MP3:
            return new ADM_audioStreamMP3(wavheader,access);
#if 0
        case WAV_WMA:
            return new ADM_audioStreamConstantChunk(wavheader,access);
#endif
        default:
            return new ADM_audioStream(wavheader,access);
    }

}
// EOF

