//
// C++ Interface: ADM_a52info
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ADM_A52INFO_H
#define ADM_A52INFO_H

#include "ADM_audioParser6_export.h"

ADM_AUDIOPARSER6_EXPORT int 	ADM_a52_syncinfo (const uint8_t * buf, int * flags, int * sample_rate, int * bit_rate);
ADM_AUDIOPARSER6_EXPORT uint8_t ADM_AC3GetInfo(const uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff)   ;

#endif