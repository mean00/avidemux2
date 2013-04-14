/**
        \file ADM_audioCore.h

*/
#ifndef ADM_audioCore_H
#define ADM_audioCore_H

#include "ADM_coreAudio6_export.h"
#include "ADM_audioCodecEnum.h"
#include "ADM_audiodef.h"
#include "ADM_assert.h"
#include "ADM_byteBuffer.h"

#define MINUS_ONE 0xffffffff
#define DITHER_SIZE 4800
#define DITHER_CHANNELS 6

ADM_COREAUDIO6_EXPORT bool   ADM_audioReorderChannels(uint32_t channels,float *data, uint32_t nb,CHANNEL_TYPE *input,CHANNEL_TYPE *output);
ADM_COREAUDIO6_EXPORT const char *getStrFromAudioCodec( uint32_t codec);
ADM_COREAUDIO6_EXPORT void            AUDMEncoder_initDither();
ADM_COREAUDIO6_EXPORT void dither16(float *start, uint32_t nb, uint8_t channels);
ADM_COREAUDIO6_EXPORT void printWavHeader(WAVHeader *hdr);
#endif
//EOF

