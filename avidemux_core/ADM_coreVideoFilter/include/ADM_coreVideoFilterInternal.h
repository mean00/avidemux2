/**
        \fn ADM_vf_plugin.h
        \brief Interface for dynamically loaded video filter
*/
#ifndef ADM_vf_plugin_h
#define ADM_vf_plugin_h
#include <stddef.h>
#include "ADM_default.h"
#include "DIA_uiTypes.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_filterCategory.h"
#include "ADM_paramList.h"
#include "ADM_coreUtils.h"

#define VF_API_VERSION 2
/* These are the 6 functions exported by each plugin ...*/
typedef ADM_coreVideoFilter  *(ADM_vf_CreateFunction)(ADM_coreVideoFilter *previous,CONFcouple *conf);
typedef void              (ADM_vf_DeleteFunction)(ADM_coreVideoFilter *codec);
typedef int               (ADM_vf_SupportedUI)(void); //  QT4/GTK / ALL
typedef uint32_t          (ADM_vf_GetApiVersion)(void);
typedef bool              (ADM_vf_GetPluginVersion)(uint32_t *major, uint32_t *minor, uint32_t *patch);
typedef const char       *(ADM_vf_GetString)(void);
typedef VF_CATEGORY       (ADM_vf_getCategory)(void);

#define DECLARE_VIDEO_FILTER(Class,Major,Minor,Patch,UI,category,internalName,displayName,Desc) \
	extern "C" { \
	ADM_coreVideoFilter *create(ADM_coreVideoFilter *previous,CONFcouple *conf)\
	{ \
		return new Class(previous,conf);\
	} \
	void *destroy(ADM_coreVideoFilter *codec) \
	{ \
		Class *a=(Class *)codec;\
		delete a;\
	}\
	int supportedUI(void) \
	{ \
		return UI; \
	} \
	uint32_t getApiVersion(void)\
	{\
			return VF_API_VERSION;\
	}\
	bool getFilterVersion(uint32_t *major,uint32_t *minor, uint32_t *patch)\
	{\
		*major=Major;\
		*minor=Minor;\
		*patch=Patch;\
		return true;\
	}\
	const char *getDesc(void)\
	{\
		return Desc; \
	}\
	const char *getInternalName(void)\
	{\
		return internalName; \
	}\
	const char *getDisplayName(void)\
	{\
		return displayName; \
	}\
    VF_CATEGORY getCategory(void) \
    { \
        return category;\
    }\
	}

#endif
