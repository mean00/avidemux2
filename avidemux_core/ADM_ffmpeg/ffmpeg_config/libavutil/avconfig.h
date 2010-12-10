#include "ADM_coreConfig.h"
#ifndef AVCONFIG_H_ADM
#define AVCONFIG_H_ADM
#ifdef ADM_BIG_ENDIAN
#define AV_HAVE_BIGENDIAN 1
#else
#define AV_HAVE_BIGENDIAN 0
#endif

#define AV_HAVE_FAST_UNALIGNED 1
#endif
