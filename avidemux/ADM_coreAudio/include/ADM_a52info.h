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
int 	ADM_a52_syncinfo (uint8_t * buf, int * flags, int * sample_rate, int * bit_rate);
uint8_t ADM_AC3GetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff)   ;
