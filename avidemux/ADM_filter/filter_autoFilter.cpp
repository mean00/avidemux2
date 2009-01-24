
/***************************************************************************
    \fn filter_autoFilter
    \brief Build aut8omatically addborder & resize filter.
    copyright            : (C) 200 by mean
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

#include "config.h"
#include "ADM_default.h"

#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"

#include "DIA_coreToolkit.h"


extern uint32_t nb_active_filter;
extern FILTER  videofilters[VF_MAX_FILTER];
extern AVDMGenericVideoStream *filterCreateFromTag(VF_FILTERS tag,CONFcouple *couple, AVDMGenericVideoStream *in);
//#include "ADM_video/ADM_vidCommonFilter.h"
//BUILD_CREATE(crop_create,AVDMVideoStreamCrop);

/**
 *      \fn insertFilter
 *      \brief Create filter
 */
static AVDMGenericVideoStream *insertFilter(AVDMGenericVideoStream *in,VF_FILTERS tag, CONFcouple *conf)
{
  
  AVDMGenericVideoStream *filter;
   
  filter=filterCreateFromTag(tag,conf, in) ;
  if(!filter)
    {
      GUI_Error_HIG("Filter","Cannot create filter.\nMake sure your plugins are properly installed.\n I might crash soon.");
      return NULL;
    }
  return filter;
}
/**
 *      \fn create_addBorder
 * 
 */
AVDMGenericVideoStream *create_addBorder(VF_FILTERS *tag,AVDMGenericVideoStream *in,uint32_t x,uint32_t x2,uint32_t y,uint32_t y2)
{
  VF_FILTERS filterTag=VF_INVALID;
  
  filterTag=filterGetTagFromName("addblack");
  if(filterTag==VF_INVALID)
    {
        GUI_Error_HIG("Border","The filter addBorder cannot be found.\nMake sure your plugins are properly installed.");
        return NULL;
    }
    // Create confcouples
  CONFcouple couple(4);
    couple.setCouple("left",x);
    couple.setCouple("right",x2);
    couple.setCouple("top",y);
    couple.setCouple("bottom",y2);
    *tag=filterTag;
    return insertFilter(in,filterTag,&couple);
}

/**
 *      \fn createResizeFromParam
 *      \brief Create & instert a resize filter
 */
AVDMGenericVideoStream *createResizeFromParam(VF_FILTERS *tag,AVDMGenericVideoStream*in, unsigned int a, unsigned int b)
{
  VF_FILTERS filterTag=VF_INVALID;
   
   filterTag=filterGetTagFromName("mpresize");
   if(filterTag==VF_INVALID)
     {
         GUI_Error_HIG("Resize","The filter resize cannot be found.\nMake sure your plugins are properly installed.");
         return NULL;
     }
     // Create confcouples
   CONFcouple couple(3);
     couple.setCouple("w",a);
     couple.setCouple("h",b);
     couple.setCouple("algo",1);
     *tag=filterTag;
     return insertFilter(in,filterTag,&couple);     
}
