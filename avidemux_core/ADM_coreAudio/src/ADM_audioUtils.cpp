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