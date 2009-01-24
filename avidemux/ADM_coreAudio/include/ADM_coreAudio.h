/**
        \file ADM_audioCore.h

*/
#ifndef ADM_audioCore_H
#define ADM_audioCore_H


#define MINUS_ONE 0xffffffff

#define DITHER_SIZE 4800
#define DITHER_CHANNELS 6
void            AUDMEncoder_initDither();
void dither16(float *start, uint32_t nb, uint8_t channels);

#include "ADM_audiodef.h"

#endif
//EOF

