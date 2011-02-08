/***************************************************************************
  \file ADM_aacadts.cpp
  \brief wrapper around libavcodec bitstream filter
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
extern "C" {
#include "libavcodec/avcodec.h"
}
#include "ADM_aacadts.h"
#define COOKIE   ((AVBitStreamFilterContext *)cookie)
#define CONTEXT  ((AVCodecContext *)codec)

static const int aacChannels[16]=
{
0, //0: Defined in AOT Specifc Config
1, //1: 1 channel: front-center
2, //2: 2 channels: front-left, front-right
3, //3: 3 channels: front-center, front-left, front-right
4, //4: 4 channels: front-center, front-left, front-right, back-center
5, // 5: 5 channels: front-center, front-left, front-right, back-left, back-right
6, // 6: 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel
8, // 7: 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel
0,0,0,0,
0,0,0,0,
};

static 	uint32_t aacSampleRate[16]=
{
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025,  8000,
	0,     0,     0,     0 
};
/**
    \fn getExtraData
    \brief extract extradata from adts/aac stream. You must have converted successfully at leat one frame.
*/
bool ADM_adts2aac::getExtraData(uint32_t *len,uint8_t **data)
{
    *data=CONTEXT->extradata;
    *len=CONTEXT->extradata_size;
    return true;
}
/**
    \fn getFrequency
    \brief get stream frequency. Convert must have been called ok once.
*/
int ADM_adts2aac::getFrequency(void)
{
    if(!(CONTEXT->extradata))
    {
            ADM_error("No extradata in aac! using default of 48 kHz");
            return 48000;
    }
    uint8_t *p=CONTEXT->extradata ;
    int dex=((p[0]&7)<<1)+(p[1]>>7);
    return aacSampleRate[dex];
}
/**
    \fn getChannels 
    \brief returns # of channels. Convert must have been called ok once.
*/
int ADM_adts2aac::getChannels(void)
{
    if(!(CONTEXT->extradata))
    {
            ADM_error("No extradata in aac! using default of 2 channels");
            return 2;
    }
    uint8_t *p=CONTEXT->extradata ;
    int dex=((p[1]>>3)&0xf);
    return aacChannels[dex];
}

/**
    \fn convert
    \brief strip adts header. Out can be null if you just want to get headers
*/

bool ADM_adts2aac::convert(int incomingLen,uint8_t *inData,int *outLen,uint8_t *out)
{
    uint8_t *o;
    *outLen=0;
    av_bitstream_filter_filter(COOKIE,CONTEXT,NULL,
                            &o,outLen,inData,incomingLen,0);
    if(!*outLen)
        return false;
    if(out)
        memcpy(out,o,*outLen);
    //printf("In : %d out %d\n",incomingLen,*outLen);
    return true;
}
/**
    \fn ctor
*/

ADM_adts2aac::ADM_adts2aac(void)
{
    AVBitStreamFilterContext *bsfc= av_bitstream_filter_init("aac_adtstoasc");
    if (!bsfc) 
    {
        ADM_error("Cannot get adts filter...\n");
        ADM_assert(0);
    }
    cookie=(void *)bsfc;
    //****
    AVCodecContext *context = avcodec_alloc_context ();
    AVCodec *dec=avcodec_find_decoder(CODEC_ID_AAC);
    if (avcodec_open(context, dec) < 0)
    {
        ADM_error("Cannot get aac codec...\n");
        ADM_assert(0);
    }
    codec=(void *)context;
}
/**
    \fn dtor
*/

ADM_adts2aac::~ADM_adts2aac()
{
    if(cookie)
    {
        av_bitstream_filter_close(COOKIE);  
        cookie=NULL;
    }
    if(codec)
    {
        av_free(CONTEXT);
        codec=NULL;
    }
}

//EOF