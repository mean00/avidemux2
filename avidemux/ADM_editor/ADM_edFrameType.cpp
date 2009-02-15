/***************************************************************************
                          ADM_edFrameType.cpp  -  description
                             -------------------
  Rederive Frame type if needed

    begin                : Fri Apr 12 2002
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ADM_assert.h"

#include "config.h"
#include "fourcc.h"
#include "prefs.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_codecs/ADM_codec.h"
#include "ADM_videoFilter.h"
#include "avi_vars.h"
#include "DIA_working.h"
#include "DIA_coreToolkit.h"
#include "avidemutils.h"


/**
	Rebuild frame type by actually decoding them
	to all videos loaded
	Use hurry_up flag if the codec is able to do it.

*/
uint8_t   ADM_Composer::rebuildFrameType ( void)
{
_VIDEOS *vi;
uint32_t frames=0,cur=0;
uint8_t *compBuffer=NULL;
//uint8_t *prepBuffer=NULL;
ADMImage *prepBuffer=NULL;
ADMImage *prepBufferNoCopy=NULL;
ADMImage *tmpImage=NULL;
uint32_t bframe;
aviInfo    info;
						
	if(!_nb_video)
	{
          GUI_Error_HIG(QT_TR_NOOP("No video loaded"), NULL);
		return 0;
	}
	if(!isIndexable())
	{
          GUI_Error_HIG(QT_TR_NOOP("Not indexable"),QT_TR_NOOP( "DivX 5 + packed?"));
		return 0;
	}

	uint32_t originalPriority = getpriority(PRIO_PROCESS, 0);
	uint32_t priorityLevel;

	prefs->get(PRIORITY_INDEXING,&priorityLevel);
	setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));

	vi=&(_videos[0]);
	vi->_aviheader->getVideoInfo (&info);
                        
	compBuffer=new uint8_t[(info.width * info.height * 3)>>1];
	ADM_assert(compBuffer);

	prepBuffer=new ADMImage(info.width ,info.height);            
    prepBufferNoCopy=new ADMImage(info.width ,info.height);      
    ADMCompressedImage img;
    img.data=compBuffer;

	ADM_assert(prepBuffer);
    ADM_assert(prepBufferNoCopy);

	for(uint32_t i=0;i<_nb_video;i++)
	{
		frames+=_videos[i]._nb_video_frames;
	}
	DIA_workingBase *work;
    uint8_t nocopy;
	work=createWorking(QT_TR_NOOP("Rebuilding Frames"));


	for(uint32_t vid=0;vid<_nb_video;vid++)
	{
		// set the decoder in fast mode
			vi=&(_videos[vid]);
			vi->_aviheader->getVideoInfo (&info);
			nocopy=vi->decoder->dontcopy();
                        if(nocopy) tmpImage=prepBufferNoCopy;
                                else tmpImage=prepBuffer;
			bframe=0;
			if(vi->_reorderReady)
			{
				cur+=vi->_nb_video_frames;
			}
			else
			{
                
				vi->decoder->decodeHeaderOnly();
				for(uint32_t j=0;j<vi->_nb_video_frames;j++)
				{
	  				vi->_aviheader->getFrame (j,&img);
					if(img.dataLength)
                                        {
		    				vi->decoder->uncompress (&img, tmpImage);
                                        }
					else
						tmpImage->flags=0;
	  				vi->_aviheader->setFlag(j,tmpImage->flags);
					if(tmpImage->flags & AVI_B_FRAME)
						bframe++;

					if(work->update(cur, frames))
	  				{
						delete work;
						vi->decoder->decodeFull();
						GUI_Error_HIG(QT_TR_NOOP("Aborted"), NULL);
						delete [] compBuffer;
						delete  prepBuffer;
						delete  prepBufferNoCopy;

						setpriority(PRIO_PROCESS, 0, originalPriority);
						return 0;
       				}
					cur++;
				}
				vi->decoder->decodeFull();
				
			}
	}
	delete work;
	delete [] compBuffer;
	delete  prepBuffer;
    delete  prepBufferNoCopy;

	setpriority(PRIO_PROCESS, 0, originalPriority);

	return 1;
}


