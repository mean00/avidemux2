/** *************************************************************************
                          \fn ADM_videoFilterDynamic
                          \brief Include for external dynamically loaded video filters
	(C) 2008 Mean fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 #ifndef VIDEO_FILTERS_DYNAMIC_H
 #define VIDEO_FILTERS_DYNAMIC_H

#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"

typedef FilterDescriptor *(VF_getDescriptor)(void);
/*
 *      This macro is used to automatically construct the struct describing a filter.
 *      It is to be used for filters that are ui agnostic i.e. they have no UI at all
 *      or use dialogFactory.
 * 
 */
#define VF_DEFINE_FILTER(myClass,myParam,name,displayname,version,category,description) \
    SCRIPT_CREATE(name##_script,myClass,myParam); \
    BUILD_CREATE(name##_create,myClass); \
	static FilterDescriptor descriptor_vf_id_##myClass (\
								0, \
								displayname, \
                                                                #name, \
								description, \
								category, \
								name##_create, \
								name##_script, \
								ADM_FILTER_API_VERSION, \
                                                                ADM_UI_ALL); \
    extern "C" { 	FilterDescriptor *ADM_VF_getDescriptor(void) {return &descriptor_vf_id_##myClass ;}};
/*
 *      Same as above, but this filter is using a specific UI and cannot be used for another UI 
 */
#ifndef  ADM_UI_TYPE_BUILD 
#define VF_DEFINE_FILTER_UI ADM_UI_TYPE_BUILD_IS_NOT_DEFINED
#else
#define VF_DEFINE_FILTER_UI(myClass,myParam,name,displayname,version,category,description) \
    SCRIPT_CREATE(name##_script,myClass,myParam); \
    BUILD_CREATE(name##_create,myClass); \
        static FilterDescriptor descriptor_vf_id_##myClass (\
                                                                0, \
                                                                displayname, \
                                                                #name, \
                                                                description, \
                                                                category, \
                                                                name##_create, \
                                                                name##_script, \
                                                                ADM_FILTER_API_VERSION, \
                                                                ADM_UI_TYPE_BUILD); \
    extern "C" {        FilterDescriptor *ADM_VF_getDescriptor(void) {return &descriptor_vf_id_##myClass ;}};                                                   
#endif
#define ADM_MINIMAL_UI_INTERFACE

#define ADM_CONF_STRING_SIZE 256
#define ADM_FILTER_DECLARE_CONF(fmt,...) {static char confString[ADM_CONF_STRING_SIZE];\
                                    snprintf(confString,ADM_CONF_STRING_SIZE-1,fmt,## __VA_ARGS__);\
                                    return confString;} 

#endif
