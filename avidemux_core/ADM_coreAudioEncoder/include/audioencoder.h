/***************************************************************************
  \file audioencoder.cpp

    copyright            : (C) 2002-6 by mean/gruntster/Mihail 
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H

#include "ADM_coreAudioEncoder6_export.h"
#include "ADM_coreAudio.h"
#include "ADM_audioCodecEnum.h"
#include "ADM_audioFilter.h" 
#include "ADM_confCouple.h"
#include "ADM_byteBuffer.h"
#define AUDIOENC_COPY 0


typedef int AUDIOENCODER;
typedef enum
{
    AudioEncoderRunning,
    AudioEncoderNoInput,
    AudioEncoderStopped
}AudioEncoderState;
/**
    \class AUDMEncoder
    \brief audio encoder base class. Combined with the audioaccess class it makes the exact opposite
            of the bridge class, i.e. convert audioFilter to ADM_access then ADM_stream.

*/
#define ADM_AUDIO_ENCODER_BUFFER_SIZE (6*32*1024)
class ADM_COREAUDIOENCODER6_EXPORT ADM_AudioEncoder 
{
  protected:

    AudioEncoderState _state;    // True if cannot encode anymore
    //
    uint8_t         *_extraData;
    uint32_t        _extraSize;
    AUDMAudioFilter *_incoming;

    
    ADM_floatBuffer tmpbuffer;  // incoming samples are stored here before encoding
    uint32_t        tmphead,tmptail;

    bool            refillBuffer(int minimum); // Mininum is in float
      
    bool            reorder(float *sample_in,float *sample_out,int samplePerChannel,CHANNEL_TYPE *mapIn,CHANNEL_TYPE *mapOut);
    bool            reorderToPlanar(float *sample_in,float *sample_out,int samplePerChannel,CHANNEL_TYPE *mapIn,CHANNEL_TYPE *mapOut);
    bool            reorderToPlanar2(float *sample_in,float **sample_out,int samplePerChannel,CHANNEL_TYPE *mapIn,CHANNEL_TYPE *mapOut);
    // The encoder can remap the audio channel (or not). If so, let's store the the configuration here
    CHANNEL_TYPE    outputChannelMapping[MAX_CHANNELS];
    WAVHeader       wavheader;  /// To be filled by the encoder, especially byterate and codec Id.
  public:
    //
                    ADM_AudioEncoder(AUDMAudioFilter *in, CONFcouple *setup);	
                    virtual ~ADM_AudioEncoder();

    virtual uint8_t extraData(uint32_t *l,uint8_t **d) {*l=_extraSize;*d=_extraData;return 1;}
    WAVHeader       *getInfo(void) {return &wavheader;}
    virtual bool    isVBR(void) {return true;}
    virtual bool    initialize(void)=0; /// Returns true if init ok, false if encoding is impossible
    virtual bool    encode(uint8_t *dest, uint32_t *len, uint32_t *samples)=0; /// returns false if eof met
    virtual bool    provideAccurateSample(void) {return true;} /// Some encoder does not provide samples, in that case
                                                                /// Return false, but the matching parser must exist!
            const std::string &getLanguage(void);
};
#endif
