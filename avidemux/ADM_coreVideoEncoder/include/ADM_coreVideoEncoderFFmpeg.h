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
#ifndef ADM_CORE_VIDEO_ENCODER_FF_H
#define ADM_CORE_VIDEO_ENCODER_FF_H

#include "ADM_coreVideoEncoder.h"
#include "ADM_colorspace.h"
extern "C" 
{
#include "ADM_lavcodec.h"
}

/**
    \class ADM_coreVideoEncoderFFmpeg
    \brief base class for VideoEncoder based on libavcodec
*/
class ADM_coreVideoEncoderFFmpeg :public ADM_coreVideoEncoder
{
protected:
               AVCodecContext   *_context;      // Contect 
               AVFrame          _frame;     
               ADMColorspace    *colorSpace;    // Colorspace converter if needed
               uint8_t          *rgbBuffer;     // Buffer for colorspace converter if needed
               ADM_colorspace   targetColorSpace; // Wanted colorspace
protected:
    virtual               bool             prolog(void); 
    virtual               bool             preEncode(void); 
    virtual               bool             setup(CodecID codecId);

public:
                            ADM_coreVideoEncoderFFmpeg(ADM_coreVideoFilter *src);
                            ~ADM_coreVideoEncoderFFmpeg();

};
#endif