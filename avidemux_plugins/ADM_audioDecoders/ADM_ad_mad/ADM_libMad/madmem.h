#ifndef MADMEM_H
#define MADMEM_H
#define ADM_LEGACY_PROGGY
#include "ADM_default.h"
// // #if 0
// // extern void *ADM_alloc(int size);
// // extern void ADM_dezalloc(void *ptr);
// // #endif
#undef malloc
#undef free
#define malloc ADM_alloc
#define free ADM_dezalloc

#endif

