/*


*/
#ifndef DMX_IDENT
#define DMX_IDENT

#include "ADM_coreDemuxerMpeg6_export.h"
typedef enum
{
        DMX_MPG_UNKNOWN=0,
        DMX_MPG_ES,
        DMX_MPG_H264_ES,
        DMX_MPG_PS,
        DMX_MPG_TS,
        DMX_MPG_TS2,
        DMX_MPG_MSDVR
}DMX_TYPE;
DMX_TYPE dmxIdentify(const char *name);
#endif
