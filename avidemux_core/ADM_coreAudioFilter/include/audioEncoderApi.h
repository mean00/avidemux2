/**
    \file audioEncoderApi.h
    \brief Helper functions to deal with audioEncoders

*/
#ifndef AUDIOENCODERAPI_H
#define AUDIOENCODERAPI_H
#include "ADM_confCouple.h"
class AUDMAudioFilter;
class ADM_AudioEncoder;
class EditableAudioTrack;
/// Select the best encoder using the audio fourcc : WAV_MP3 etc...
uint8_t audio_selectCodecByTag(int dex,uint32_t tag);
/// Set an option for example "DISABLERESERVOIR",1
uint8_t audioSetOption(int dex,const char *option, uint32_t value);
/// Select the copy Codec
uint8_t audio_setCopyCodec(int dex);
/// Directly set the codec, *only to be used
uint8_t audioCodecSetByIndex(int dex,int i);
/// Spawn a new encoder
ADM_AudioEncoder *audioEncoderCreate(EditableAudioTrack *ed,AUDMAudioFilter *filter,bool globalHeader);
ADM_AudioEncoder *audioEncoderCreate(int dex,AUDMAudioFilter *filter,bool globalHeader);
/// Select a encoder by its name e.g. "lame", used only by JS. Update UI as well
uint8_t audioCodecSetByName(int dex, const char *name);
/// Returns the name of the currently selected codec
const char *audioCodecGetName( int dex );
/// Update the UI with the currently selected codec
void audioPrintCurrentCodec(int dex);
/// Returns 1 if we are in process mode, 0 if in copy mode
uint32_t audioProcessMode(int dex);
/// Retrieve current audio encoder configuration
bool getAudioExtraConf(int dex,uint32_t *bitrate,CONFcouple **c);
/// Set current audio encoder configuration
bool setAudioExtraConf(int dex,CONFcouple *c);



#endif
// EOF
