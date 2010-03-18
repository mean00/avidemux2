/**
    \file ADM_videoProcess.h
    \brief Wrap an encoder as a VideoStream
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_VIDEOPROCESS_H
#define ADM_VIDEOPROCESS_H
#include "ADM_muxer.h"
/**
    \class ADM_videoStream

*/
#include "ADM_compressedImage.h"
#include "ADM_coreVideoEncoder.h"
/**
    \class ADM_videoStreamProcess
    \brief Wrapper around encoder
    @warning After creation, the VideoStream becomes the owner of the encoder and it will deleted here

*/
class ADM_videoStreamProcess: public ADM_videoStream
{
protected:
            
   
            ADM_coreVideoEncoder *encoder;
            uint8_t              *data;
public:
             ADM_videoStreamProcess(ADM_coreVideoEncoder *encoder);
    virtual ~ADM_videoStreamProcess();

virtual     bool     getPacket(ADMBitstream *out);
virtual     bool     getExtraData(uint32_t *extraLen, uint8_t **extraData) ;
virtual     bool     providePts(void) {return true;}
virtual     uint64_t getVideoDuration(void);
virtual     bool     isDualPass(void) { ADM_assert(encoder);return encoder->isDualPass();}
ADM_coreVideoEncoder *getEncoder(void) {return encoder;}
};

#endif
