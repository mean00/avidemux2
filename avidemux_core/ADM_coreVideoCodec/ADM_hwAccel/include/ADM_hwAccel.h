/***************************************************************************
         \fn ADM_hwAccel.h
         \brief Top class for ffmpeg hwAccel
         \author mean, fixounet@free.fr (C) 2006
    
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

/**
 * 
 */
class ADM_COREVIDEOCODEC6_EXPORT ADM_acceleratedDecoder
{
public:
                     ADM_acceleratedDecoder();
        virtual      ~ADM_acceleratedDecoder();
        virtual bool uncompress (ADMCompressedImage * in, ADMImage * out);
};

/**
 *  \class ADM_hwAccel
 */
extern "C"
{
class ADM_hwAccelEntry
{
public:    
     const char             *name;
     virtual bool           canSupportThis(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat)=0;
     virtual                ADM_acceleratedDecoder *spawn( void )=0;     
     virtual                ~ADM_hwAccelEntry()= 0;
};
}
/**
 * \fn ADM_hwAccelManager
 */
class ADM_COREVIDEOCODEC6_EXPORT ADM_hwAccelManager
{
public:       
       static bool                registerDecoder(ADM_hwAccelEntry *);
       static ADM_hwAccelEntry    *lookup(struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt);
       static ADM_acceleratedDecoder *spawn( struct AVCodecContext *avctx,  const enum AVPixelFormat *fmt,enum AVPixelFormat &outputFormat );     
};
// EOF
