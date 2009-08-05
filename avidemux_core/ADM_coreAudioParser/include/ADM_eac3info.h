/**

        \file ADM_eac3info
        \brief extract info from EAC3/A52B/DD+ streams
        Author: mean <fixounet@free.fr>, (C) 2009
        Code very derived from ffmpeg (tables etc...)

*/

#ifndef ADM_EAC3INFO_H
#define ADM_EAC3INFO_H
bool     ADM_a52b_syncinfo (uint8_t * buf, int * flags, int * sample_rate, int * bit_rate);
bool     ADM_EAC3GetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *byterate, uint32_t *chan,uint32_t *syncoff);
#endif //ADM_EAC3INFO_H