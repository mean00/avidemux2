/**
    \fn ADM_audioUtils.cpp
    \brief Will be moved later to ADM_coreAudio


*/
#include "ADM_default.h"
#include "ADM_coreAudio.h"
#include <math.h>

static float rand_table[DITHER_CHANNELS][DITHER_SIZE];

void AUDMEncoder_initDither(void)
{
  printf("Initializing Dithering tables\n");
	float d, dp;
	for (int c = 0; c < DITHER_CHANNELS; c++) {
		dp = 0;
		for (int i = 0; i < DITHER_SIZE-1; i++) {
			d = rand() / (float)RAND_MAX - 0.5;
			rand_table[c][i] = d - dp;
			dp = d;
		}
  		rand_table[c][DITHER_SIZE-1] = 0 - dp;
	}
}

void dither16(float *start, uint32_t len, uint8_t channels)
{
	static uint16_t nr = 0;
	int16_t *data_int = (int16_t *)start;
	float *data = start;

	len /= channels;
	for (int i = 0; i < len; i++) {
		for (int c = 0; c < channels; c++) {
			*data = roundf(*data * 32766 + rand_table[c][nr]);
			if (*data > 32767.0f) *data = 32767;
			if (*data < -32768.0f) *data = -32768;
			*data_int = (int16_t) *data;
			data++;
			data_int++;
		}
		nr++;
		if (nr >= DITHER_SIZE)
			nr = 0;
	}
}
/**
        \fn ADM_audioReorderChannels
*/
bool   ADM_audioReorderChannels(uint32_t channels,float *data, uint32_t nb,CHANNEL_TYPE *input,CHANNEL_TYPE *output)
{    

    float tmp [channels];
	static uint8_t reorder[MAX_CHANNELS];
	static bool reorder_on;
	
		reorder_on = 0;
        
		// Should we reorder the channels (might be needed for encoder ?
		if (channels > 2) 
		{
			int j = 0;

			for (int i = 0; i < channels; i++) 
			{
				for (int c = 0; c < channels; c++) 
				{
					if (input[c] == output[i]) 
					{
						if (j != c)
							reorder_on = 1;
						reorder[j++] = c;
					}
				}
			}
		}
	

	if (reorder_on)
		for (int i = 0; i < nb; i++) 
        {
			memcpy(tmp, data, sizeof(tmp));
			for (int c = 0; c < channels; c++)
				*data++ = tmp[reorder[c]];
		}

    return true;
}

void printWavHeader(WAVHeader *hdr)
{
#undef X_DUMP
#define X_DUMP(x) printf(#x":\t\t:%"PRIu32"\n",hdr->x);

          X_DUMP(encoding);
          X_DUMP(channels);	/* 1 = mono, 2 = stereo */
          X_DUMP(frequency);	/* One of 11025, 22050, or 44100 Hz */
          X_DUMP(byterate);	/* Average bytes per second */
          X_DUMP(blockalign);	/* Bytes per sample block */
          X_DUMP(bitspersample);	/* One of 8, 12, 16, or 4 for ADPCM */

}