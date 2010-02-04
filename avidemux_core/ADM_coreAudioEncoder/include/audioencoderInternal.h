/**
    \file audioencoderInternal.h
    \brief interface to audio encoder plugins

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef AUDIOENCODERINTERNAL_H
#define AUDIOENCODERINTERNAL_H

#define ADM_AUDIO_ENCODER_API_VERSION 4
#include "stddef.h"
#include "audioencoder.h"
#include "ADM_paramList.h"
class AUDMEncoder;
class ADM_AudioEncoder;

/*!
  This structure defines an audio encoder
  \param encoder Encoder attached to this descriptor
   \param name The name of the codec
  \param bitrate The bitrate in kb/s
  \param configure Function to call to configure the codec
  \param maxChannels The maximum # of channels this codec supports
  \param param : An opaque structure that contains the codec specific configuration datas
*/
typedef struct
{
    uint32_t     apiVersion;            // const
    ADM_AudioEncoder *(*create)(AUDMAudioFilter *head, bool globalHeader);
    void         (*destroy)(ADM_AudioEncoder *codec);
    bool         (*configure)(void);    
    const char   *codecName;        // Internal name (tag)
    const char   *menuName;         // Displayed name (in menu)
    const char   *description;
    uint32_t     maxChannels;       // Const
    uint32_t     major,minor,patch;     // Const
    uint32_t     wavTag;                // const Avi fourcc
    uint32_t     priority;              // const Higher means the codec is prefered and should appear first in the list
    bool         (*getConfigurationData)(CONFcouple **conf); // Get the encoder private conf
    bool         (*setConfigurationData)(CONFcouple *conf); // Get the encoder private conf

    uint32_t     (*getBitrate)(void);
    void         (*setBitrate)(uint32_t br);
 
    bool         (*setOption)(const char *paramName, uint32_t value);

    void         *opaque;               // Hide stuff in here
}ADM_audioEncoder;

// Macros to declare audio encoder
/**************************************************************************/
#define ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(Class) \
static bool getConfigurationData (CONFcouple **conf); \
static bool setConfigurationData (CONFcouple *conf);\
static uint32_t     getBitrate(void); \
static void         setBitrate(uint32_t br); \
\
static ADM_AudioEncoder * create (AUDMAudioFilter * head,bool globalHeader) \
{ \
  return new Class (head,globalHeader); \
} \
static void destroy (ADM_AudioEncoder * in) \
{\
  Class *z = (Class *) in; \
  delete z; \
} 
//******************************************************
#define ADM_DECLARE_AUDIO_ENCODER_CONFIG(templ,configData,bitrateVar) \
bool         getConfigurationData(CONFcouple **conf) \
{\
    if(configData==NULL) {*conf=NULL;return true;} \
    return ADM_paramSave(conf,templ,configData); \
} \
bool setConfigurationData (CONFcouple *conf)\
{\
 return ADM_paramLoad(conf,templ,configData); \
}\
\
\
extern "C" ADM_audioEncoder *getInfo (void) \
{ \
  return &encoderDesc; \
}  \
uint32_t     getBitrate(void) {return bitrateVar;};\
void         setBitrate(uint32_t br) {bitrateVar=br;}

#ifndef QT_TR_NOOP
#define QT_TR_NOOP(x) x
#endif
#endif
//EOF
