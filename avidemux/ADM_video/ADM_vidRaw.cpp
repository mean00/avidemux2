/***************************************************************************
                          ADM_vidRaw.cpp  -  description
                             -------------------

    Pass throught filter, just to have a coherent interface

    begin                : Wed Mar 20 2002
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
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_video/ADM_videoNull.h"




//_______________________________________________________________

AVDMVideoStreamRaw::AVDMVideoStreamRaw(ADM_Composer *in,uint32_t start, uint32_t nb)
{
aviInfo aviinf;
  	_in=in;

  	// now build infos

  	ADM_assert(_in->getVideoInfo(&aviinf));
  	_info.width=aviinf.width;
  	_info.height=aviinf.height;
  	_info.nb_frames=nb;
	_start=start;
  	_info.encoding=0;
	_info.fps1000=aviinf.fps1000;
	ADM_assert(start+nb<=aviinf.nb_frames);
	printf("\n Raw stream initialized with start frame = %"LU"\n",_start);
}
AVDMVideoStreamRaw::~AVDMVideoStreamRaw()
{


}
//
//	Basically ask a uncompressed frame from editor and ask
//		GUI to decompress it .
//

uint8_t AVDMVideoStreamRaw::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
        UNUSED_ARG(len);
    	UNUSED_ARG(flags);
                if(frame>= _info.nb_frames) return 0;
		*len=(_info.width*_info.height*3)>>1;
		if(!(frame<_info.nb_frames))
		{
				printf("\n going out of bounds! :%"LU" / %"LU" \n",frame,_info.nb_frames);
				return 0;
		}

			// read uncompressed frame
		if(!_in->getUncompressedFrame(_start+frame,data,flags)     )
     			return 0;

        return 1;


}

uint8_t AVDMVideoStreamRaw::configure( AVDMGenericVideoStream *instream){
	UNUSED_ARG(instream);
	return 0;
}
