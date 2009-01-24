/***************************************************************************
                          ADM_nuvAudio.cpp  -  description
                             -------------------

                            Handle NUV (nuppel video) Audio track 

		* Audio is simple : 44.1 khz, stereo, pcm
		* Strongly derivated from avi audio lib
	- Added buffering through _compression type R. Should get rid of
			a/v sync problem

 ***************************************************************************

    begin                : Tue Jul 25 2002
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

#include "config.h"
#include <stdio.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef __FreeBSD__
	#include <sys/types.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include "ADM_assert.h"

#include "ADM_default.h"
#include "ADM_editor/ADM_Video.h"
#include "fourcc.h"
#include "ADM_nuv.h"

//_______________________________________________________
//
//
//_______________________________________________________

nuvAudio::nuvAudio(nuvIndex *idx, uint32_t nbchunk, FILE * fd,uint32_t fq,mythHeader *hdr)
{
	_nb_chunks=nbchunk;
	_fd=fd;
	_current_index=0;
	_abs_position=0;
	_rel_position=0;
	_pos=0;
	_index=idx;
	// nuv format is fixed : PCM stero 44.1 khz
	if(!hdr|| fourCC::check(hdr->audio_fourcc,(uint8_t *)"RAWA"))
	{
	_wavheader=new WAVHeader;
	memset(_wavheader,0,sizeof(WAVHeader));
	_wavheader->bitspersample=16;
	_wavheader->frequency=fq;
	_wavheader->channels=2;
	_wavheader->byterate=2*fq*2;
	_wavheader->encoding=WAV_PCM;
	}
	else
	{
	_wavheader=new WAVHeader;
	memset(_wavheader,0,sizeof(WAVHeader));
	_wavheader->bitspersample=hdr->audio_bits_per_sample;;
	_wavheader->frequency=hdr->audio_sample_rate;
	_wavheader->channels=hdr->audio_channels;;
	_wavheader->encoding=WAV_MP3; // ??	
	_wavheader->byterate=16000;

	}
	_destroyable=1;	
	strcpy(_name,"nuv audio");	
	// compute length
	_length=0;
	for(uint32_t i=0;i<_nb_chunks;i++)
		{
			_length+=_index[i]._len;
		}
	printf("\n Nuv audio : %lu bytes (%lu chunks)\n",_length,_nb_chunks);
	if(hdr)
	 if(fourCC::check(hdr->audio_fourcc,(uint8_t *)"LAME"))
       {
               _wavheader->encoding=WAV_MP3;
               uint8_t bfr[4096];
               WAVHeader hdr;
               goTo(0);
               read(4096,bfr);
                if(mpegAudioIdentify(bfr, 4090, &hdr))
                       memcpy(_wavheader,&hdr,sizeof(hdr));
     		goTo(0);

       }

}
//_______________________________________________________
//
//
//_______________________________________________________
uint8_t nuvAudio::goTo(uint32_t newoffset)
{
    uint32_t len;
    len = newoffset;
    if(len>=_length)
    	{
       	printf("\n Out of bound !\n");
     		return 0;
       }
    _current_index = 0;	// start at beginning
#ifdef VERBOSE_L3
    printf("\n Stream offset : %lu\n", newoffset);
#endif
    do
      {
	  if (len >= _index[_current_index]._len)	// skip whole subchunk
	    {
		len -= _index[_current_index]._len;
		_current_index++;
  		if(_current_index>=_nb_chunks)
    			{
                  printf("\n idx : %lu max: %lu len:%lu\n",  _current_index,_nb_chunks,len);
                  //ADM_assert(0);
                  //pos=0;
                  return 0;
       		};
		_rel_position = 0;
	  } else		// we got the last one
	    {
		_rel_position = len;
		len = 0;
	    }
	  //printf("\n %lu len bytes to go",len);
      }
    while (len);
    	_abs_position = _index[_current_index]._pos;
	ADM_assert(_current_index<_nb_chunks);
    	_pos=newoffset;
    return 1;
}
//_______________________________________________________
//
//
//_______________________________________________________
uint32_t nuvAudio::read(uint32_t len,uint8_t *buffer)
{
    uint32_t togo;
    uint32_t avail, rd;

    // just to be sure....  

    fseeko(_fd,_abs_position+_rel_position,SEEK_SET);

    togo = len;

#ifdef VERBOSE_L3
    printf("\n ABS: %lu rel:%lu len:%lu", _abs_position, _rel_position,
	   len);
#endif

     do
     {
	  avail = _index[_current_index]._len - _rel_position;	// how much available ?

	  if (avail > togo)	// we can grab all in one move
	    {
		if(_index[_current_index]._compression=='R')
			memset(buffer,0,togo);
		else
			if(togo!= fread( (uint8_t *) buffer,1,togo,_fd))
			{
				printf("\n ***WARNING : incomplete chunk ***\n");
			}

#ifdef VERBOSE_L3

		printf("\n REL: %lu rel:%lu len:%lu", _abs_position,
		       _rel_position, togo);
#endif

		_rel_position += togo;
#ifdef VERBOSE_L3

		printf("\n FINISH: %lu rel:%lu len:%lu", _abs_position,
		       _rel_position, togo);
#endif

		buffer += togo;
		togo = 0;
	  } else		// read the whole subchunk and go to next one
	    {
#ifdef VERBOSE_L3

		printf("\n CONT: %lu rel:%lu len:%lu", _abs_position,
		       _rel_position, togo);
#endif
		if(_index[_current_index]._compression=='R')
		{
			memset(buffer,0,avail);
			rd=avail;
		}
		else
		{
		rd = fread( buffer,1,avail,_fd);
		}
		if (rd != avail)
		  {
		      printf("\n Error : Expected :%lu bytes read :%lu \n",     rd, avail);
		      //ADM_assert(0);
		      return rd;

		  }
		buffer += avail;
		togo -= avail;

		_current_index++;
		if (_current_index>=_nb_chunks)
		  {
#ifdef VERBOSE_L3

		printf("\n OVR: %lu rel:%lu len:%lu", _abs_position,
		       _rel_position, togo);
#endif
		      _abs_position =_index[0]._pos ;
            _rel_position = 0;
		      ADM_assert(len >= togo);
        	  _pos+=len;
            _pos-=togo;
		      return (len - togo);

		  }
    	else
     	{
#ifdef VERBOSE_L3
		printf("\n CONT: %lu rel:%lu len:%lu", _abs_position,
		       _rel_position, togo);
#endif
		_abs_position = _index[_current_index]._pos;
		_rel_position = 0;
		//_riff->goTo(_abs_position);
		fseeko(_fd,_abs_position,SEEK_SET);
	    }
      }
    }
    while (togo);
    _pos+=len;
    return len;

}
//_______________________________________________________
//
//
//_______________________________________________________


nuvAudio::~nuvAudio()
{
	// nothing special to do...

}
//_______________________________________________________
uint8_t nuvAudio::getNbChunk(uint32_t *ch)
{
	*ch=_nb_chunks;
	return 1;
}
