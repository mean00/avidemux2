/***************************************************************************
                          filter_saveload.cpp  -  description
                             -------------------
	Save/load filter

    begin                : Wed Apr 12 2003
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
#define ADM_LEGACY_PROGGY

#include "ADM_default.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include "fourcc.h"
#include "DIA_coreToolkit.h"
#include "ADM_quota.h"


#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
//#include "ADM_video/ADM_videoNull.h"
#include "ADM_videoFilter_internal.h"
//#include "ADM_video/ADM_vidPartial.h"
#include "avi_vars.h"
// exported vars
extern uint32_t nb_active_filter;
extern FILTER  videofilters[VF_MAX_FILTER];

extern AVDMGenericVideoStream *filterCreateFromTag(VF_FILTERS tag,CONFcouple *couple, AVDMGenericVideoStream *in);
extern void filterCleanUp( void );


/*

  	Try to rebuild filter from text file


*/

static int32_t getIntegerAttribute(xmlNodePtr node, char *name, char *atrname);
static xmlNodePtr nextByName(xmlNodePtr node,char *name);
static CONFcouple *buildCouple(uint32_t nb, xmlNodePtr node);
extern int qxmlSaveFormatFile(const char *filename, xmlDocPtr cur, int format);
/**
	Xml Read/write ConfCouple from files
*/
void filterSaveXml(const char *docname)
{
	filterSaveXml(docname,0);
}

void filterSaveXml(const char *docname,uint8_t silent)
{
 xmlDocPtr xdoc;
 xmlNodePtr node;
 xmlAttrPtr attr;
 char tmpstring[1024];
VF_FILTERS tag;
int max;
		UNUSED_ARG(silent);
		if( ! (xdoc = xmlNewDoc((const xmlChar *)"1.0")) ) {printf("Problem creating xml doc\n");	return ;}
		if( ! (xdoc->children = xmlNewNode(NULL,(xmlChar *)"filters")) )
			{
				xmlFreeDoc(xdoc);
				xdoc = NULL;
				return ;
			}
		// we add # of filteras as attribute to the first children
		max=nb_active_filter;
		if(max) max--;
		sprintf(tmpstring,"%d",max);
		attr=xmlNewProp(xdoc->children,(xmlChar *)"FilterNumber",(xmlChar *)tmpstring);
		if(max)
		for(uint32_t curFilter=1;curFilter<nb_active_filter;curFilter++)
		{
			// for each filter we create a new node

			node= xmlNewChild(xdoc->children,NULL,(xmlChar *)"Filter",NULL );

			tag=videofilters[curFilter].tag;

			// and fill-up some informations
			sprintf(tmpstring,"%d",tag);
			attr=xmlNewProp(node,(xmlChar *)"Tag",(xmlChar *)tmpstring);

			attr=xmlNewProp(node,(xmlChar *)"Conf",(xmlChar *)videofilters[curFilter].filter->printConf());

			// now we create a child containing the config
			CONFcouple *setup;

			if( videofilters[curFilter].filter->getCoupledConf(&setup))
			{
				// we create a new child parameters with # of parameter as attribute
				 xmlNodePtr nodeConf; //,tmpnode;
// 				 xmlAttrPtr attrConf;


				 char *nm,*val;

					nodeConf= xmlNewChild(node,NULL,(xmlChar *)"Parameters",NULL );
					sprintf(tmpstring,"%"LU"",setup->getNumber());
					xmlNewProp(nodeConf,(xmlChar *)"Number",(xmlChar *)tmpstring);

					// and write each attribute
					for(uint32_t i=0;i<setup->getNumber();i++)
					{
						ADM_assert(setup->getEntry(i,&nm,&val));
						xmlNewChild(nodeConf,NULL,(xmlChar *)nm,(xmlChar *)val );
					}
				delete setup;
			}

		}

		xmlSetDocCompressMode(xdoc,0);  // we want pure text
		qxmlSaveFormatFile(docname,xdoc,1);
		xmlFreeDoc(xdoc);
	return ;
}
void filterLoadXml(const char *docname)
{
	filterLoadXml(docname,0);
}

int filterLoadXml(const char *docname,uint8_t silent)
{
#if 0
 xmlDocPtr xdoc;
 xmlNodePtr node,subnode;
int32_t nb_filters,itag;
CONFcouple *couple;
//uint16_t s16;
			UNUSED_ARG(silent);
			xdoc=xmlParseFile(docname);
			if(!xdoc)
				{
                                  GUI_Error_HIG(QT_TR_NOOP("Problem reading XML filters"), NULL);
					return 0;
				}
			node=xdoc->children;
			nb_filters=getIntegerAttribute(node,(  char *)"filters",(  char *)"FilterNumber");
			if(nb_filters==-1)
					{
							xmlFreeDoc(xdoc);
                                                        GUI_Error_HIG(QT_TR_NOOP("No filter found"), NULL);
							return 0;
					}

			if(1<nb_active_filter)
			{
					  filterCleanUp();
			}

			// if there is no active filter
			// Create at least the editor output one

			if(!nb_active_filter)
			{
                                uint32_t fStart,fEnd;

				// avoid going down
				aviInfo aviinf;
  				// now build infos
  				video_body->getVideoInfo(&aviinf);
				fStart=0;
				fEnd=aviinf.nb_frames-1;
  		 		videofilters[0].filter=  new AVDMVideoStreamNull(video_body,0,fEnd);

    			}
			nb_active_filter=1;

			// now ready to add new ones

			subnode=node->children;
			for(uint32_t curFilter=0;curFilter<(uint32_t)nb_filters;curFilter++)
			{
				// lookup next filter_saveload
				subnode=nextByName(subnode ,(char *)"Filter");
				if(!node)
					{
                                          GUI_Error_HIG(QT_TR_NOOP("Could not find a filter"), NULL);
						return 0;
					}

				itag=getIntegerAttribute(subnode,(char *)"Filter",(char *)"Tag");
				if(itag==-1)
					{
							xmlFreeDoc(xdoc);
                                                        GUI_Error_HIG(QT_TR_NOOP("No tag found"), NULL);
							return 0;
					}
				// now we got the tag
				printf("\n %"LU" tag \n",itag);

				// and we can build a CONFCouple if necessary
				xmlNodePtr conf;
				conf=subnode->children;
				conf=nextByName(conf ,(char *)"Parameters");
				if(!conf)
					{ // no paramaters needed
						couple=NULL;
					}
				else
					{
						int32_t args=getIntegerAttribute(conf,(char *)"Parameters",(char *)"Number");
			//			printf("\n ** needs %d param :\n",args);

						conf=conf->children;
						ADM_assert(conf);
						ADM_assert(args);
						couple=buildCouple(args,conf);

					}

				// once we have tag and CONFcouple we can build the filter
			//	if(couple) couple->dump();
				videofilters[nb_active_filter].filter= filterCreateFromTag( (VF_FILTERS)itag,
													couple,
													videofilters[nb_active_filter-1].filter);
				videofilters[nb_active_filter].tag=(VF_FILTERS)itag;
				videofilters[nb_active_filter].conf=couple;;
				nb_active_filter++;


				couple=NULL;
				// next filter please!
				subnode=subnode->next;
			}
			printf("\n found : %"LU" filters\n",nb_filters);
	xmlFreeDoc(xdoc);
#endif
	return 1;
}
CONFcouple *buildCouple(uint32_t nb, xmlNodePtr node)
{
char *val,*nm;
	CONFcouple *couple=new CONFcouple(nb);
	while(node)
	{
		nm=(char *)node->name;
		if(!xmlStrcmp((xmlChar *)nm,(xmlChar *)"text"))
			{
				node=node->next;
			 	continue;
			 }
		val=(char *)xmlNodeGetContent(node);

		ADM_assert(val);
		ADM_assert(couple->setCouple(nm,val));
	//	printf("\n --> :%s: %s \n",nm,val);
		nb--;
		node=node->next;

	};
	ADM_assert(nb==0);
	return couple;

}
xmlNodePtr nextByName(xmlNodePtr node,char *name)
{
// lookup next filter_saveload
				if(!node) return NULL;
				while(1)
				{
				//	printf("\n %d %s\n",curFilter,subnode->name);
					if(!xmlStrcmp(node->name,(const xmlChar *)name)) break;
					node=node->next;
					if(!node)
					{
                                          GUI_Error_HIG(QT_TR_NOOP("Node not found"), NULL);
						return NULL;
					}
				};
				return node;

}
int32_t getIntegerAttribute(xmlNodePtr node, char *name, char *atrname)
{

			if(!xmlStrcmp(node->name,(const xmlChar *)name))
			{
				// found
				char *str;
				str=(char *)xmlGetProp(node,(const xmlChar *)atrname);
				return atoi(str);
			}
			printf("\n %s was the name\n",node->name);



	return -1;
}

//EOF
