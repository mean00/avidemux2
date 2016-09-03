#ifndef ADM_COREVIDEOFILTERFUNC_H
#define ADM_COREVIDEOFILTERFUNC_H

#include "ADM_coreVideoFilter6_export.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_editor/include/IEditor.h"

ADM_COREVIDEOFILTER6_EXPORT bool ADM_vf_clearFilters(void);
ADM_COREVIDEOFILTER6_EXPORT ADM_vf_plugin *ADM_vf_getPluginFromTag(uint32_t tag);
ADM_COREVIDEOFILTER6_EXPORT bool ADM_vf_removeFilterAtIndex(int index);
ADM_COREVIDEOFILTER6_EXPORT bool ADM_vf_recreateChain(void);
ADM_COREVIDEOFILTER6_EXPORT ADM_coreVideoFilter *ADM_vf_createFromTag(uint32_t tag, ADM_coreVideoFilter *last, CONFcouple *couples);
ADM_COREVIDEOFILTER6_EXPORT ADM_VideoFilterElement* ADM_vf_insertFilterFromTag(IEditor *editor, uint32_t tag, CONFcouple *c, int index);
ADM_COREVIDEOFILTER6_EXPORT ADM_VideoFilterElement* ADM_vf_addFilterFromTag(IEditor *editor, uint32_t tag, CONFcouple *c, bool configure);
ADM_coreVideoFilter *ADM_vf_getLastVideoFilter(IEditor *editor);

#endif
