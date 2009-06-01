/**
    \file  ADM_coreVideoEncoderInternal.h
    \brief interface to video encoder plugins

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef VIDEOENCODERINTERNAL_H
#define VIDEOENCODERINTERNAL_H

#define ADM_VIDEO_ENCODER_API_VERSION 1
#include "ADM_coreVideoEncoder.h"
#include "DIA_uiTypes.h"

/*!
  This structure defines a video encoder
  \param encoder Encoder attached to this descriptor
  \param name The name of the codec
  \param configure Function to call to configure the codec
  \param param : An opaque structure that contains the codec specific configuration datas
*/
typedef struct
{
    const char   *encoderName;        // Internal name (tag)
    const char   *menuName;         // Displayed name (in menu)
    const char   *fourCC;
    const char   *description;      // Short description

    uint32_t     apiVersion;            // const
    ADM_coreVideoEncoder *(*create)(ADM_coreVideoFilter *head);  
    void         (*destroy)(ADM_coreVideoEncoder *codec);
    bool         (*configure)(void);                                // Call UI to set it up
    bool         (*getConfigurationData)(uint32_t *l, uint8_t **d); // Get the encoder private conf
    bool         (*setConfigurationData)(uint32_t l, uint8_t *d);   // Set the encoder private conf

    ADM_UI_TYPE  UIType;                // Type of UI
    uint32_t     major,minor,patch;     // Version of the plugin

    void         *opaque;               // Hide stuff in here
}ADM_videoEncoderDesc;

// Macros to declare audio encoder
/**************************************************************************/
#define ADM_DECLARE_VIDEO_ENCODER_PREAMBLE(Class) \
static bool getConfigurationData (uint32_t * l, uint8_t ** d); \
static bool setConfigurationData (uint32_t l, uint8_t * d);\
\
static ADM_coreVideoEncoder * create (ADM_coreVideoFilter * head) \
{ \
  return new Class (head); \
} \
static void destroy (ADM_coreVideoEncoder * in) \
{\
  Class *z = (Class *) in; \
  delete z; \
} 
//******************************************************
#define ADM_DECLARE_VIDEO_ENCODER_CONFIG(configData) \
bool getConfigurationData (uint32_t * l, uint8_t ** d)\
{\
  *l = sizeof (configData); \
  *d = (uint8_t *) & configData; \
  return 1; \
} \
bool setConfigurationData (uint32_t l, uint8_t * d)\
{\
  if (sizeof (configData) != l) \
    {\
      GUI_Error_HIG ("Audio Encoder",\
		     "The configuration size does not match the codec size"); \
      return 0; \
    }\
  memcpy (&configData, d, l); \
  return 1;\
}


// Use that one is the encoder has not configuration

#define ADM_DECLARE_VIDEO_ENCODER_NO_CONFIG() \
bool getConfigurationData (uint32_t * l, uint8_t ** d)\
{\
  *l =0; \
  *d = NULL; \
  return 1; \
} \
bool setConfigurationData (uint32_t l, uint8_t * d)\
{\
  return 1;\
}


#define ADM_DECLARE_VIDEO_ENCODER_MAIN(name,menuName,fourcc,desc,configure,uiType,maj,minV,patch) \
static ADM_videoEncoderDesc encoderDesc={\
    name,\
    menuName,\
    fourcc,\
    desc,\
    ADM_VIDEO_ENCODER_API_VERSION,\
    &create,\
    &destroy,\
    configure,\
    getConfigurationData,\
    setConfigurationData,\
    uiType,\
    maj,minV,patch,\
    NULL\
};\
extern "C" ADM_videoEncoderDesc *getInfo (void) \
{ \
  return &encoderDesc; \
}  \

#ifndef QT_TR_NOOP
#define QT_TR_NOOP(x) x
#endif
#endif
//EOF
