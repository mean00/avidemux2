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
#include "opus/opus_multistream.h"
/**
 */
class ADM_AudiocodecOpus : public     ADM_Audiocodec
{
protected:
    OpusDecoder     *opus_handle;
    OpusMSDecoder   *opus_multistream_handle;

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

#define OPUS_HEADER_SIZE 19

ADM_AudiocodecOpus::ADM_AudiocodecOpus(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d) : 
  ADM_Audiocodec(fourcc,*info)
{
    ADM_assert(fourcc==WAV_OPUS);
    opus_handle=NULL;
    opus_multistream_handle=NULL;
    int er,nbStreams,nbCoupled;
    uint8_t *mapping;
    if(info->channels>2)
    {
        if(l>=OPUS_HEADER_SIZE+2+info->channels)
        {
            nbStreams=d[OPUS_HEADER_SIZE];
            nbCoupled=d[OPUS_HEADER_SIZE+1];
            if((nbStreams+nbCoupled) != info->channels)
                ADM_warning("Inconsistent channel mapping: %d streams, %d coupled, but %d channels.\n",nbStreams,nbCoupled,info->channels);
            mapping=d+OPUS_HEADER_SIZE+2;
        }else
        {
            ADM_error("We have more than 2 channels, but not enough opus extradata (%d), crashing.\n",l);
            ADM_assert(0);
        }
        opus_multistream_handle=opus_multistream_decoder_create(info->frequency,info->channels,nbStreams,nbCoupled,mapping,&er);
        if(opus_multistream_handle)
            ADM_info("Created opus decoder for %d streams (%d coupled), %d channels, mapping = %d\n",nbStreams,nbCoupled,info->channels,(int)(*mapping));
    }else
    {
        opus_handle=opus_decoder_create(info->frequency,info->channels,&er);
    }
    if(!opus_handle && !opus_multistream_handle)
    {
        ADM_error("Cannot init libopus, error=%d\n",er);
        ADM_assert(0);   
    }
    CHANNEL_TYPE *p_ch_type = channelMapping;
#define DOIT(y) *(p_ch_type++)=ADM_CH_##y;
    if(info->channels>=5)
    {
        DOIT(FRONT_LEFT)
        DOIT(FRONT_CENTER)
        DOIT(FRONT_RIGHT)
        DOIT(REAR_LEFT)
        DOIT(REAR_RIGHT)
        DOIT(LFE)
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
    if(opus_multistream_handle)
    {
        opus_multistream_decoder_destroy(opus_multistream_handle);
        opus_multistream_handle=NULL;
    }
}

/**
 */
uint8_t ADM_AudiocodecOpus::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
    *nbOut=0;
    int err;
    const int framesize=5760;

    if(opus_handle)
        err=opus_decode_float(opus_handle,inptr,nbIn,outptr,framesize,false);
    else
        err=opus_multistream_decode_float(opus_multistream_handle,inptr,nbIn,outptr,framesize,false);
    //ADM_info("Incoming = %d bytes, out samples=%d\n",nbIn,err);
    if(err>0)
    {
        *nbOut=err*wavHeader.channels;
        return 1;
    }
    return 0; 
}

