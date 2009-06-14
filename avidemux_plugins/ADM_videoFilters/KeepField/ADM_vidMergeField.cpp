/***************************************************************************
                          Separate Fields.cpp  -  description
                             -------------------
Convert a x*y * f fps video into -> x*(y/2)*fps/2 video

Same idea as for avisynth separatefield


    begin                : Thu Mar 21 2002
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

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"
#include "ADM_vidFieldUtil.h"
#include "ADM_vidSeparateField.h"


static FILTER_PARAM swapParam={0,{""}};

//REGISTERX(VF_INTERLACING, "mergefield",QT_TR_NOOP("Merge fields"),QT_TR_NOOP(
//    "Merge two pictures as if they were two fields."),VF_MERGEFIELDS,1,mergefield_create,mergefield_script);

VF_DEFINE_FILTER(AVDMVideoMergeField,swapParam,
    mergefield,
                QT_TR_NOOP("Merge fields"),
                1,
                VF_INTERLACING,
                QT_TR_NOOP("Merge two pictures as if they were two fields."));
//------------------ and merge them ------------------


char *AVDMVideoMergeField::printConf( void )
{
 	ADM_FILTER_DECLARE_CONF(" Merge fields");
        
}

//_______________________________________________________________
AVDMVideoMergeField::AVDMVideoMergeField(
									AVDMGenericVideoStream *in,CONFcouple *setup)
{
UNUSED_ARG(setup);
  	_in=in;
   	memcpy(&_info,_in->getInfo(),sizeof(_info));	
	vidCache=new VideoCache(4,_in);
	

	_info.height<<=1;
	_info.fps1000>>=1;
	_info.nb_frames>>=1;


}

// ___ destructor_____________
AVDMVideoMergeField::~AVDMVideoMergeField()
{
 		delete vidCache;
		vidCache=NULL;
}

/**
	Interleave frame*2 and frame*2+1
*/
uint8_t AVDMVideoMergeField::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
uint32_t ref,ref2;
ADMImage *ptr1,*ptr2;
		if(frame>=_info.nb_frames) return 0;

		ref=frame<<1;
		ref2=ref+1;
		ptr1=vidCache->getImage(ref);
		ptr2=vidCache->getImage(ref+1);
		
		if(!ptr1 || !ptr2)
		{
			printf("Merge field : cannot read\n");
			vidCache->unlockAll();
		 	return 0;
		}
		 vidFieldMerge(_info.width,_info.height,ptr1->data,ptr2->data,data->data);
		 vidCache->unlockAll();
		
      return 1;
}
