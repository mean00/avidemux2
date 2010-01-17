/*

(c) Mean 2006
*/
#ifndef AVS_PROXY_INTERNAL_H
#include "avsHeader.h"
typedef struct SktHeader
{
    uint32_t cmd;
    uint32_t frame;
    uint32_t payloadLen;
    uint32_t magic;
}SktHeader;


#endif
