/***************************************************************************
                          audiotimeline.cpp  -  description
                             -------------------
    Try to build a timeline for audio track
    Very useful for VBR audio
    the bitrate*time does not work in that case

    begin                : Fri May 3 2002
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
#include <math.h>
#include "config.h"
#include "avifmt.h"
#include "avifmt2.h"
#include "fourcc.h"
#include "aviaudio.hxx"
#include "prefs.h"

#include "DIA_working.h"
#include "avidemutils.h"

#include "ADM_mp3info.h"

#define ATOM 100 //1152>>1
#define BIG_BUFFER (128*1024)

uint8_t AVDMGenericAudioStream::buildAudioTimeLine(void)
{

uint32_t in,d=0,rd=0,ms10,offset=0,index=0; //,left=0,len=0;;
uint32_t retry=50;
		// we do nothing in case of WAV or unhandler stream
  		// that left only MP3 for now ...
     if (_wavheader->encoding != WAV_MP3 &&_wavheader->encoding != WAV_MP2 )
      {
	  return 0;
      }
      // in case of mpeg3 we will build a pseudo memory map
      // one tick is 10 ms
      // 100*3600*2*4= 360*8*1000=3000 000= 3 meg
      // for a 2 hour movie
      // should be acceptable

	  uint32_t originalPriority = getpriority(PRIO_PROCESS, 0);
	  uint32_t priorityLevel;

	  prefs->get(PRIORITY_INDEXING,&priorityLevel);
	  setpriority(PRIO_PROCESS, 0, ADM_getNiceValue(priorityLevel));

      goTo(0);  //rewind
      printf("\n scanning timeline\n");
      // ms10 is the raw length of 10 ms of uncompressed audio
      // in sample
      ms10=_wavheader->frequency/100;


     if (_audioMap)
		 delete[] _audioMap;

     _audioMap=new ST_point[100*3600*4]; // 4 hours / 6 Megs


      ADM_assert(_audioMap);

      DIA_workingBase *work;

      work=createWorking(QT_TR_NOOP("Building VBR map"));

      goTo(0);
      uint32_t Mul=2*_wavheader->channels;
      uint32_t len,sample;
      while(offset<_length)
      	{
              			work->update(offset,_length);
				if(!work->isAlive())
				{
					delete work;
					work=createWorking(QT_TR_NOOP("Building VBR map"));
					work->update(offset,_length);
				}

                		// read a packet
				if(!getPacket(internalBuffer, &len,&sample))
				{
                                    if(!index && retry)
                                    {
                                        retry--;
                                        continue;
                                    }
					printf("MapVBR:Get packet failed\n");
					break;
				}
			        offset+=len;
			      	rd += sample*Mul;
      				_audioMap[index].foffset=offset;
      				_audioMap[index++].woffset=rd;
         }

end:
		setpriority(PRIO_PROCESS, 0, originalPriority);

        if(!index)
        {
            delete [] _audioMap;
            _audioMap=NULL;
            _wavheader->blockalign=1152; // Mark it as VBR
            delete work;
            goTo(0);	// Purge
            printf("Build VBR failed!\n");
            return 0;
        }
      _nbMap=index;
      _wavheader->blockalign=1152; // Mark it as VBR
      printf("\n Nb entries in timeline : %"LU"\n",_nbMap);
      delete work;
      goTo(0);	// Purge
     return 1;
}

//
//	Read one byte
//
//
uint8_t  AVDMGenericAudioStream::readc( uint8_t *c)
{
 	uint32_t status;
  			status=read(1,c);
     		return status;

}

