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

#include "ADM_coreVideoEncoder6_export.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_colorspace.h"
#include "ADM_encoderConf.h"
#include "ADM_coreVideoEncoderFFmpeg_param.h"
#include "FFcodecSettings.h"

/**
    \class ADM_coreVideoEncoderFFmpeg
    \brief base class for VideoEncoder based on libavcodec
*/
class ADM_COREVIDEOENCODER6_EXPORT ADM_coreVideoEncoderFFmpeg :public ADM_coreVideoEncoder
{
protected:
               FFcodecSettings  Settings;
               AVCodecContext   *_context;      // Context 
               AVFrame          *_frame;     
               ADMColorScalerSimple    *colorSpace;    // Colorspace converter if needed
               ADM_byteBuffer   rgbByteBuffer;     // Buffer for colorspace converter if needed
               ADM_colorspace   targetColorSpace; // Wanted colorspace
               bool             loadStatFile(const char *file);
               char             *statFileName;
               FILE             *statFile;
               int              pass;   // Pass number = 1 or 2, valid only if we use 2 pass mode
               bool             _isMT; // True if multithreaded
               bool             _globalHeader;
               double           timeScaler;
               bool             _closeCodec;
               bool             _hasSettings;
              
protected:
                          int              encodeWrapper(AVFrame *in,ADMBitstream *out); // Returns encoded size of <0 for error
    virtual               bool             prolog(ADMImage *img); 
    virtual               bool             preEncode(void); 
    virtual		  bool		   configureContext(void);
    virtual               bool             setup(AVCodecID codecId);
    virtual               bool             setupByName(const char *name);
                          bool             setupInternal(AVCodec *codec);
    virtual               bool             getExtraData(uint32_t *l,uint8_t **d) ;
                          bool             presetContext(FFcodecSettings *set);
                          bool             postEncode(ADMBitstream *out, uint32_t size);
    virtual               bool             setPassAndLogFile(int pass,const char *name); // Call this before setup if needed !
                          bool             setupPass(void);  
                          bool             encoderMT (void);
                          int64_t          timingToLav(uint64_t val);
                          uint64_t         lavToTiming(int64_t val);
public:
                                            ADM_coreVideoEncoderFFmpeg(ADM_coreVideoFilter *src,FFcodecSettings *settings=NULL,bool globalHeader=false);
virtual                                     ~ADM_coreVideoEncoderFFmpeg();

};
#endif
