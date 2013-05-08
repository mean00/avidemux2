/**
    \file  ADM_coreVideoDecoderInternal.h
    \brief interface to video decoder plugins

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef VIDEODECODERINTERNAL_H
#define VIDEODECODERINTERNAL_H

#define ADM_VIDEO_DECODER_API_VERSION 1
#include "ADM_codec.h"

/*!
  This structure defines a video DECODER
  \param DECODER DECODER attached to this descriptor
  \param name The name of the codec
  \param configure Function to call to configure the codec
  \param param : An opaque structure that contains the codec specific configuration datas
*/
typedef struct
{
    const char   *decoderName;      // Internal name (tag)
    const char   *menuName;         // Displayed name (in menu)
    const char   *description;      // Short description

    uint32_t     apiVersion;            // const
    decoders     *(*create)(uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp);
    void         (*destroy)(decoders *codec);
    bool         (*configure)(void);                                // Call UI to set it up
    uint32_t     major,minor,patch;     // Version of the plugin
    uint32_t     *fccs;                 // List of supported fourCCs ending by 0
    void         *opaque;               // Hide stuff in here
}ADM_videoDecoderDesc;

// Macros to declare video decoder
/**************************************************************************/
#define ADM_DECLARE_VIDEO_DECODER_PREAMBLE(Class) \
static decoders * create (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)\
{ \
  Class *r= new Class (w,h,fcc,extraDataLen,extraData,bpp); \
  if(!r->initializedOk()) {delete r;return NULL;} \
  return r;}\
\
static void destroy (decoders * in) \
{\
  Class *z = (Class *) in; \
  delete z; \
} 
//******************************************************

#define ADM_DECLARE_VIDEO_DECODER_MAIN(name,menuName,desc,fcc,configure,maj,minV,patch) \
static ADM_videoDecoderDesc DECODERDesc={\
    name,\
    menuName,\
    desc,\
    ADM_VIDEO_DECODER_API_VERSION,\
    &create,\
    &destroy,\
    configure,\
    maj,minV,patch,\
    fcc,\
    NULL\
};\
extern "C" ADM_videoDecoderDesc *getInfo (void) \
{ \
  return &DECODERDesc; \
}  

#endif
//EOF
