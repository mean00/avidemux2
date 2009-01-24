//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ADM_assert.h"

#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_ogm.h"
#include "ADM_ogmpages.h"


uint8_t oggHeader::reorder( void )
{
OgIndex *newindex;
uint8_t ret=1;
uint32_t nbFrame= _videostream.dwLength;
	// already done..
	if( _reordered) return 1;
	ADM_assert(_index);
	printf("Ogm reordering..\n");
	newindex=new OgIndex[nbFrame];
	// clear B frame flag for last frames
	_index[nbFrame-1].flags &=~AVI_B_FRAME;

			//__________________________________________
			// the direct index is in DTS time (i.e. decoder time)
			// we will now do the PTS index, so that frame numbering is done
			// according to the frame # as they are seen by editor / user
			// I1 P0 B0 B1 P1 B2 B3 I2 B7 B8
			// xx I1 B0 B1 P0 B2 B3 P1 B7 B8
			//__________________________________________
			uint32_t forward=0;
			uint32_t curPTS=0;
			uint32_t dropped=0;

			for(uint32_t c=1;c<nbFrame;c++)
			{
				if(!(_index[c].flags & AVI_B_FRAME))
					{
								memcpy(&newindex[curPTS],
										&_index[forward],
										sizeof(OgIndex));
								forward=c;
								curPTS++;
								dropped++;
					}
					else
					{// we need  at lest 2 i/P frames to start decoding B frames
						if(dropped>=1)
						{
							memcpy(&newindex[curPTS],
								&_index[c],
								sizeof(OgIndex));
							curPTS++;
						}
						else
						{
						printf("We dropped a frame (%d/%d).\n",dropped,c);
						}
					}
			}

			uint32_t last;


			// put back last I/P we had in store
			memcpy(&newindex[curPTS],
				&_index[forward],
				sizeof(OgIndex));
			last=curPTS;

			_videostream.dwLength= _mainaviheader.dwTotalFrames=nbFrame=last+1;
			// last frame is always I

			delete [] _index;

			_index=newindex;;
			// last frame cannot be B frame
			_index[last].flags&=~AVI_B_FRAME;
			_index[0].flags=AVI_KEY_FRAME;
			 _reordered=ret;
			 if(ret)
			 	printf("Ogm reordering ok\n");
			return ret;

}
