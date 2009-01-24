/***************************************************************************
                          op_avisavedual.cpp  -  description
                             -------------------

		Save avi in copy mode for audio & video but with 2 tracks

    begin                : Wed Sep 11 2002
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
#include <math.h>
#include "config.h"
#include "ADM_default.h"
#include "ADM_threads.h"

#include "fourcc.h"
#include "avi_vars.h"


//#include "avilist.h"

#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"
#include "op_aviwrite.hxx"
#include "op_avisave.h"
#include "op_savecopy.h"

#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_audio/audioex.h"
#include "ADM_audiofilter/audioeng_buildfilters.h"

GenericAviSaveCopyDualAudio::GenericAviSaveCopyDualAudio (void	*track)
			: GenericAviSaveCopy()
{
   printf("**********************************\n");
   printf("second audio track set\n");
//    audio_filter2= track;
    _audioCurrent2=0;
}

//
//      Just to keep gcc happy....
//

uint8_t GenericAviSaveCopyDualAudio::setupAudio (void)
{
  int32_t shift=0;
  if(!audio_filter2) return 0;
  if(!currentaudiostream) return 0;
  
  audio_filter=buildAudioFilter( currentaudiostream,video_body->getTime (frameStart));
  audio_filter2->goToTime(0);
  return 1;
}
//---------------------------------------------------------------------------
uint8_t    GenericAviSaveCopyDualAudio::doOneTrack (uint32_t index,void *stream,uint32_t target,uint32_t *current)
{
  uint32_t    len;
  uint32_t sample,packetLen,packets=0;
  
  _audioInBuffer=0;
  // VBR mode, one packet per frame
#if 0
  if(stream->packetPerFrame()     || stream->isVBR() )
  {
    while(*current<target)
    {
      if(!stream->getPacket(abuffer,&packetLen,&sample))
      {
        printf("AVIWR:Could not read packet\n");
        return 0;
      }
      *current+=sample;
      if(!index)
        writter->saveAudioFrame (packetLen,abuffer);
      else
        writter->saveAudioFrameDual (packetLen,abuffer);
//      encoding_gui->feedAudioFrame(packetLen);
    }
	 	
  }
  else // CBR mode, pack them
  {
    sample=0;
    // _audioTarget is the # of sample we want
    while(*current<target)
    {
      if(!stream->getPacket(abuffer+_audioInBuffer,&packetLen,&sample))
      {
        printf("AVIWR:Could not read packet\n");
        break;
      }
      _audioInBuffer+=packetLen;
      *current+=sample;		
      packets++;
    }
    if (_audioInBuffer)
    {
      if(!index)
        writter->saveAudioFrame (_audioInBuffer, abuffer);
      else
        writter->saveAudioFrameDual (_audioInBuffer,abuffer);
//      encoding_gui->feedAudioFrame(_audioInBuffer);	  
    }
  }
#endif
  return 1;
}
// **************************************************************
uint8_t
GenericAviSaveCopyDualAudio::writeAudioChunk (uint32_t frame)
{
  uint32_t    len;
  // if there is no audio, we do nothing
  if (!audio_filter)
    return 0;
    
  double t;
  
  t=frame+1;
  t=t/fps1000;
  t=t*1000*audio_filter->getInfo()->frequency;
  _audioTarget=(uint32_t )floor(t);
  
//  if(!doOneTrack(0,audio_filter,_audioTarget,&_audioCurrent)) return 0;
//  if(!doOneTrack(1,audio_filter2,_audioTarget,&_audioCurrent2)) return 0;
  return 1;
}
//____________
