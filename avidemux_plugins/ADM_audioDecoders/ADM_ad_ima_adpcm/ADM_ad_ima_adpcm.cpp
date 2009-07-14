/**
    \file ADM_ad_ima_adpcm.cpp
    \brief Audio decoders built ulaw from mplayer or ffmpeg (??, can't remember)
    \author mean (c) 2009

*/
/*
    IMA ADPCM Decoder for MPlayer
      by Mike Melanson

    This file is in charge of decoding all of the various IMA ADPCM data
    formats that various entities have created. Details about the data
    formats can be found here:
      http://www.pcisys.net/~melanson/codecs/

    So far, this file handles these formats:
      'ima4': IMA ADPCM found in QT files
        0x11: IMA ADPCM found in MS AVI/ASF/WAV files
        0x61: DK4 ADPCM found in certain AVI files on Sega Saturn CD-ROMs;
              note that this is a 'rogue' format number in that it was
              never officially registered with Microsoft
    0x1100736d: IMA ADPCM coded like in MS AVI/ASF/WAV found in QT files
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_default.h"
#include <math.h>
#define ADM_NO_CONFIG_H
#include "ADM_ffmpeg/libavutil/bswap.h"
#include "ADM_ad_plugin.h"

/**
    \class ADM_AudiocodecImaAdpcm
    \brief
*/

 uint8_t scratchPad[SCRATCH_PAD_SIZE];
#define IMA_BUFFER 4096*8
class ADM_AudiocodecImaAdpcm : public     ADM_Audiocodec
{
	protected:
		uint32_t _inStock,_me,_channels;
		int ss_div,ss_mul; // ???
		void *_contextVoid;
		uint8_t _buffer[ IMA_BUFFER];
		uint32_t _head,_tail;

	public:
		ADM_AudiocodecImaAdpcm(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
		virtual	~ADM_AudiocodecImaAdpcm();
		virtual	uint8_t beginDecompress(void) {_head=_tail=0;return 1;}
		virtual	uint8_t endDecompress(void) {_head=_tail=0;return 1;}
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
};
// Supported formats + declare our plugin
//*******************************************************
static uint32_t Formats[]={WAV_IMAADPCM};
DECLARE_AUDIO_DECODER(ADM_AudiocodecImaAdpcm,						// Class
			0,0,1, 												// Major, minor,patch
			Formats, 											// Supported formats
			"IMA ADPCM decoder plugin for avidemux (c) Mean\n"); 	// Desc
//********************************************************


#define MS_IMA_ADPCM_PREAMBLE_SIZE 4

#define QT_IMA_ADPCM_PREAMBLE_SIZE 2
#define QT_IMA_ADPCM_BLOCK_SIZE 0x22
#define QT_IMA_ADPCM_SAMPLES_PER_BLOCK 64

#define BE_16(x) (be2me_16(*(unsigned short *)(x)))
#define BE_32(x) (be2me_32(*(unsigned int *)(x)))
#define LE_16(x) (le2me_16(*(unsigned short *)(x)))
#define LE_32(x) (le2me_32(*(unsigned int *)(x)))

int ms_ima_adpcm_decode_block(unsigned short *output,
  unsigned char *input, int channels, int block_size);

// pertinent tables for IMA ADPCM
static int adpcm_step[89] =
{
  7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
  19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
  50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
  130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
  337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
  876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
  2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
  5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
  15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int adpcm_index[16] =
{
  -1, -1, -1, -1, 2, 4, 6, 8,
  -1, -1, -1, -1, 2, 4, 6, 8
};

// useful macros
// clamp a number between 0 and 88
#define CLAMP_0_TO_88(x)  if (x < 0) x = 0; else if (x > 88) x = 88;
// clamp a number within a signed 16-bit range
#define CLAMP_S16(x)  if (x < -32768) x = -32768; \
  else if (x > 32767) x = 32767;
// clamp a number above 16
#define CLAMP_ABOVE_16(x)  if (x < 16) x = 16;
// sign extend a 16-bit value
#define SE_16BIT(x)  if (x & 0x8000) x -= 0x10000;
// sign extend a 4-bit value
#define SE_4BIT(x)  if (x & 0x8) x -= 0x10;

// static ad_info_t info = 
// {
// 	"IMA ADPCM audio decoder",
// 	"imaadpcm",
// 	"Nick Kurshev",
// 	"Mike Melanson",
// 	""
// };

ADM_AudiocodecImaAdpcm::ADM_AudiocodecImaAdpcm( uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d)
        : ADM_Audiocodec(fourcc)
{
        _me=info->encoding;
        _channels=info->channels;

         if ((info->encoding == WAV_IMAADPCM) || (info->encoding == 0x61) 
        //||      (info->encoding == (uint16_t)0x1100736d)
        )
  {
     ss_div = info->blockalign -
      (MS_IMA_ADPCM_PREAMBLE_SIZE * info->channels) * 2;
     ss_mul = info->blockalign;
  }
  else
  {
    ss_div = QT_IMA_ADPCM_SAMPLES_PER_BLOCK;
    ss_mul = QT_IMA_ADPCM_BLOCK_SIZE * info->channels;
  }
  //sh_audio->audio_in_minsize=sss_mul;
  _tail=_head=0;
  printf("Block size: %d\n",ss_mul);
}
ADM_AudiocodecImaAdpcm::~ADM_AudiocodecImaAdpcm()
{

}

uint8_t ADM_AudiocodecImaAdpcm::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
int produced=0,one;
uint8_t  *start;
int16_t *run16;
// Add to buffer
  
  ADM_assert((_tail+nbIn)<IMA_BUFFER);
  memcpy(&(_buffer[_tail]),inptr,nbIn);
  _tail+=nbIn;
  *nbOut=0;

  if((_tail-_head)<ss_mul) 
        return 0;


  if ((_me== WAV_IMAADPCM))// || (sh_audio->format == 0x1100736d))
  {
        while((_tail-_head)>=ss_mul)
        {
                start=(uint8_t *)&(_buffer[_head]);
                one=  ms_ima_adpcm_decode_block(
                        (unsigned short *)scratchPad,start,_channels , ss_mul);
                _head+=ss_mul;
                produced+=one;
                run16=(int16_t *)scratchPad;
                for(int i=0;i<one;i++)
                {
                  *outptr++=((float)run16[i])/32767.;
                }
        }
        if(_tail>IMA_BUFFER/2 && _head)
        {
                memmove(_buffer,&_buffer[_head],_tail-_head);
                _tail-=_head;
                _head=0;
        }
        *nbOut=produced;
        return 1;
  }
#if 0
  else if (_me == 0x61)
  {
    return 2 * dk4_ima_adpcm_decode_block(
      (unsigned short*)outptr, ptr, _channels,  ss_mul);
  }
  else
  {
    return 2 * qt_ima_adpcm_decode_block(
      (unsigned short*)outptr, ptr, _channels);
  }
#endif
        return 0;
}


static void decode_nibbles(unsigned short *output,
  int output_size, int channels,
  int predictor_l, int index_l,
  int predictor_r, int index_r)
{
  int step[2];
  int predictor[2];
  int index[2];
  int diff;
  int i;
  int sign;
  int delta;
  int channel_number = 0;

  step[0] = adpcm_step[index_l];
  step[1] = adpcm_step[index_r];
  predictor[0] = predictor_l;
  predictor[1] = predictor_r;
  index[0] = index_l;
  index[1] = index_r;

  for (i = 0; i < output_size; i++)
  {
    delta = output[i];

    index[channel_number] += adpcm_index[delta];
    CLAMP_0_TO_88(index[channel_number]);

    sign = delta & 8;
    delta = delta & 7;

    diff = step[channel_number] >> 3;
    if (delta & 4) diff += step[channel_number];
    if (delta & 2) diff += step[channel_number] >> 1;
    if (delta & 1) diff += step[channel_number] >> 2;

    if (sign)
      predictor[channel_number] -= diff;
    else
      predictor[channel_number] += diff;

    CLAMP_S16(predictor[channel_number]);
    output[i] = predictor[channel_number];
    step[channel_number] = adpcm_step[index[channel_number]];

    // toggle channel
    channel_number ^= channels - 1;

  }
}

int ms_ima_adpcm_decode_block(unsigned short *output,
  unsigned char *input, int channels, int block_size)
{
  int predictor_l = 0;
  int predictor_r = 0;
  int index_l = 0;
  int index_r = 0;
  int i;
  int channel_counter;
  int channel_index;
  int channel_index_l;
  int channel_index_r;

  predictor_l = LE_16(&input[0]);
  SE_16BIT(predictor_l);
  index_l = input[2];
  if (channels == 2)
  {
    predictor_r = LE_16(&input[4]);
    SE_16BIT(predictor_r);
    index_r = input[6];
  }

  if (channels == 1)
    for (i = 0;
      i < (block_size - MS_IMA_ADPCM_PREAMBLE_SIZE * channels); i++)
    {
      output[i * 2 + 0] = input[MS_IMA_ADPCM_PREAMBLE_SIZE + i] & 0x0F;
      output[i * 2 + 1] = input[MS_IMA_ADPCM_PREAMBLE_SIZE + i] >> 4;
    }
  else
  {
    // encoded as 8 nibbles (4 bytes) per channel; switch channel every
    // 4th byte
    channel_counter = 0;
    channel_index_l = 0;
    channel_index_r = 1;
    channel_index = channel_index_l;
    for (i = 0;
      i < (block_size - MS_IMA_ADPCM_PREAMBLE_SIZE * channels); i++)
    {
      output[channel_index + 0] =
        input[MS_IMA_ADPCM_PREAMBLE_SIZE * 2 + i] & 0x0F;
      output[channel_index + 2] =
        input[MS_IMA_ADPCM_PREAMBLE_SIZE * 2 + i] >> 4;
      channel_index += 4;
      channel_counter++;
      if (channel_counter == 4)
      {
        channel_index_l = channel_index;
        channel_index = channel_index_r;
      }
      else if (channel_counter == 8)
      {
        channel_index_r = channel_index;
        channel_index = channel_index_l;
        channel_counter = 0;
      }
    }
  }
  
  decode_nibbles(output,
    (block_size - MS_IMA_ADPCM_PREAMBLE_SIZE * channels) * 2,
    channels,
    predictor_l, index_l,
    predictor_r, index_r);

  return (block_size - MS_IMA_ADPCM_PREAMBLE_SIZE * channels) * 2;
}


// EOF
