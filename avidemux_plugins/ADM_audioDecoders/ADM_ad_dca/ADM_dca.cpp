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
#ifdef USE_LIBDCA_002
extern "C" {
#include "dts.h"
}
#define LIBDCA_STATE dts_state_t
#define DTS(x) DTS_##x
#else
extern "C" {
#include "dca.h"
}
#define LIBDCA_STATE dca_state_t
#define DTS(x) DCA_##x 
#endif

#define DTS_HEADER_SIZE (10)

class ADM_AudiocodecDCA : public     ADM_Audiocodec
{
	protected:
		void *dts_handle;

	public:
		ADM_AudiocodecDCA(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d);
		virtual	~ADM_AudiocodecDCA();
		virtual	uint8_t beginDecompress(void);
		virtual	uint8_t endDecompress(void);
		virtual	uint8_t run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut);
		virtual	uint8_t isCompressed(void) {return 1;}
		virtual	uint8_t isDecompressable(void) {return 1;}

   };
typedef struct
{
		int (*dts_block)(LIBDCA_STATE* state);
		int (*dts_blocks_num) (LIBDCA_STATE* state);
		int (*dts_frame) (LIBDCA_STATE* state, uint8_t* buf, int* flags, level_t* level, sample_t bias);
		void (*dts_free) (LIBDCA_STATE* state);
		LIBDCA_STATE* (*dts_init) (uint32_t mm_accel);
		sample_t* (*dts_samples) (LIBDCA_STATE* state);
		int (*dts_syncinfo) (LIBDCA_STATE* state, uint8_t* buf, int* flags, int* sample_rate, int* bit_rate, int* frame_length);
}DCA_func_struct;
#ifdef USE_LIBDCA_002
DCA_func_struct dcaFunctions=
{
	dts_block, 
	dts_blocks_num, 
	dts_frame, 
	dts_free, 
	dts_init,
	dts_samples, 
	dts_syncinfo
};	
#else // 05
DCA_func_struct dcaFunctions=
{
	dca_block, 
	dca_blocks_num, 
	dca_frame, 
	dca_free, 
	dca_init,
	dca_samples, 
	dca_syncinfo
};	
#endif

DCA_func_struct *dca=&dcaFunctions;

#define DTS_HANDLE ((LIBDCA_STATE *)dts_handle)
// Supported formats + declare our plugin
//*******************************************************
 static  ad_supportedFormat Formats[]={{WAV_DTS,AD_LOW_QUAL}};
DECLARE_AUDIO_DECODER(ADM_AudiocodecDCA,						// Class
			0,0,1, 												// Major, minor,patch 
			Formats, 											// Supported formats
#ifdef USE_LIBDCA_002			
			"libDts decoder plugin for avidemux (c) Mean/Gruntster\n"); 	// Desc
#else
			"libDca decoder plugin for avidemux (c) Mean/Gruntster\n"); 	// Desc
#endif
   //********************************************************
ADM_AudiocodecDCA::ADM_AudiocodecDCA(uint32_t fourcc, WAVHeader *info, uint32_t l, uint8_t *d) : 
  ADM_Audiocodec(fourcc,*info)
{
    int flags=0;
    ADM_assert(fourcc==WAV_DTS);
    dts_handle=NULL;
    
#ifdef ADM_CPU_X86
#define CHK(x,y) if(CpuCaps::has##x()) flags|=MM_ACCEL_X86_##y;
    CHK(MMX,MMX);
    CHK(3DNOW,3DNOW);
    CHK(MMXEXT,MMXEXT);
#endif
    
    dts_handle=(void *)dca->dts_init(flags);
    if(!dts_handle)
    {
        printf("Cannot init libdca\n");
        ADM_assert(0);   
    }
}

ADM_AudiocodecDCA::~ADM_AudiocodecDCA( )
{
    if(dts_handle)
    {
        dca->dts_free(DTS_HANDLE);
        dts_handle=NULL;
    }
}

uint8_t ADM_AudiocodecDCA::beginDecompress( void )
{
    return 1;
}

uint8_t ADM_AudiocodecDCA::endDecompress( void )
{
    return 1;
}

uint8_t ADM_AudiocodecDCA::run(uint8_t *inptr, uint32_t nbIn, float *outptr, uint32_t *nbOut)
{
    uint32_t avail;
    uint32_t length,syncoff;
    int flags = 0, samprate = 0, bitrate = 0, frame_length;
    uint8_t chan = wavHeader.channels;
    *nbOut=0;


    //  Ready to decode
    while(nbIn)
    {
        if(nbIn<DTS_HEADER_SIZE)
        {
            if(nbIn)
                printf("[DTS]: no data to decode avail %u\n",nbIn);
            break;
        }
        //length = ADM_DCAGetInfo(ptr,nbIn, &samprate, &bitrate, &chan,&syncoff,&flags);
        length = dca->dts_syncinfo (DTS_HANDLE,  inptr, &flags, &samprate,&bitrate, &frame_length);

        if(!length)
        {
            printf("[DTS] dts_syncinfo failed\n");
            ADM_assert(0); 
        }
        if(length>nbIn)
        {
            // not enough data
            break;
        }
        CHANNEL_TYPE *p_ch_type = channelMapping;

		switch (flags & DTS(CHANNEL_MASK)) 
		{
			case DTS(MONO):
				*(p_ch_type++) =ADM_CH_MONO;
			break;
			case DTS(STEREO):
			case DTS(DOLBY):
				*(p_ch_type++) =ADM_CH_FRONT_LEFT;
				*(p_ch_type++) =ADM_CH_FRONT_RIGHT;
			break;
			case DTS(3F):
				*(p_ch_type++) =ADM_CH_FRONT_CENTER;
				*(p_ch_type++) =ADM_CH_FRONT_LEFT;
				*(p_ch_type++) =ADM_CH_FRONT_RIGHT;
			break;
			case DTS(2F1R):
				*(p_ch_type++) =ADM_CH_FRONT_LEFT;
				*(p_ch_type++) =ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) =ADM_CH_REAR_CENTER;
			break;
			case DTS(3F1R):
				*(p_ch_type++) =ADM_CH_FRONT_CENTER;
				*(p_ch_type++) =ADM_CH_FRONT_LEFT;
				*(p_ch_type++) =ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) =ADM_CH_REAR_CENTER;
			break;
			case DTS(2F2R):
				*(p_ch_type++) =ADM_CH_FRONT_LEFT;
				*(p_ch_type++) =ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) =ADM_CH_REAR_LEFT;
				*(p_ch_type++) =ADM_CH_REAR_RIGHT;
			break;
			case DTS(3F2R):
				*(p_ch_type++) =ADM_CH_FRONT_CENTER;
				*(p_ch_type++) =ADM_CH_FRONT_LEFT;
				*(p_ch_type++) =ADM_CH_FRONT_RIGHT;
				*(p_ch_type++) =ADM_CH_REAR_LEFT;
				*(p_ch_type++) =ADM_CH_REAR_RIGHT;
			break;
			default:
				ADM_assert(0);
		} // End switch
		if (flags & DTS(LFE)) 
		{
			*(p_ch_type++) =ADM_CH_LFE;
		}
	

        sample_t level = 1, bias = 0;
        flags &=DTS(CHANNEL_MASK);
        flags |= DTS(ADJUST_LEVEL);

        if (dca->dts_frame(DTS_HANDLE, inptr, &flags, &level, bias))
        {
            printf("\n DTS_frame failed!");
            inptr+=length;
            nbIn-=length;
            outptr+=256*chan;
            *nbOut+=256*chan;
            break;
        };

        inptr+=length;
        nbIn-=length;
        // Each block is 256 samples
        *nbOut += 256 * chan * dca->dts_blocks_num(DTS_HANDLE);
	float *cur;
	for (int i = 0; i < dca->dts_blocks_num(DTS_HANDLE); i++) {
		if (dca->dts_block(DTS_HANDLE)) {
			printf("\n[DTS] dts_block failed on block %d/%d\n",i,dca->dts_blocks_num (DTS_HANDLE));
			// in that case we silent out the chunk
			memset(outptr, 0, 256 * chan * sizeof(float));
		} else {
			float *cur;
			for (int k = 0; k < chan; k++) {
				sample_t *sample=(sample_t *)dca->dts_samples(DTS_HANDLE);
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

