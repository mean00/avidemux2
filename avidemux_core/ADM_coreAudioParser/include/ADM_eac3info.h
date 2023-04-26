/**

        \file ADM_eac3info
        \brief extract info from EAC3/A52B/DD+ streams
        Author: mean <fixounet@free.fr>, (C) 2009
        Code very derived from ffmpeg (tables etc...)

*/

#ifndef ADM_EAC3INFO_H
#define ADM_EAC3INFO_H

#include "ADM_audioParser6_export.h"

#define ADM_AC3_HEADER_SIZE 7
#define ADM_EAC3_FLAG_PKT_COMPLETE 1 /* the header of the next independent frame found in the buffer at the expected location */
#define ADM_EAC3_FLAG_PKT_DAMAGED  2 /* no sync at the end of a frame */

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
    uint32_t flags;
}ADM_EAC3_INFO;

ADM_AUDIOPARSER6_EXPORT bool     ADM_EAC3GetInfo(const uint8_t *buf, uint32_t len, uint32_t *syncoff, ADM_EAC3_INFO *info, bool *plainAC3 = NULL);
#endif //ADM_EAC3INFO_H
