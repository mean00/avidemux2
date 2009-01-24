/***************************************************************************
                          ADM_3gpAudio.cpp  -  description
                             -------------------

	Provide access to the audio track embedded in 3gp file
	It can be either AMR NB/WB/ AAC
	The most common being AMR NB

 ***************************************************************************

    begin                : Tue Jul 23 2003
    copyright            : (C) 2002/2005 by mean
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "fourcc.h"
#include "ADM_mp4.h"
//_______________________________________________________
//
//
//_______________________________________________________

// MP4Audio::MP4Audio(_3gpIndex *idx, uint32_t nbchunk, FILE * fd,WAVHeader *incoming,uint32_t extraLen,uint8_t *extraData,uint32_t duration)
MP4Audio::MP4Audio(const char *name,MP4Track *track)
{
	_nb_chunks=track->nbIndex;
	_fd=fopen(name,"rb");
        ADM_assert(_fd);
	_current_index=0;
	_abs_position=0;
	_rel_position=0;
	_pos=0;
	_index=track->index;

	_extraLen=track->extraDataSize;
	_extraData=track->extraData;
	
	_wavheader=new WAVHeader;
        memcpy(_wavheader,&(track->_rdWav),sizeof(WAVHeader));
	

	_destroyable=1;	
	strcpy(_name,"3gp audio");	
	// compute length
	_length=0;
	for(uint32_t i=0;i<_nb_chunks;i++)
		{
			_length+=track->index[i].size;
		}
	printf("\n [MP4 audio] : %lu bytes (%lu chunks)\n",_length,_nb_chunks);

	printf("Byterate     :%d\n",_wavheader->byterate);
	printf("Frequency :%d\n",_wavheader->frequency);
	printf("Encoding   :%d\n",_wavheader->encoding);
	printf("Channels   :%d\n",_wavheader->channels);
	printf("Extra data :%lu\n",_extraLen);
        if(_nb_chunks)
            _audioDuration=_index[_nb_chunks-1].time;
       // _wavheader->frequency=48000;
    	goToTime(0);
}
 uint8_t	MP4Audio::goToTime(uint32_t mstime)
{
uint64_t target=mstime;
		target*=1000; // us
		if(target>_index[_nb_chunks-1].time)
		{
			printf("3GP: going out of time asked %lu : avail %lu\n",mstime,_index[_nb_chunks-1].time/1000);
			_current_index=_nb_chunks-1;
			return 1;
		}
		for(uint32_t i=0;i<_nb_chunks;i++)
		{
			if(_index[i].time >= target)
			{
				_current_index=i;
				printf("3gp Go to time succeeded chunk :%lu time ask:%lu time get:%lu\n",i,mstime,
						_index[i].time/1000);
				return 1;
			}
		
		}
		printf("3GP: gototime Failed\n");
		return 0;
}
//_______________________________________________________
//
//
//_______________________________________________________
uint8_t MP4Audio::goTo(uint32_t newoffset)
{
   ADM_assert(0);
    return 1;
}
//______________________________________
uint8_t MP4Audio::getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples)
{

uint32_t r=0;
double delta;
	if(_current_index>=_nb_chunks)  
        {
              printf("MP4Audio : index max :%u/%u\n",_current_index,_nb_chunks);
              return 0;
        }
	  fseeko(_fd,_index[_current_index].offset,SEEK_SET);
	  r=fread(dest,1,_index[_current_index].size,_fd);
          if(!r)
          {
            printf("[MP4 Audio] Cannot read \n"); 
          }
          else
          {
            
          }
	  if(_current_index==_nb_chunks-1)
	  {
	  	
                // Assume the last sample is the same size as the previous one
	  	//*samples=1024;
                delta=_index[_nb_chunks-1].time;
                
                if(_audioDuration>delta)
                {
                        delta=_audioDuration-delta;
                        delta/=1000;
                        // delta is the duration of the current chunk in us
                        delta*=_wavheader->frequency;
                        delta/=1000.; // mss -> second
                        *samples=(uint32_t)floor(delta);
                }else *samples=1024;
                printf("[MP4Audio]: Last sample %d current chunk %d nb chunk %d\n",
                                *samples,_current_index,_nb_chunks);
                
	  }
	  else
	  {
	  	
		delta=_index[_current_index+1].time-_index[_current_index].time;
		
		// delta is the duration of the current chunk in us
		delta*=_wavheader->frequency;
		delta/=1000.*1000.; // us -> second
		*samples=(uint32_t)floor(delta);
	     
	  }
#if 0
          printf("[MP4Audio]Read %u bytes\n", r);
            printf("MP4Audio : index  :%u/%u sample : %u\n",_current_index,_nb_chunks,*samples);
#endif
	  _current_index++;
	  *len=r;
	  
	  
	  return 1;
}
//_______________________________________________________
//
//
//_______________________________________________________

uint8_t	MP4Audio::extraData(uint32_t *l,uint8_t **d)
{
	if(_extraLen && _extraData)
	{
		*l=_extraLen;
		*d=_extraData;
		return 1;
	
	}
	*l=0;
	*d=NULL;
	return 0;
}

//_______________________________________________________
//
//
//_______________________________________________________
uint32_t MP4Audio::read(uint32_t len,uint8_t *buffer)
{
    uint32_t size,samples;
    if(!getPacket(buffer,&size,&samples)) return 0;
    return size;
    

}
//_______________________________________________________
//
//
//_______________________________________________________


MP4Audio::~MP4Audio()
{
	// nothing special to do...
	delete _wavheader;
	_wavheader=NULL;
        if(_fd)
        {
            fclose(_fd);
            _fd=NULL;
        }
}
//_______________________________________________________
uint8_t MP4Audio::getNbChunk(uint32_t *ch)
{
	*ch=_nb_chunks;
	return 1;
}
