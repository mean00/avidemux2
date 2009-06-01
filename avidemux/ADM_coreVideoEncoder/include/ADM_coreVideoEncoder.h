/***************************************************************************
                          \fn ADM_coreVideoEncoder
                          \brief Base class for video encoder plugin
                             -------------------
    
    copyright            : (C) 2002/2009 by mean
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
#ifndef ADM_CORE_VIDEO_ENCODER_H
#define ADM_CORE_VIDEO_ENCODER_H

#include "ADM_coreVideoFilter.h"
#include "ADM_bitstream.h"

typedef enum
{
    ADM_ENCODER_OPTION_MOV_MODE=1

}ADM_coreEncoderOption;

/**
    \class ADM_coreVideoEncoder
    \brief base class for VideoEncoder
*/
class ADM_coreVideoEncoder
{
protected:
                            ADM_coreVideoFilter *source;
                            ADMImage            *image;
public:
                            ADM_coreVideoEncoder(ADM_coreVideoFilter *src);
                            ~ADM_coreVideoEncoder();
virtual        bool         encode (ADMBitstream * out)=0;

virtual        bool         setOption(ADM_coreEncoderOption option, int32_t val) {return true;} /// Set external header mode (ESDS atom & friends)
virtual        bool         setFreeOption(const char *option, void *data) {return true;} /// Set external header mode (ESDS atom & friends)


virtual        bool         isDualPass(void) {return false;}
virtual        bool         startPass2(void) {return true;}
virtual        bool         getExtraData(uint32_t *l,uint8_t **d) {*l=0;*d=NULL;return true;}

virtual        const char *getDisplayName(void)=0;       /// E.g. FFmpeg mpeg4      
virtual        const char *getCodecName(void)=0;         /// aka fourcc
virtual        const char *getFCCHandler(void)=0;        /// fourcc 2, needed to build AVI header

virtual        bool        configure(void)=0;           /// Pop-up UI
virtual        bool        getConfiguration(uint32_t *l,uint8_t **d)=0; /// Get current conf to save it
virtual        bool        setConfiguration(uint32_t l,uint8_t *d)=0;   /// Set conf, call it just after creation
               uint32_t    getWidth(void) {return source->getInfo()->width;}
               uint32_t    getHeight(void) {return source->getInfo()->height;}
               uint32_t    getFrameIncrement(void) {return source->getInfo()->frameIncrement;}
               uint64_t    getTotalDuration(void) {return source->getInfo()->totalDuration;}
};

#endif