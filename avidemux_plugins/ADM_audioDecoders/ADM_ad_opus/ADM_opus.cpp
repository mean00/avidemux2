/***************************************************************************
                          ADM_codecac3.cpp  -  description
                             -------------------
    begin                : Fri May 31 2002
    copyright            : (C) 2002 by mean
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
#include <math.h>
#include "ADM_default.h"
#include "ADM_ad_plugin.h"
#include "opus/opus.h"
/**
 */
class ADM_AudiocodecOpus : public     ADM_Audiocodec
{
	protected:
		OpusDecoder     *opus_handle;

	public:
		ADM_AudiocodecOpus(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
		virtual	~ADM_AudiocodecOpus();
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
		virtual	uint8_t isDecompressable(void) {return 1;}

   };

// Supported formats + declare our plugin
//*******************************************************
 static  ad_supportedFormat Formats[]={{WAV_OPUS,AD_HIGH_QUAL}};
DECLARE_AUDIO_DECODER(ADM_AudiocodecOpus,						// Class
			0,0,1, 												// Major, minor,patch 
			Formats, 											// Supported formats

			"libOpus decoder plugin for avidemux (c) Mean\n"); 	// Desc
   //********************************************************
ADM_AudiocodecOpus::ADM_AudiocodecOpus(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d) : 
  ADM_Audiocodec(fourcc,*info)
{
    int flags=0;
    ADM_assert(fourcc==WAV_OPUS);
    opus_handle=NULL;
    int er;
    opus_handle=opus_decoder_create(info->frequency,info->channels,&er);
    if(!opus_handle)
    {
        ADM_warning("Cannot init libopus, error=%d \n");
        ADM_assert(0);   
    }
}
/**
 */
ADM_AudiocodecOpus::~ADM_AudiocodecOpus( )
{
    if(opus_handle)
    {
        opus_decoder_destroy(opus_handle);
        opus_handle=NULL;
    }
}

/**
 */
uint8_t ADM_AudiocodecOpus::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
    uint32_t avail;
    uint32_t length,syncoff;
    int flags = 0, samprate = 0, bitrate = 0, frame_length;
    uint8_t chan = wavHeader.channels;
    *nbOut=0;


    int err=opus_decode_float(opus_handle,inptr,nbIn,outptr,5760,false); //??
    //ADM_info("Incoming = %d bytes, out samples=%d\n",nbIn,err);
    if(err>0)
    {
        *nbOut=err*wavHeader.channels;
        return 1;
    }
    return 0; 
}

