/**
    \file audioencoderInternal.h
    \brief interface to audio encoder plugins

*/
#ifndef AUDIOENCODERINTERNAL_H
#define AUDIOENCODERINTERNAL_H

#define ADM_AUDIO_ENCODER_API_VERSION 1
#include "audioencoder.h"
class AUDMEncoder;
class AUDMAudioFilter;


typedef struct
{
    uint32_t     apiVersion;            // const
    AUDMEncoder *(*create)(AUDMAudioFilter *head);  
    void         (*destroy)(AUDMEncoder *codec);
    uint8_t      (*configure)(void);    
    const char   *codecName;        // Internal name (tag)
    const char   *menuName;         // Displayed name (in menu)
    const char   *description;
    uint32_t     maxChannels;       // Const
    uint32_t     major,minor,patch;     // Const
    uint32_t     wavTag;                // const Avi fourcc
    uint32_t     priority;              // const Higher means the codec is prefered and should appear first in the list
    uint8_t      (*getConfigurationData)(uint32_t *l, uint8_t **d); // Get the encoder private conf
    uint8_t      (*setConfigurationData)(uint32_t l, uint8_t *d); // Get the encoder private conf

    uint32_t     (*getBitrate)(void);
    void         (*setBitrate)(uint32_t br);
 
    uint8_t      (*setOption)(const char *paramName, uint32_t value);

    void         *opaque;               // Hide stuff in here
}ADM_audioEncoder;

// Macros to declare audio encoder
/**************************************************************************/
#define ADM_DECLARE_AUDIO_ENCODER_PREAMBLE(Class) \
static uint8_t getConfigurationData (uint32_t * l, uint8_t ** d); \
static uint8_t setConfigurationData (uint32_t l, uint8_t * d);\
static uint32_t     getBitrate(void); \
static void         setBitrate(uint32_t br); \
\
static AUDMEncoder * create (AUDMAudioFilter * head) \
{ \
  return new Class (head); \
} \
static void destroy (AUDMEncoder * in) \
{\
  Class *z = (Class *) in; \
  delete z; \
} 
//******************************************************
#define ADM_DECLARE_AUDIO_ENCODER_CONFIG(configData) \
uint8_t getConfigurationData (uint32_t * l, uint8_t ** d)\
{\
  *l = sizeof (configData); \
  *d = (uint8_t *) & configData; \
  return 1; \
} \
uint8_t setConfigurationData (uint32_t l, uint8_t * d)\
{\
  if (sizeof (configData) != l) \
    {\
      GUI_Error_HIG ("Audio Encoder",\
		     "The configuration size does not match the codec size"); \
      return 0; \
    }\
  memcpy (&configData, d, l); \
  return 1;\
}\
\
\
extern "C" ADM_audioEncoder *getInfo (void) \
{ \
  return &encoderDesc; \
}  \
uint32_t     getBitrate(void) {return configData.bitrate;};\
void         setBitrate(uint32_t br) {configData.bitrate=br;}

#ifndef QT_TR_NOOP
#define QT_TR_NOOP(x) x
#endif
#endif
//EOF
