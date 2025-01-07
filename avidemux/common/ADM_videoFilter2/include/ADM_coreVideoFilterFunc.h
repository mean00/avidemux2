#ifndef ADM_COREVIDEOFILTERFUNC_H
#define ADM_COREVIDEOFILTERFUNC_H

#include "ADM_coreVideoFilter6_export.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_editor/include/IEditor.h"

// #define LOCAL_EXPORT ADM_COREVIDEOFILTER6_EXPORT
#define LOCAL_EXPORT

LOCAL_EXPORT bool ADM_vf_clearFilters(void);
LOCAL_EXPORT ADM_vf_plugin *ADM_vf_getPluginFromTag(uint32_t tag);
LOCAL_EXPORT bool ADM_vf_duplicateFilterAtIndex(IEditor *editor, int index);
LOCAL_EXPORT bool ADM_vf_removeFilterAtIndex(int index);
LOCAL_EXPORT bool ADM_vf_toggleFilterEnabledAtIndex(int index);
LOCAL_EXPORT bool ADM_vf_recreateChain(void);
LOCAL_EXPORT ADM_coreVideoFilter *ADM_vf_createFromTag(uint32_t tag, ADM_coreVideoFilter *last, CONFcouple *couples);
LOCAL_EXPORT ADM_VideoFilterElement *ADM_vf_insertFilterFromTag(IEditor *editor, uint32_t tag, CONFcouple *c,
                                                                int index);
LOCAL_EXPORT ADM_VideoFilterElement *ADM_vf_addFilterFromTag(IEditor *editor, uint32_t tag, CONFcouple *c,
                                                             bool configure);
ADM_coreVideoFilter *ADM_vf_getLastVideoFilter(IEditor *editor);
LOCAL_EXPORT void ADM_vf_updateBridge(IEditor *editor);
#undef LOCAL_EXPORT

#endif
