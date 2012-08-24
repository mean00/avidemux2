/***************************************************************************
                          video_filters.h  -  description
                             -------------------
    begin                : Wed Mar 27 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 #ifndef __VIDEO_FILTERS__
 #define  __VIDEO_FILTERS__
#include "DIA_uiTypes.h"
#include "ADM_videoFilter_iface.h"

typedef AVDMGenericVideoStream *(ADM_createT) (AVDMGenericVideoStream *in, CONFcouple *);
typedef AVDMGenericVideoStream *(ADM_create_from_scriptT) (AVDMGenericVideoStream *in, int n,Arg *args);

class FilterDescriptor
{
public:
	
	 		const char *name; // external name asprinted
			const char *filterName; // internal name, used to lookup a filter by its name
			const char *description; // Description 
			const VF_CATEGORY category;
		
			const ADM_createT *create;
			const ADM_create_from_scriptT *create_from_script;
			const void (*destroy)(AVDMGenericVideoStream *old); /* Maybe needed ...*/
		
			const uint32_t apiVersion;
			const uint32_t uiFlags;
			const uint32_t filterVersion;
			VF_FILTERS tag;
			
		
			FilterDescriptor(uint32_t tag,
					const char *name,
					const char *filtername,
					const char *descTex,
					VF_CATEGORY category,
					AVDMGenericVideoStream *(*create) (AVDMGenericVideoStream *in, CONFcouple *),
					AVDMGenericVideoStream *(*create_from_script) (AVDMGenericVideoStream *in, int n,Arg *args),
					uint32_t apiVersion=ADM_FILTER_API_VERSION,
					uint32_t uiFlags=ADM_UI_ALL,uint32_t filterVersion=1):
						tag(tag),
						name(name),filterName(filtername),description(descTex),category(category),
						create(create),create_from_script(create_from_script),destroy(NULL),
						apiVersion(apiVersion),uiFlags(uiFlags),
						filterVersion(filterVersion)
					
			{
				tag=0;
			}
			FilterDescriptor() :name(NULL),filterName(NULL),description(NULL),category(VF_MAX),create(NULL),create_from_script(NULL),
						destroy(NULL),apiVersion(0),uiFlags(0),filterVersion(0)
			{
				
			}
};   
 	
   typedef struct
   {
          VF_FILTERS              tag;
          AVDMGenericVideoStream *filter;
          CONFcouple             *conf;
   }FILTER;
   
/* Number of activated filters you can have ..*/   
#define VF_MAX_FILTER 100
 
/* Some utility functions to deal with filter list */   
FILTER      *getCurrentVideoFilterList (uint32_t * count);
const FilterDescriptor * filterGetEntryFromTag (VF_FILTERS tag);
const char *filterGetNameFromTag(VF_FILTERS tag);
VF_FILTERS 	filterGetTagFromName(const char *inname);
uint8_t 	filterAddScript(VF_FILTERS tags,uint32_t n,Arg *args);

/* Statically register filters */
void registerFilterEx(const char *name,const char *filtername,VF_CATEGORY category,
		AVDMGenericVideoStream *(*create) (AVDMGenericVideoStream *in, CONFcouple *),
		AVDMGenericVideoStream *(*create_from_script) (AVDMGenericVideoStream *in, int n,Arg *args),
		const char *descText);

/* Save / load filters */
void filterSaveXml(const char *name);
void filterSaveXml(const char *name,uint8_t silent);
void filterLoadXml(const char *name);
int  filterLoadXml(const char *name,uint8_t silent);
void filterSaveScriptJS(FILE *f);

 #endif
