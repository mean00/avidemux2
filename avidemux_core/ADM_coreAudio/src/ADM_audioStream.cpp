/**
    \file ADM_audioStream
    \brief Base class

*/
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "ADM_audioStreamMP3.h"
#include "ADM_audioStreamAC3.h"
#include "ADM_audioStreamEac3.h"
#include "ADM_audioStreamDCA.h"
#include "ADM_audioStreamPCM.h"
#include "ADM_audioStreamConstantChunk.h"
#include "ADM_audioCodecEnum.h"
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
ADM_audioStream  *ADM_audioCreateStream(WAVHeader *wavheader, ADM_audioAccess *access,bool makeTimeMap)
{
uint8_t *data;
uint32_t size;
    switch(wavheader->encoding)
    {
        case WAV_EAC3:
            return new ADM_audioStreamEAC3(wavheader,access);    
        case WAV_AC3:
            return new ADM_audioStreamAC3(wavheader,access);
        case WAV_MP2:
        case WAV_MP3:
            return new ADM_audioStreamMP3(wavheader,access,makeTimeMap);
        case WAV_PCM:
        case WAV_LPCM:
            return new ADM_audioStreamPCM(wavheader,access);
        case WAV_DTS:
            return new ADM_audioStreamDCA(wavheader,access);
#if 0
        case WAV_WMA:
            return new ADM_audioStreamConstantChunk(wavheader,access);
#endif
        default:
            return new ADM_audioStream(wavheader,access);
    }

}

/**
        \fn getStrFromAudioCodec
        \brief Return a plain string from the codec_id
*/
const char *getStrFromAudioCodec( uint32_t codec)
{
      switch(codec)
      {
              case WAV_DTS: return QT_TR_NOOP("DTS");
              case WAV_PCM: return QT_TR_NOOP("PCM");
              case WAV_MP2: return QT_TR_NOOP("MP2");
              case WAV_MP3: return QT_TR_NOOP("MP3");
              case WAV_WMA:  return QT_TR_NOOP("WMA");
              case WAV_LPCM: return QT_TR_NOOP("LPCM");
              case WAV_AC3:  return QT_TR_NOOP("AC3");
              case WAV_EAC3:  return QT_TR_NOOP("E-AC3");
              case WAV_OGG_VORBIS: return QT_TR_NOOP("Ogg Vorbis");
              case WAV_MP4: return QT_TR_NOOP("MP4");
              case WAV_AAC: return QT_TR_NOOP("AAC");
              case WAV_QDM2: return QT_TR_NOOP("QDM2");
              case WAV_AMRNB: return QT_TR_NOOP("AMR-NB");
              case WAV_MSADPCM: return QT_TR_NOOP("MSADPCM");
              case WAV_ULAW: return QT_TR_NOOP("ULAW");
              case WAV_IMAADPCM: return QT_TR_NOOP("IMA ADPCM");
              case WAV_8BITS_UNSIGNED:return QT_TR_NOOP("8-bit PCM");
      }
      return QT_TR_NOOP("Unknown codec");
}

// EOF

