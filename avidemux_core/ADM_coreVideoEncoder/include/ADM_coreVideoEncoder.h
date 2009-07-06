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
#include "ADM_frameType.h"
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
virtual        bool         setup(void) {return true;}    /// Call once before using            
virtual        bool         encode (ADMBitstream * out)=0;

virtual        bool         isDualPass(void) {return false;}
virtual        bool         startPass2(void) {return true;}
virtual        bool         getExtraData(uint32_t *l,uint8_t **d) {*l=0;*d=NULL;return true;}
virtual const  char         *getFourcc(void) =0;
               uint32_t    getWidth(void) {return source->getInfo()->width;}
               uint32_t    getHeight(void) {return source->getInfo()->height;}
               uint32_t    getFrameIncrement(void) {return source->getInfo()->frameIncrement;}
               uint64_t    getTotalDuration(void) {return source->getInfo()->totalDuration;}
virtual        bool        setPassAndLogFile(int pass,const char *name) {return false;}
};
// Spawn a new encoder using the index from the menu = the index in the vector
ADM_coreVideoEncoder *createVideoEncoderFromIndex(ADM_coreVideoFilter *chain,int index);
bool                  videoEncoder6SelectByName(const char *name);
bool                  videoEncoder6Configure(void);
bool                  videoEncoder6_SetCurrentEncoder(uint32_t index);

#endif