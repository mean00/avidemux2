/***************************************************************************
                          ADM_3gpAudio.cpp  -  description
                             -------------------

	Provide access to the audio track embedded in 3gp file
	It can be either AMR NB/WB/ AAC
	The most common being AMR NB

 ***************************************************************************

    begin                : Tue Jul 23 2003
    copyright            : (C) 2002/2005/2008 by mean
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


#include <string.h>
#include <math.h>

#include "ADM_default.h"
#include "ADM_Video.h"
#include "fourcc.h"
#include "ADM_mp4.h"


#define adm_printf(...) {}
#define aprintf(...) {}




/**
    \fn ADM_mp4AudioAccess
    \brief constructor
*/
 ADM_mp4AudioAccess::ADM_mp4AudioAccess(const char *name,MP4Track *track)
{
    _nb_chunks=track->nbIndex;
    _fd=ADM_fopen(name,"rb");
    ADM_assert(_fd);
    _current_index=0;
    _index=track->index;
    _endOfStream=false;

    extraDataLen=track->extraDataSize;
    extraData=track->extraData;

    // Check if MP3 track is actually MP2
    if(track->_rdWav.encoding == WAV_MP3 && _nb_chunks && _index[0].size >= 4)
    {
        uint8_t sample[4];

        fseeko(_fd, _index[0].offset, SEEK_SET);
        if(fread(&sample, 1, 4, _fd) < 4) return;

        uint32_t fcc = sample[0] << 24 | sample[1] << 16 | sample[2] << 8 | sample[3];
        int layer = 4 - ((fcc >> 17) & 0x3);

        if (layer == 2)
            track->_rdWav.encoding = WAV_MP2;
    }

    if(track->_rdWav.byterate == AUDIO_BYTERATE_UNSET || track->_rdWav.encoding == WAV_MP3)
    {
        int average = AUDIO_BYTERATE_UNSET;
        ADM_info("Estimating audio byterate...\n");
        uint64_t duration = _index[_nb_chunks-1].dts;
        if(duration != ADM_NO_PTS && duration > 100000) // at least 0.1 s
        {
            double d = duration;
            d /= 1000;
            d = track->totalDataSize / d;
            d *= 1000;
            if(d > 0 && d < (192000 * 32)) // anything beyond 7.1 at 192 kHz float must be clearly bogus
                average = (uint32_t)d;
        }
        if(average != AUDIO_BYTERATE_UNSET)
        {
            if(track->_rdWav.byterate == AUDIO_BYTERATE_UNSET)
            {
                track->_rdWav.byterate = average;
            }else // mp3
            {
                int probed = track->_rdWav.byterate;
#define MAX_AUDIO_BYTERATE_DEVIATION_FROM_AVG 100
                if(abs(average-probed) > MAX_AUDIO_BYTERATE_DEVIATION_FROM_AVG) // VBR mode mp3
                {
                    ADM_warning("Probed byterate %d doesn't match average %d, VBR?\n",probed,average);
                    track->_rdWav.byterate = average;
                }
            }
        }else if(track->_rdWav.byterate == AUDIO_BYTERATE_UNSET)
            track->_rdWav.byterate = 128000 >> 3; // use a dummy value as the last resort
    }
}
/**
    \fn ADM_mp4AudioAccess
    \brief destructor
*/

ADM_mp4AudioAccess::~ADM_mp4AudioAccess()
{
    if(_fd)
    {
        fclose(_fd);
        _fd=NULL;
    }
}
/**
    \fn ADM_mp4AudioAccess
    \brief goToTime
*/
bool      ADM_mp4AudioAccess::goToTime(uint64_t timeUs)
{
uint64_t target=timeUs;
		if(target>_index[_nb_chunks-1].dts)
		{
			printf("[MP4]: going out of time asked %" PRIu64" : avail %" PRIu64"\n",timeUs/1000,_index[_nb_chunks-1].dts/1000);
			_current_index=_nb_chunks-1;
			return true;
		}
		for(uint32_t i=0;i<_nb_chunks;i++)
		{
			if(_index[i].dts >= target)
			{
				_current_index=i;
				printf("[MP4] Go to time succeeded chunk :%" PRIu32" time ask:%" PRIu64" time get:%" PRIu64"\n",i,timeUs/1000,
						_index[i].dts/1000);
				return true;
			}

		}
		printf("[MP4]: gototime Failed\n");
		return false;

}
/**
    \fn getPacket
*/
bool    ADM_mp4AudioAccess::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
    uint32_t r=0;
    if(_current_index>=_nb_chunks)
    {
        if(!_endOfStream)
        {
            printf("[ADM_mp4AudioAccess::getPacket] Requested index %u out of bounds, max: %u\n",_current_index,(_nb_chunks)?(_nb_chunks-1):0);
            _endOfStream=true;
        }
        return 0;
    }
    fseeko(_fd,_index[_current_index].offset,SEEK_SET);
    r=fread(buffer,1,_index[_current_index].size,_fd);
    if(!r)
    {
        printf("[MP4 Audio] Cannot read \n");
        return false;
    }
    *dts=_index[_current_index].dts;
    *size=r;
    _current_index++;
    _endOfStream=false;
    return true;
}
/**
    \fn getDurationInUs
*/

uint64_t  ADM_mp4AudioAccess::getDurationInUs(void)
{
    return _index[_nb_chunks-1].dts;

}
//EOF
