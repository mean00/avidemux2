#ifndef ADM_COREVIDEOFILTERFUNC_H
#define ADM_COREVIDEOFILTERFUNC_H

#include "ADM_coreVideoFilterInternal.h"
#include "ADM_editor/include/IEditor.h"

bool ADM_vf_clearFilters(void);
ADM_vf_plugin *ADM_vf_getPluginFromTag(uint32_t tag);
bool ADM_vf_removeFilterAtIndex(int index);
bool ADM_vf_recreateChain(void);
ADM_coreVideoFilter *ADM_vf_createFromTag(uint32_t tag, ADM_coreVideoFilter *last, CONFcouple *couples);
int ADM_vf_addFilterFromTag(IEditor *editor, uint32_t tag, CONFcouple *c, bool configure);
ADM_coreVideoFilter *ADM_vf_getLastVideoFilter(IEditor *editor);

#endif
