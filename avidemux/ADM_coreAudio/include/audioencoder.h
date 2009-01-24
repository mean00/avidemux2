/***************************************************************************
    copyright            : (C) 2006 by mean
    email                : fixounet@free.fr
 ***************************************************************************/


#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H
/*!
  This structure defines an audio encoder
  \param encoder Encoder attached to this descriptor
   \param name The name of the codec
  \param bitrate The bitrate in kb/s
  \param configure Function to call to configure the codec
  \param maxChannels The maximum # of channels this codec supports
  \param param : An opaque structure that contains the codec specific configuration datas
*/
#include "ADM_coreAudio.h"
#include "ADM_audioCodecEnum.h"


#define AUDIOENC_COPY 0

class AUDMEncoder;
class AUDMAudioFilter;

typedef int AUDIOENCODER;

/*!
  Base class for all audio encoder.It does the reverse of the bridge class and offers a proper GenericAudioStreamAPI

*/
// // FIXME!!!!
#include "ADM_audioFilter.h" // FIXME!!!
 //_____________________________________________
class AUDMEncoder //: public AVDMGenericAudioStream
{
  protected:
    
    //
    uint32_t grab(uint8_t *outbuffer);
    uint32_t grab(float *outbuffer) {ADM_assert(0);return 1;}
    uint32_t  eof_met;
    uint32_t  _chunk;
    //
    uint8_t         *_extraData;
    uint32_t        _extraSize;
    AUDMAudioFilter *_incoming;
    uint8_t         cleanup(void);
    
    float          *tmpbuffer;
    uint8_t        refillBuffer(int minimum); // Mininum is in float

    
    void reorderChannels(float *data, uint32_t nb,CHANNEL_TYPE *input,CHANNEL_TYPE *output);

    uint32_t       tmphead,tmptail;
    // The encoder can remap the audio channel (or not). If so, let's store the the configuration here
    CHANNEL_TYPE outputChannelMapping[MAX_CHANNELS];
  public:
    //
    WAVHeader       *_wavheader;
    //
    uint32_t read(uint32_t len,uint8_t *buffer);
    uint32_t read(uint32_t len,float *buffer) {ADM_assert(0);return 1;}
    //
    virtual ~AUDMEncoder();
    AUDMEncoder(AUDMAudioFilter *in);	

    virtual uint8_t initialize(void)=0;
    virtual uint8_t getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples)=0;
    virtual uint8_t packetPerFrame( void) {return 1;}
    virtual uint8_t extraData(uint32_t *l,uint8_t **d) {*l=_extraSize;*d=_extraData;return 1;}
            uint8_t  goTo(uint32_t timeMS) {ADM_assert(0);return 1;}
};
// Used by some old code (lame/twolame) OBSOLETE   / DO NOT USE
typedef enum  
{
   	ADM_STEREO=1,
   	ADM_JSTEREO,
   	ADM_MONO
} ADM_mode;

#endif
