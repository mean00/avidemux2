/***************************************************************************
                          audioeng_rawshift.cpp  -  description
                             -------------------

 	This filter i used only in copy mode
			     
    begin                : Fri Jun 28 2002
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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ADM_default.h"

#include "ADM_audioStream.h"
#include "audioprocess.hxx"
#include "audio_raw.h"
#if 0
AVDMProcessAudio_RawShift::AVDMProcessAudio_RawShift(ADM_audioStream * instream,
      uint32_t starttime, int32_t msoff): AVDMBufferedAudioStream    (instream)
{
    _wavheader = new WAVHeader;
    memcpy(_wavheader, _instream->getInfo(), sizeof(WAVHeader));
    _starttime=starttime;
    
    _hold=0;
    printf("[Raw shift] : Start:%u ms, shift  %d\n",_starttime,msoff);
    msoff=-msoff;
        if (msoff > 0) // just seek in the file
        {
                _starttime+=msoff;
                _instream->goToTime(  _starttime);
        }
        else
        {
                // need to dupe the beginning
                // if there is enough audio
                msoff=-msoff;
                if(_starttime>=msoff)
                {
                        _starttime-=msoff;
                        _instream->goToTime(  _starttime); // just rewind to compensate
                }
                else
                {
                        // we have to dupe a bit
                        msoff-=_starttime;
                        _starttime=0;
                        int32_t dupe;
                        _instream->goToTime(0);
                        _hold=(msoff*_wavheader->frequency)/1000; // in sample
                }
        }
//    _length = instream->getLength();
    printf("[Raw shift] : Start:%u ms, offset in sample  %d\n",_starttime,_hold);
};

AVDMProcessAudio_RawShift::~AVDMProcessAudio_RawShift()
{
  delete _wavheader;
}

//
//	If filterOn, it means we have to dupe sampleOffset sample, going back to start
//		and do it again
//
uint8_t	 AVDMProcessAudio_RawShift::getPacket(uint8_t *dest, uint32_t *len, uint32_t *samples)
{
        uint8_t r;
        uint64_t dts;
                
                if(!_hold)
                        return _instream->getPacket(dest,len,64*1024,samples,&dts); // BAZOOKA

                // filter is still on
                r=_instream->getPacket(dest,len,64*1024,samples,&dts); // BAZOOKA


                if(!r)
                {
                        printf("[RawShift]: Readerror\n");
                        *len=0;
                        *samples=0;
                        return 0;
                }
                if(_hold>0)
                {
                  _hold-=(int32_t)*samples;
                  if(_hold<=0)
                  {
                    _hold=0;  
                    _instream->goToTime(  _starttime);
                    printf("[RawShift] Rewinding to %u ms \n",_starttime);
                  }
                }
              return r;

}

uint32_t AVDMProcessAudio_RawShift::read(uint32_t len, uint8_t * buffer)
{
    return readDecompress(len, buffer);
};
uint32_t AVDMProcessAudio_RawShift::readDecompress(uint32_t len,
						    uint8_t * buffer)
{
 uint32_t l,sam;
          if(!getPacket(buffer,&len,&sam))
                  return 0;
          return len;

}
uint8_t AVDMProcessAudio_RawShift::goToTime(uint32_t newoffset)
{
        // since we are in copy mode, the only value accepted is 0 here
        ADM_assert(!newoffset);
        _instream->goToTime(  _starttime);
        return 1;
}

uint8_t AVDMProcessAudio_RawShift::goTo(uint32_t newoffset)
{

        // since we are in copy mode, the only value accepted is 0 here
        ADM_assert(!newoffset);
        _instream->goToTime(  _starttime);
        return 1;
};
// EOF
#endif
