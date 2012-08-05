#ifndef ADM_MEMSUPPORT_H
#define ADM_MEMSUPPORT_H

#include "ADM_core6_export.h"
#include "ADM_inttype.h"

ADM_CORE6_EXPORT void ADM_memStat(void);
ADM_CORE6_EXPORT void ADM_memStatInit(void);
ADM_CORE6_EXPORT void ADM_memStatEnd(void);
ADM_CORE6_EXPORT uint8_t ADM_InitMemcpy(void);

#endif