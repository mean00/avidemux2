/**

        \file ADM_eac3info
        \brief extract info from EAC3/A52B/DD+ streams
        Author: mean <fixounet@free.fr>, (C) 2009
        Code very derived from ffmpeg (tables etc...)

*/

#ifndef ADM_EAC3INFO_H
#define ADM_EAC3INFO_H

#include "ADM_audioParser6_export.h"

/**
    \struct ADM_EAC3_INFO
*/
typedef struct
{
    uint32_t frequency;
    uint32_t byterate;
    uint32_t channels;
    uint32_t frameSizeInBytes;
    uint32_t samples;
}ADM_EAC3_INFO;

ADM_AUDIOPARSER6_EXPORT bool     ADM_EAC3GetInfo(uint8_t *buf, uint32_t len, uint32_t *syncoff,ADM_EAC3_INFO *info);
#endif //ADM_EAC3INFO_H