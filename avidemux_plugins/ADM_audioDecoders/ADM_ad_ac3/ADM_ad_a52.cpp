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
#include "ADM_default.h"
#include "ADM_ad_plugin.h"

extern "C" {
#include "ADM_liba52/a52.h"
#include "ADM_liba52/mm_accel.h"
};

#define AC3_HANDLE ((a52_state_t *)ac3_handle)
#define ADM_AC3_BUFFER (50000*2)
/**
    \class ADM_AudiocodecAC3
*/
class ADM_AudiocodecAC3 : public     ADM_Audiocodec
{
	protected:
		void *ac3_handle;
		void *ac3_sample;

	public:
		ADM_AudiocodecAC3(uint32_t fourcc, WAVHeader *info,uint32_t extraLength,uint8_t *extraDatab);
		virtual	~ADM_AudiocodecAC3();
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
		virtual	uint8_t isDecompressable(void) {return 1;}

   };
// Supported formats + declare our plugin
//*******************************************************
   static  ad_supportedFormat Formats[]={{WAV_AC3,AD_MEDIUM_QUAL}};
   DECLARE_AUDIO_DECODER(ADM_AudiocodecAC3,						// Class
		   	0,0,1, 												// Major, minor,patch
		   	Formats, 											// Supported formats
		   	"LibAC3 decoder plugin for avidemux (c) Mean\n"); 	// Desc
   //********************************************************

ADM_AudiocodecAC3::ADM_AudiocodecAC3( uint32_t fourcc, WAVHeader *info,uint32_t extraLength,uint8_t *extraData)
		:   ADM_Audiocodec(fourcc,*info)
{
    int flags=0;
    
    ADM_assert(fourcc==WAV_AC3);
    ac3_handle=NULL;
    ac3_sample=NULL;
#ifdef ADM_CPU_X86
#define CHK(x,y) if(CpuCaps::has##x()) flags|=MM_ACCEL_X86_##y;
    CHK(MMX,MMX);
    CHK(3DNOW,3DNOW);
    CHK(MMXEXT,MMXEXT);
#endif

    ac3_handle=(void *)a52_init(flags);
    if(!ac3_handle)
    {
        ADM_error("Cannot init a52\n");
        ADM_assert(0);
    }
    ac3_sample=(sample_t *)a52_samples(AC3_HANDLE);
    if(!ac3_sample)
    {
        ADM_warning("Cannot init a52 sample\n");
        ADM_assert(0);
    }

      
}

ADM_AudiocodecAC3::~ADM_AudiocodecAC3( )
{
    if(ac3_handle)
    {
        a52_free(AC3_HANDLE);
        ac3_handle=NULL;
        ac3_sample=NULL;
    }
    
}


uint8_t ADM_AudiocodecAC3::run(uint8_t *inptr, uint32_t nbIn, float *outptr,   uint32_t *nbOut)
{
    uint32_t avail;
    uint32_t length;
    int flags = 0, samprate = 0, bitrate = 0;
    uint8_t chan = wavHeader.channels;
    *nbOut=0;

    //  Ready to decode
    while(nbIn)
    {
        if(nbIn<7)
        {
            if(nbIn)
                ADM_warning("[a52]: no enough data to decode, available %"PRIu32" bytes, need at least 7\n",nbIn);
            break;
        }
        length = a52_syncinfo(inptr, &flags, &samprate, &bitrate);
        if(length==0)
        {
            ADM_warning("[a52] No startcode found\n");
            break;
        }
        if(length>nbIn)
        {
            // not enough data
            break;
        }


		CHANNEL_TYPE *p_ch_type = channelMapping;
		if (flags & A52_LFE) {
			*(p_ch_type++) = ADM_CH_LFE;
		}
		switch (flags & A52_CHANNEL_MASK) {
            case A52_CHANNEL:
			case A52_MONO:
				*(p_ch_type++) = ADM_CH_MONO;
			break;
			case A52_STEREO:
			case A52_DOLBY:
				*(p_ch_type++) = ADM_CH_FRONT_LEFT;
				*(p_ch_type++) = ADM_CH_FRONT_RIGHT;
			break;
			case A52_3F:
				*(p_ch_type++) = ADM_CH_FRONT_LEFT;
				*(p_ch_type++) = ADM_CH_FRONT_CENTER;
				*(p_ch_type++) = ADM_CH_FRONT_RIGHT;
			break;
			case A52_2F1R:
				*(p_ch_type++) = ADM_CH_FRONT_LEFT;
				*(p_ch_type++) = ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) = ADM_CH_REAR_CENTER;
			break;
			case A52_3F1R:
				*(p_ch_type++) = ADM_CH_FRONT_LEFT;
				*(p_ch_type++) = ADM_CH_FRONT_CENTER;
				*(p_ch_type++) = ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) = ADM_CH_REAR_CENTER;
			break;
			case A52_2F2R:
				*(p_ch_type++) = ADM_CH_FRONT_LEFT;
				*(p_ch_type++) = ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) = ADM_CH_REAR_LEFT;
				*(p_ch_type++) = ADM_CH_REAR_RIGHT;
			break;
            
			case A52_3F2R:
				*(p_ch_type++) = ADM_CH_FRONT_LEFT;
				*(p_ch_type++) = ADM_CH_FRONT_CENTER;
				*(p_ch_type++) = ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) = ADM_CH_REAR_LEFT;
				*(p_ch_type++) = ADM_CH_REAR_RIGHT;
			break;
			default:
				ADM_assert(0);
		}


        sample_t level = 1, bias = 0;

        if (a52_frame(AC3_HANDLE, inptr, &flags, &level, bias))
        {
            ADM_warning(" A52_frame failed!\n");
            inptr+=length;
            nbIn-=length;
            *nbOut += 256 * chan * 6;
            break;
        };
        inptr+=length;
        nbIn-=length;
        *nbOut += 256 * chan * 6;

        float *cur;
        for (int i = 0; i < 6; i++) {
                if (a52_block(AC3_HANDLE)) {
                        ADM_warning(" A52_block failed! on fblock :%d\n", i);
                        // in that case we silent out the chunk
                        memset(outptr, 0, 256 * chan * sizeof(float));
                } else {
                        for (int k = 0; k < chan; k++) {
                                sample_t *sample=(sample_t *)ac3_sample;
                                sample += 256 * k;
                                cur = outptr + k;
                                for (int j = 0; j < 256; j++) {
                                        *cur = *sample++;
                                        cur+=chan;
                                }
                        }
                }
                outptr += chan * 256;
        }
    }
    return 1;

}
