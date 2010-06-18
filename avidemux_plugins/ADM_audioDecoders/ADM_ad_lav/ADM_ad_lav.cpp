/**
    \file ADM_ad_lav.cpp
    \brief Audio decoders built around libavcodec
    \author mean (c) 2009

*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_lavcodec.h"
#include "ADM_default.h"
#include "ADM_ad_plugin.h"
#include "fourcc.h"

#define ADMWA_BUF (4*1024*16) // 64 kB internal
class ADM_AudiocodecWMA : public     ADM_Audiocodec
{
	protected:
		void *_contextVoid;
		uint8_t _buffer[ ADMWA_BUF];
		uint32_t _tail,_head;
		uint32_t _blockalign;
        uint32_t channels;

	public:
		ADM_AudiocodecWMA(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
		virtual	~ADM_AudiocodecWMA() ;
		virtual	uint8_t beginDecompress(void);
		virtual	uint8_t endDecompress(void);
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
};




// Supported formats + declare our plugin
//*******************************************************

static  ad_supportedFormat Formats[]={
        {WAV_WMA,AD_MEDIUM_QUAL},
        {WAV_QDM2,AD_MEDIUM_QUAL},
        {WAV_AMV_ADPCM,AD_MEDIUM_QUAL},
        {WAV_NELLYMOSER,AD_MEDIUM_QUAL},
        {WAV_DTS,AD_MEDIUM_QUAL},
        {WAV_MP3,AD_MEDIUM_QUAL},
        {WAV_MP2,AD_MEDIUM_QUAL},
        {WAV_AC3,AD_LOW_QUAL},   // liba52 preferred ???
        {WAV_AAC,AD_LOW_QUAL},   // liba52 preferred ???
        {0x706D,AD_LOW_QUAL}, 
        {WAV_EAC3,AD_MEDIUM_QUAL}
  
};

DECLARE_AUDIO_DECODER(ADM_AudiocodecWMA,						// Class
			0,0,1, 							       // Major, minor,patch
			Formats, 							// Supported formats
			"Lavcodec decoder plugin for avidemux (c) Mean/Gruntster\n"); 	// Desc
//********************************************************

#define _context ((AVCodecContext *)_contextVoid)

uint8_t scratchPad[SCRATCH_PAD_SIZE];
/**
        \fn beginDecompress
*/
   uint8_t ADM_AudiocodecWMA::beginDecompress( void )
   {
            _tail=_head=0;
            return 1;
   };
/**
        \fn endDecompress
*/

   uint8_t ADM_AudiocodecWMA::endDecompress( void )
   {
          _tail=_head=0;
          return 1;
   };
/**
        \fn ADM_AudiocodecWMA
*/

 ADM_AudiocodecWMA::ADM_AudiocodecWMA(uint32_t fourcc,WAVHeader *info,uint32_t l,uint8_t *d)
       :  ADM_Audiocodec(fourcc)
 {
    printf(" [ADM_AD_LAV] Using decoder for type 0x%x\n",info->encoding);
    printf(" [ADM_AD_LAV] #of channels %d\n",info->channels);
    _tail=_head=0;
    channels=info->channels;
    _contextVoid=(void *)avcodec_alloc_context();
    ADM_assert(_contextVoid);
    // Fills in some values...
    _context->codec_type=AVMEDIA_TYPE_AUDIO;
    _context->sample_rate = info->frequency;
    _context->channels = info->channels;
    _blockalign=_context->block_align = info->blockalign;
    _context->bit_rate = info->byterate*8;
    switch(fourcc)
    {
      case WAV_WMA:
        _context->codec_id = CODEC_ID_WMAV2;
        break;
      case WAV_QDM2:
        _context->codec_id = CODEC_ID_QDM2;
        break;
      case WAV_AMV_ADPCM:
        _context->codec_id = CODEC_ID_ADPCM_IMA_AMV;
        _blockalign=1;
        break;
      case WAV_NELLYMOSER:
        _context->codec_id = CODEC_ID_NELLYMOSER;
        _blockalign=1;
        break;
      case WAV_DTS:
        _context->codec_id = CODEC_ID_DTS;
        _blockalign = 1;
        break;
      case WAV_MP3:
        _context->codec_id = CODEC_ID_MP3;
        _blockalign = 1;
        break;
      case WAV_MP2:
        _context->codec_id = CODEC_ID_MP2;
        _blockalign = 1;
        break;
      case WAV_AC3:
        _context->codec_id = CODEC_ID_AC3;
        _blockalign = 1;
        break;
      case WAV_EAC3:
        _context->codec_id = CODEC_ID_EAC3;
        _blockalign = 1;
        break;
      case WAV_AAC:
      case 0x706D:
        _context->codec_id = CODEC_ID_AAC;
        _blockalign = 1;
        break;
      default:
             ADM_assert(0);
    }





    _context->extradata=(uint8_t *)d;
    _context->extradata_size=(int)l;
    printf("[ADM_AD_LAV] Using %d bytes of extra header data\n", _context->extradata_size);
    mixDump((uint8_t *)_context->extradata,_context->extradata_size);

   AVCodec *codec=avcodec_find_decoder(_context->codec_id);
   if(!codec) {ADM_assert(0);}
    if (avcodec_open(_context, codec) < 0)
    {
        printf("[audioCodec] Lavc audio decoder init failed !\n");
        ADM_assert(0);
    }
    if(!_blockalign)
    {
      if(_context->block_align) _blockalign=_context->block_align;
      else
      {
        printf("[ADM_ad_lav] : no blockalign taking 378\n");
        _blockalign=378;
      }
    }
    printf("[ADM_ad_lav] init successful (blockalign %d)\n",info->blockalign);
  
}
 ADM_AudiocodecWMA::~ADM_AudiocodecWMA()
 {
        avcodec_close(_context);
        ADM_dealloc(_context);
        _contextVoid=NULL;
}
/**
    \fn run

*/
uint8_t ADM_AudiocodecWMA::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
int out=0;
int max=0,pout=0;
int16_t *run16;
int nbChunk;

        *nbOut=0;
        // Shrink
        if(_head && (_tail+nbIn)*3>ADMWA_BUF*2)
        {
            memmove(_buffer,_buffer+_head,_tail-_head);
            _tail-=_head;
            _head=0;
        }
        //
        ADM_assert(nbIn+_tail<ADMWA_BUF);
        memcpy(_buffer+_tail,inptr,nbIn);
        _tail+=nbIn;
        while(_tail-_head>=_blockalign)
        {
          nbChunk=(_tail-_head)/_blockalign;
          pout=SCRATCH_PAD_SIZE;
          out=avcodec_decode_audio2(_context,(int16_t *)scratchPad,
                                   &pout,_buffer+_head,nbChunk*_blockalign);

          if(out<0)
          {
            printf( "[ADM_ad_lav] *** WMA decoding error (%u)***\n",_blockalign);
            _head+=1; // Try skipping some bytes
            continue;
          }
          if(pout>=SCRATCH_PAD_SIZE)
          {
            printf("[ADM_ad_lav]Produced : %u, buffer %u,in%u\n",pout,SCRATCH_PAD_SIZE,_tail-_head);
            ADM_assert(0);
          }

//            printf("Channel layout :%llx\n",_context->channel_layout);

          if(_context->codec_id == CODEC_ID_NELLYMOSER)
          { // Hack, it returns inconsistent size
            out=nbChunk*_blockalign;
          }
          _head+=out; // consumed bytes
          pout>>=1;
          *nbOut+=pout;
          run16=(int16_t *)scratchPad;
          for(int i=0;i<pout;i++)
          {
            *outptr++=((float)run16[i])/32767.;
          }
        }
          if(channels>=5 )
            {
            CHANNEL_TYPE *p_ch_type = channelMapping;
#define DOIT(x,y) if(_context->channel_layout & CH_##x) *(p_ch_type++)=ADM_CH_##y;
                    
                    DOIT(FRONT_LEFT,FRONT_LEFT);
                    DOIT(FRONT_RIGHT,FRONT_RIGHT);
                    DOIT(FRONT_CENTER,FRONT_CENTER);                    
                    DOIT(LOW_FREQUENCY,LFE);
                    DOIT(SIDE_LEFT,REAR_LEFT);
                    DOIT(SIDE_RIGHT,REAR_RIGHT);
            }
        
        return 1;
}
//---

