/***************************************************************************
                          ADM_edit.cpp  -  description
                             -------------------
    begin                : Thu Feb 28 2002
    copyright            : (C) 2002/2008 by mean
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
#include "math.h"

#include <fcntl.h>
#include <errno.h>

#if defined(__MINGW32__) || defined(ADM_BSD_FAMILY)
#include <sys/stat.h>
#endif

#include "fourcc.h"
#include "ADM_editor/ADM_edit.hxx"

#if 0

#include "ADM_inputs/ADM_inpics/ADM_pics.h"
#include "ADM_inputs/ADM_nuv/ADM_nuv.h"
#include "ADM_inputs/ADM_h263/ADM_h263.h"
//#include "ADM_3gp/ADM_3gp.h"
#include "ADM_inputs/ADM_mp4/ADM_mp4.h"
#include "ADM_inputs/ADM_openDML/ADM_openDML.h"
#include "ADM_inputs/ADM_avsproxy/ADM_avsproxy.h"
#include "ADM_inputs/ADM_matroska/ADM_mkv.h"
//#include "ADM_inputs/ADM_flv/ADM_flv.h"
#include "ADM_inputs/ADM_amv/ADM_amv.h"
#include "ADM_inputs/ADM_asf/ADM_asf.h"
#include "ADM_inputs/ADM_ogm/ADM_ogm.h"
#endif

#include "DIA_coreToolkit.h"
#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "prefs.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_EDITOR
#include "ADM_osSupport/ADM_debug.h"

#//include "ADM_inputs/ADM_mpegdemuxer/dmx_indexer.h"
#include "ADM_outputfmt.h"
#include "ADM_edPtsDts.h"

vidHeader *ADM_demuxerSpawn(uint32_t magic,const char *name);

int DIA_mpegIndexer (char **mpegFile, char **indexFile, int *aid,
		     int already = 0);
void DIA_indexerPrefill(char *name);
extern uint8_t indexMpeg (char *mpeg, char *file, uint8_t aid);

extern uint8_t loadVideoCodecConf( const char *name);
extern uint8_t parseScript(char *name);
uint8_t UI_SetCurrentFormat( ADM_OUT_FORMAT fmt );
const char *VBR_MSG = QT_TR_NOOP("Avidemux detected VBR MP3 audio in this file. For keeping audio/video in sync, time map is needed. Build it now?\n\nYou can do it later with \"Audio -> Build VBR Time Map\".");
//
//

#define TEST_MPEG2DEC

ADM_Composer::ADM_Composer (void) : ADM_audioStream(NULL,NULL)
{
uint32_t type,value;

   packetBufferSize=0;
   packetBufferDts=ADM_NO_PTS;

  _nb_segment = 0;
  _nb_video = 0;
  _total_frames = 0;
  _audioseg = 0;
  _audiooffset = 0;
  _audioSample=0;
  _lastseg = 99;
  _lastframe = 99;
  _nb_clipboard=0;
  _haveMarkers=0; // only edl have markers
  // Initialize a default postprocessing (dummy)
  initPostProc(&_pp,16,16);
  if(!prefs->get(DEFAULT_POSTPROC_TYPE,&type)) type=3;
  if(!prefs->get(DEFAULT_POSTPROC_VALUE,&value)) value=3;

  _pp.postProcType=type;
  _pp.postProcStrength=value;
  _pp.swapuv=0;
  _pp.forcedQuant=0;
  updatePostProc(&_pp);
  _imageBuffer=NULL;
  _internalFlags=0;
  // Start with a clean base
  memset (_videos, 0, sizeof (_videos));
  max_seg = MAX_SEG;
  _segments = new _SEGMENT[max_seg];
  memset (_segments, 0, sizeof (_segments));
  _scratch=NULL;

}
/**
	Remap 1:1 video to segments

*/
#define YOURAUDIO(x) _videos[x].audioTracks[_videos[x].currentAudioStream]
uint8_t ADM_Composer::resetSeg( void )
{
	_total_frames=0;
	for(uint32_t i=0;i<_nb_video;i++)
	{
		_segments[i]._reference = i;
        if(_videos[i].audioTracks)
            _segments[i]._audio_size = YOURAUDIO(i)->size;
        else
            _segments[i]._audio_size=0;
  		_segments[i]._audio_start = 0;
  		_segments[i]._start_frame = 0;
		_segments[i]._audio_duration = 0;
		_segments[i]._nb_frames   =   _videos[i]._nb_video_frames ;
		_total_frames+=_segments[i]._nb_frames  ;
		updateAudioTrack (i);
	}

  	_nb_segment=_nb_video;
  	computeTotalFrames();
	dumpSeg();
	return 1;
}
/**
	Return extra Header info present in avi chunk that are needed to initialize
	the video codec
	It is assumed that there is only one file or can share the same init data
	(huffyuv for example)
*/
uint8_t ADM_Composer::getExtraHeaderData (uint32_t * len, uint8_t ** data)
{
  return _videos[0]._aviheader->getExtraHeaderData (len, data);

}


/**
	Purge all segments
*/
uint8_t ADM_Composer::deleteAllSegments (void)
{


  memset (_segments, 0, sizeof (_segments));
  _nb_segment = 0;
  _total_frames=computeTotalFrames();
  return 1;

}

/**
	\fn Purge all videos
    \brief delete datas associated with all video
*/
void
ADM_Composer::deleteAllVideos (void)
{

  for (uint32_t vid = 0; vid < _nb_video; vid++)
    {

      // if there is a video decoder...
      if (_videos[vid].decoder)
            delete _videos[vid].decoder;
      if(_videos[vid].color)
            delete _videos[vid].color;
      // prevent from crashing
      _videos[vid]._aviheader->close ();
      delete _videos[vid]._aviheader;
      if(_videos[vid]._videoCache)
      	delete  _videos[vid]._videoCache;
      _videos[vid]._videoCache=NULL;
     // Delete audio codec too
     // audioStream will be deleted by the demuxer
      if(_videos[vid].audioTracks)
      {
            for(int i=0;i<_videos[vid].nbAudioStream;i++)
            {
                delete _videos[vid].audioTracks[i];
            }
            delete [] _videos[vid].audioTracks;
            _videos[vid].audioTracks=NULL;
      }
    }

  memset (_videos, 0, sizeof (_videos));


  if(_imageBuffer)
  	delete _imageBuffer;
  _imageBuffer=NULL;

}

ADM_Composer::~ADM_Composer ()
{
	deleteAllSegments();
	deleteAllVideos();
	deletePostProc(&_pp);

	if(_segments)
	{
		delete[] _segments;
		_segments=NULL;
	}
	if(_scratch)
	{
		delete _scratch;
		_scratch=NULL;
	}
}

/*
   			Return Magic : 4*4 bytes first

*/

/**
    \fn addFile
    \brief	Load or append a file.	The file type is determined automatically and the ad-hoc video decoder is spawned

    @param name: filename
    @param mode: 0 open, 1 append


    @return 1 on success, 0 on failure


*/
uint8_t ADM_Composer::addFile (const char *name, uint8_t mode)
{
  uint8_t    ret =    0;
  aviInfo    info;



UNUSED_ARG(mode);
	_haveMarkers=0; // by default no markers are present
  ADM_assert (_nb_segment < max_seg);
  ADM_assert (_nb_video < MAX_VIDEO);

    FILE *f=fopen(name,"r");
    uint8_t buffer[4];
    if(!f) return 0;
    fread(buffer,4,1,f);
    fclose(f);
    uint32_t magic=(buffer[3]<<24)+(buffer[2]<<16)+(buffer[1]<<8)+(buffer[0]);


  // First find the demuxer....
   	_videos[_nb_video]._aviheader=ADM_demuxerSpawn(magic,name);
    if(!_videos[_nb_video]._aviheader)
    {
     char str[512+1];
     snprintf(str,512,QT_TR_NOOP("Cannot find a demuxer for %s"), name);
      str[512] = '\0';
      GUI_Error_HIG(str,NULL);
      return 0;
    }
    ret = _videos[_nb_video]._aviheader->open(name);
   // check opening was successful
   if (ret == 0) {
     char str[512+1];
     snprintf(str,512,QT_TR_NOOP("Attempt to open %s failed!"), name);
      str[512] = '\0';
      GUI_Error_HIG(str,NULL);
      delete _videos[_nb_video]._aviheader;
      return 0;
   }

   /* check for resolution */
   if( _nb_video ){
      /* append operation */
      aviInfo info0, infox;
      _videos[   0     ]._aviheader->getVideoInfo (&info0);
      _videos[_nb_video]._aviheader->getVideoInfo (&infox);
      if( info0.width != infox.width || info0.height != infox.height ){
        char str[512+1];
         str[0] = '\0';
         if( info0.width != infox.width )
            strcpy(str,"width");
         if( info0.height != infox.height )
            snprintf(str+strlen(str),512-strlen(str),
              "%sheight%sdifferent between first and this video stream",
                 (strlen(str)?" and ":""),
                 (strlen(str)?" are ":" is ") );
         str[512] = '\0';
         GUI_Error_HIG(str,QT_TR_NOOP("You cannot mix different video dimensions yet. Using the partial video filter later, will not work around this problem. The workaround is:\n1.) \"resize\" / \"add border\" / \"crop\" each stream to the same resolution\n2.) concatinate them together"));
         delete _videos[_nb_video]._aviheader;
         return 0;
      }
   }

  // else update info
  _videos[_nb_video]._aviheader->getVideoInfo (&info);
  _videos[_nb_video]._aviheader->setMyName (name);

  // Printf some info about extradata
  {
    uint32_t l=0;
    uint8_t *d=NULL;
    _videos[_nb_video]._aviheader->getExtraHeaderData(&l,&d);
    if(l && d)
    {
        printf("[Editor]The video codec has some extradata (%d bytes)\n",l);
        mixDump(d,l);
        printf("\n");
    }
  }
  // 1st if it is our first video we update postproc
 if(!_nb_video)
 {
        uint32_t type,value;

        if(!prefs->get(DEFAULT_POSTPROC_TYPE,&type)) type=3;
        if(!prefs->get(DEFAULT_POSTPROC_VALUE,&value)) value=3;

	deletePostProc(&_pp );
 	initPostProc(&_pp,info.width,info.height);
	_pp.postProcType=type;
	_pp.postProcStrength=value;
	_pp.forcedQuant=0;
	updatePostProc(&_pp);

	if(_imageBuffer) delete _imageBuffer;
	_imageBuffer=new ADMImage(info.width,info.height);
 	_imageBuffer->_qSize= ((info.width+15)>>4)*((info.height+15)>>4);
	_imageBuffer->quant=new uint8_t[_imageBuffer->_qSize];
	_imageBuffer->_qStride=(info.width+15)>>4;
 }


//    fourCC::print( info.fcc );
  _total_frames += info.nb_frames;
  _videos[_nb_video]._nb_video_frames = info.nb_frames;


  // Update audio infos
  // an spawn the appropriate decoder
  //_________________________
   uint32_t nbAStream=_videos[_nb_video]._aviheader->getNbAudioStreams();

  if (!nbAStream)
    {
      printf ("[Editor] *** NO AUDIO ***\n");
      _videos[_nb_video].audioTracks = NULL;
      _videos[_nb_video].nbAudioStream=0;
      _videos[_nb_video].currentAudioStream=0;
    }
  else
    {
        // Read and construct the audio tracks for that videos


      audioInfo *info;
      uint32_t extraLen;
      uint8_t  *extraData;
      ADM_audioStream *stream;
      WAVHeader *header;

      _VIDEOS *thisVid=&(_videos[_nb_video]);
      // Create streams
      thisVid->audioTracks=new ADM_audioStreamTrack*[nbAStream];
      thisVid->nbAudioStream=nbAStream;
      for(int i=0;i<nbAStream;i++)
      {
            ADM_audioStreamTrack *track=new ADM_audioStreamTrack;


            header=thisVid->_aviheader->getAudioInfo(i );
            memcpy(&(track->wavheader),header,sizeof(*header));

            // We need at last fq so that advanceDts will work
            if(!i && !_nb_video)
            {
                wavHeader.frequency=header->frequency;
                wavHeader.channels=header->channels;
            }

            thisVid->_aviheader->getAudioStream(i,&stream);
            ADM_assert(stream);
            track->stream=stream;

            track->duration=stream->getDurationInUs();
            track->size=0;

            stream->getExtraData(&extraLen,&extraData);
            track->codec=getAudioCodec(header->encoding,header,extraLen,extraData);

            thisVid->audioTracks[i]=track;

      }
//      printf("[Editor] Duration in seconds: %"LLU", in samples: %"LLU"\n",_videos[_nb_video]._audio_duration/_wavinfo->frequency,_videos[_nb_video]._audio_duration);
    }

  printf ("[Editor] Decoder FCC: ");
  fourCC::print (info.fcc);
  // ugly hack
  if (info.fps1000 > 2000 * 1000)
    {
      printf ("[Editor] FPS too high, switching to 25 fps hardcoded\n");
      info.fps1000 = 25 * 1000;
      updateVideoInfo (&info);
    }
  uint32_t    	l;
  uint8_t 	*d;
  _videos[_nb_video]._aviheader->getExtraHeaderData (&l, &d);
  _videos[_nb_video].decoder = getDecoder (info.fcc,  info.width, info.height, l, d,info.bpp);

  _videos[_nb_video]._videoCache   =   new EditorCache(10,info.width,info.height) ;

  float frameD=info.fps1000;
  frameD=frameD/1000;
  frameD=1/frameD;
  frameD*=1000000;
  _videos[_nb_video].timeIncrementInUs=(uint64_t)frameD;
  printf("[Editor] About %"LLU" microseconds per frame\n",_videos[_nb_video].timeIncrementInUs);

  //
  //  And automatically create the segment
  //
  if(_videos[_nb_video].audioTracks)
    {
        ADM_audioStreamTrack *trk=_videos[_nb_video].audioTracks[0];
        _segments[_nb_segment]._audio_size = trk->size;
        _segments[_nb_segment]._audio_duration =trk->duration;
    }else
    {
        _segments[_nb_segment]._audio_size = 0;
        _segments[_nb_segment]._audio_duration =0;
    }

  _segments[_nb_segment]._reference = _nb_video;
  _segments[_nb_segment]._audio_start = 0;
  _segments[_nb_segment]._start_frame = 0;
  _segments[_nb_segment]._nb_frames   =   _videos[_nb_video]._nb_video_frames ;


//****************************


  // next one please
#if 0
        if(_videos[_nb_video].audioTracks)
        {
            WAVHeader *    _wavinfo=&(_videos[_nb_video].audioTracks[0]->wavheader);
            if(_wavinfo->encoding==WAV_MP3 && _wavinfo->blockalign==1152)
            {
              uint32_t autovbr=0;
              prefs->get(FEATURE_AUTO_BUILDMAP,&autovbr);
              if(autovbr || GUI_Confirmation_HIG(QT_TR_NOOP("Build Time Map"),QT_TR_NOOP( "Build VBR time map?"), VBR_MSG))
                    {
                   // _videos[_nb_video]._isAudioVbr=_videos[_nb_video]._audiostream->buildAudioTimeLine ();
                    }
            }
        }
#endif
	_nb_video++;
	_nb_segment++;

//______________________________________
// 1-  check for B _ frame  existence
// 2- check  for consistency with reported flags
//______________________________________
	uint8_t count=0;
TryAgain:
	_VIDEOS 	*vid;
	uint32_t err=0;

		vid= &(_videos[_nb_video-1]);
		vid->_reorderReady=0;
        vid->_unpackReady=0;
		// we only try if we got everything needed...
		if(!vid->decoder)
		{
			printf("[Editor] no decoder to check for B- frame\n");
		}else
        {
            decoders *decoder=vid->decoder;
            if(vid->_aviheader->providePts()==false) // Else we rely on demuxer PTS
            {
                printf("[Editor] This container does not provide PTS \n");
                if(decoder->bFramePossible())
                {
                    printf("[Editor] B- frame possible with that codec \n");
                    if(isMpeg4Compatible(info.fcc))
                    {
                        printf("[Editor] It is mpeg4-SP/ASP, try to guess all PTS\n");                        
                        setMpeg4PtsFromDts(vid->_aviheader,vid->timeIncrementInUs);
                    }
                }
                else   
                {
                        printf("[Editor] No B frame with that codec\n");
                        setPtsEqualDts(vid->_aviheader,vid->timeIncrementInUs);
                }
            }
        }
		GoToIntra(0);
        durationInUs=vid->_aviheader->getVideoDuration();
		printf("[Editor] End of B-frame check\n");

  return 1;
}
/**
	Send a re-order order to all video if
		- They may need it
		- It is not already done.
*/
uint8_t ADM_Composer::reorder (void)
{
_VIDEOS *vid;
	for(uint32_t i=0;i<_nb_video;i++)
	{
		vid=&_videos[i];
#if 0
		if(!vid->_reorderReady) // not already reordered ?
		{
				if(vid->decoder->bFramePossible()) // can be re-ordered ?
				{
						if((vid->_reorderReady=vid->_aviheader->reorder()))
						{
							aviInfo    info;
							_videos[i]._aviheader->getVideoInfo (&info);

							printf(" Video %"LU" has been reordered\n",i);
						}

				}
		}
#endif
	}
	return 1;
}
/*
        If one of the videos has VBR audio we handle the whole editor audio has VBR
        If it is CBR, it is not harmful
        and it avoid loosing the VBR info in case we do VBR time map upon loading
*/
uint8_t ADM_Composer::hasVBRVideos(void)
{
        for(int i=0;i<_nb_video;i++)
        {
                ADM_audioStreamTrack *trk=getTrack(i);
                if(trk)
                    if(trk->vbr) return 1;
        }
        return 0;
}

/*

*/
uint32_t ADM_Composer::getPARWidth()
{
  if (_nb_video)
  {
    return _videos[0].decoder->getPARWidth();
  }
  return 1;

}

uint32_t ADM_Composer::getPARHeight()
{
  if (_nb_video)
  {
    return _videos[0].decoder->getPARHeight();
  }
  return 1;

}

/**
	Set decoder settings (post process/swap u&v...)
	for the segment referred by frame

*/
uint8_t ADM_Composer::setDecodeParam (uint32_t frame)
{
uint32_t seg,relframe,ref;
  if (_nb_video)
  {
   if (!convFrame2Seg (frame, &seg, &relframe))
    {
      printf ("\n Conversion failed !\n");
      return 0;
    }
    // Search source
     ref = _segments[seg]._reference;
    _videos[ref].decoder->setParam ();
  }
  return 1;

}

/**
	Free all allocated memory and destroy all editors object


*/
uint8_t ADM_Composer::cleanup (void)
{
  deleteAllSegments ();
  deleteAllVideos ();
  _nb_segment = 0;
  _nb_video = 0;
  _total_frames = 0;

	if(_scratch)
	{
		delete _scratch;
		_scratch=NULL;
	}

  return 1;
}
/*
        param:
                source : source #
                start : start frame in source #
                nb    : nb frame to copy into segment
*/
uint8_t ADM_Composer::addSegment(uint32_t source,uint32_t start, uint32_t nb)
{
        // do some sanity check
        if(_nb_segment==max_seg-1)
	{
	   _SEGMENT *s;
            max_seg += MAX_SEG;
            s = new _SEGMENT[max_seg];
            memset (s, 0, sizeof(_SEGMENT)*max_seg);
            memcpy(s,_segments,sizeof(_SEGMENT)*(max_seg-MAX_SEG));
            delete _segments;
            _segments = s;
        }
        if(_nb_video<=source)
        {
                printf("[editor]: No such source %d/%d\n",source,_nb_video);
                 return 0;
        }
        if(_videos[source]._nb_video_frames<=start)
        {
                printf("[editor]:start out of bound %d/%d\n",start,_videos[source]._nb_video_frames);
                 return 0;
        }
        if(_videos[source]._nb_video_frames<start+nb)
        {
                printf("[editor]:end out of bound %d/%d\n",start+nb,_videos[source]._nb_video_frames);
                 return 0;
        }
        // ok, let's go
        _SEGMENT *seg=&(_segments[_nb_segment]);
        seg->_reference=source;
        seg->_start_frame=start;
        seg->_nb_frames=nb;
        _nb_segment++;
        updateAudioTrack (_nb_segment-1);
        _total_frames=computeTotalFrames();

        return 1;
}
/**
______________________________________________________
//  Remove frames , the frame are given as seen by GUI
//  We remove from start to end -1
// [start,end[
//______________________________________________________
*/
uint8_t ADM_Composer::removeFrames (uint32_t start, uint32_t end)
{

  uint32_t
    seg1,
    seg2,
    rel1,
    rel2;
  uint8_t
    lastone =
    0;

  if (end == _total_frames - 1)
    lastone = 1;

  // sanity check
  if (start > end)
    return 0;
  //if((1+start)==end) return 0;

  // convert frame to block, relative frame
  if (!convFrame2Seg (start, &seg1, &rel1) ||
      !convFrame2Seg (end, &seg2, &rel2))
    {
      ADM_assert (0);
    }
  // if seg1 != seg2 we can just modify seg1 and seg2
  if (seg1 != seg2)
    {
      // remove end of seg1

      removeFrom (rel1, seg1, 1);
      //  delete in between seg
      for (uint32_t seg = seg1 + 1; seg < (seg2); seg++)
	_segments[seg]._nb_frames = 0;
      // remove beginning of seg2
      removeTo (rel2, seg2, lastone);
    }
  else
    {
      // it is in the same segment, split it...
      // complete seg ?
      if ((rel1 == _segments[seg1]._start_frame)
	  && (rel2 ==
	      (_segments[seg1]._start_frame +
	       _segments[seg1]._nb_frames - 1)))
	{
	  _segments[seg1]._nb_frames = 0;
	}
      else
	{
	  // split in between.... duh !
	  duplicateSegment (seg1);
	  //
	  removeFrom (rel1, seg1, 1);
	  removeTo (rel2, seg1 + 1, lastone);
	}
    }

  // Crunch
  crunch ();
  sanityCheck ();
  // Compute total nb of frame
  _total_frames = computeTotalFrames ();
  printf ("\n %"LU" frames ", _total_frames);
  return 1;

}
//******************************
// Select audio track
//
//******************************
uint8_t ADM_Composer::getAudioStreamsInfo(uint32_t frame,uint32_t *nbStreams, audioInfo **infos)
{
uint32_t seg,rel,reference;

        if (!convFrame2Seg (frame, &seg, &rel))
        {
                printf("Editor : frame2seg failed (%u)\n",frame);
                return 0;
        }
        reference=_segments[seg]._reference;
        if(!_videos[reference].nbAudioStream)
        {
            *nbStreams=0;
            *infos=NULL;
        }
       // return _videos[reference]._aviheader->getAudioStreamsInfo(nbStreams,infos);

        *nbStreams=_videos[reference].nbAudioStream;
        *infos=new audioInfo[*nbStreams];
        for(int i=0;i<*nbStreams;i++)
        {
            WAVHeader *wav=&(_videos[reference].audioTracks[i]->wavheader);
            (*infos)->bitrate=(wav->byterate*8)/1000;
            (*infos)->channels=wav->channels;
            (*infos)->encoding=wav->encoding;
            (*infos)->frequency=wav->frequency;
            (*infos)->av_sync=0;
         }
        return 1;
    return 0;
}
/*
        Change the audio track for the source video attached to the "frame" frame

*/
uint32_t ADM_Composer::getCurrentAudioStreamNumber(uint32_t frame)
{
uint32_t   seg,rel,reference;

        if (!convFrame2Seg (frame, &seg, &rel))
        {
                printf("Editor : frame2seg failed (%u)\n",frame);
                return 0;
        }
        reference=_segments[seg]._reference;

        return _videos[reference].currentAudioStream;
}
/**
        \fn changeAudioStream

*/
uint8_t ADM_Composer::changeAudioStream(uint32_t frame,uint32_t newstream)
{
uint32_t   seg,rel,reference;
double     duration;
WAVHeader *wav;
aviInfo    info;

        if (!convFrame2Seg (frame, &seg, &rel))
        {
                printf("Editor : frame2seg failed (%u)\n",frame);
                return 0;
        }
        reference=_segments[seg]._reference;
        ADM_audioStreamTrack **trks=_videos[reference].audioTracks;
        uint32_t nb=_videos[reference].nbAudioStream;
        if(newstream>=nb)
        {
            return 0;
        }
        _videos[reference].currentAudioStream=newstream;
        for(uint32_t i=0;i<_nb_segment;i++)
                updateAudioTrack(i);
        return 1;
}

/**
______________________________________________________
//
//	Copy the start/eng seg  to clipboard
//______________________________________________________
*/
uint8_t ADM_Composer::copyToClipBoard (uint32_t start, uint32_t end)
{

  uint32_t	    seg1,    seg2,    rel1,    rel2;
  uint8_t    lastone =    0;
uint32_t seg=0xfff;

  if (end == _total_frames - 1)
    lastone = 1;

  // sanity check
  if (start > end)
  {
    printf("End < Start \n");
    return 0;
   }
  //if((1+start)==end) return 0;

  // convert frame to block, relative frame
  if (!convFrame2Seg (start, &seg1, &rel1) ||
      !convFrame2Seg (end, &seg2, &rel2))
    {
      ADM_assert (0);
    }
    _nb_clipboard=0;
  // if seg1 != seg2 we can just modify seg1 and seg2
  if (seg1 != seg2)
    {
    aprintf("Diff  seg: %"LU" /%"LU" from %"LU" to %"LU" \n",seg1,seg2,rel1,rel2);
      // remove end of seg1
	_clipboard[_nb_clipboard]._reference=_segments[seg1]._reference;
	_clipboard[_nb_clipboard]._start_frame=rel1;
 	_clipboard[_nb_clipboard]._nb_frames =_segments[seg]._nb_frames- (rel1 - _segments[seg]._start_frame);
	_nb_clipboard++;
      // copy  in between seg
      for ( seg = seg1 + 1; seg <=seg2; seg++)
		memcpy(&_clipboard[_nb_clipboard++], &_segments[seg],sizeof(_segments[0]));
      // Adjust nb frame for last seg
      uint32_t l;
      l=_nb_clipboard-1;
	_clipboard[l]._nb_frames=rel2-_segments[seg2]._start_frame;
    }
  else
    {
      // it is in the same segment, split it...
      // complete seg ?
      if ((rel1 == _segments[seg1]._start_frame)
	  && (rel2 ==
	      (_segments[seg1]._start_frame +
	       _segments[seg1]._nb_frames - 1)))
	{
	  aprintf("Full seg: %"LU" from %"LU" to %"LU" \n",seg1,rel1,rel2);
		memcpy(&_clipboard[_nb_clipboard++], &_segments[seg1],sizeof(_segments[0]));
	}
      else
	{
	  // we just take a part of one chunk
	  aprintf("Same seg: %"LU" from %"LU" to %"LU" \n",seg1,rel1,rel2);
	  memcpy(&_clipboard[_nb_clipboard], &_segments[seg1],sizeof(_segments[0]));
	  _clipboard[_nb_clipboard]._start_frame=rel1;
	  _clipboard[_nb_clipboard]._nb_frames=rel2-rel1;
	_nb_clipboard++;
	aprintf("clipboard: %"LU" \n",_nb_clipboard);
	}
    }
	dumpSeg();
  return 1;

}
uint8_t ADM_Composer::pasteFromClipBoard (uint32_t whereto)
{
uint32_t rel,seg;

	if (!convFrame2Seg (whereto, &seg, &rel) )
    	{
      		ADM_assert (0);
    	}
	dumpSeg();

	// past at frame 0
	if(	seg==0 && rel==_segments[0]._start_frame)
	{
		aprintf("Pasting at frame 0\n");
		for(uint32_t i=0;i<_nb_clipboard;i++)
			duplicateSegment(seg);
		memcpy(&_segments[0],&_clipboard[0],_nb_clipboard*sizeof(_clipboard[0]));
	}
	else
	if(rel==_segments[seg]._start_frame+_segments[seg]._nb_frames )
	{
		aprintf("\n setting at the end of seg %"LU"\n",seg);
		// we put it after OLD insert OLD+1
		for(uint32_t i=0;i<_nb_clipboard;i++)
			duplicateSegment(seg);
		memcpy(&_segments[seg+1],&_clipboard[0],_nb_clipboard*sizeof(_clipboard[0]));

	}
	else // need to split it
	{
		for(uint32_t i=0;i<_nb_clipboard+1;i++)
			duplicateSegment(seg);
		memcpy(&_segments[seg+1],&_clipboard[0],_nb_clipboard*sizeof(_clipboard[0]));

		// and the last one
		_segments[seg+_nb_clipboard+1]._nb_frames=(_segments[seg]._start_frame+_segments[seg]._nb_frames)-rel;
		_segments[seg+_nb_clipboard+1]._start_frame=rel;
		// adjust the current one
		_segments[seg]._nb_frames=rel-_segments[seg]._start_frame;
	}
	 _total_frames = computeTotalFrames ();
	for(uint32_t i=0;i<_nb_segment;i++)
 		updateAudioTrack(i);
  dumpSeg();
  return 1;

}

//____________________________________
//      Duplicate a segment
//____________________________________

uint8_t ADM_Composer::duplicateSegment (uint32_t segno)
{

  for (uint32_t i = _nb_segment; i > segno; i--)
    {

      memcpy (&_segments[i], &_segments[i - 1], sizeof (_SEGMENT));

    }
  _nb_segment++;
  return 1;


}

//____________________________________
//      Remove empty segments
//____________________________________
uint8_t ADM_Composer::crunch (void)
{
  uint32_t
    seg =
    0;
  while (seg < _nb_segment)
    {
      if (_segments[seg]._nb_frames == 0)
	{
	  //

	  for (uint32_t c = seg + 1; c < _nb_segment; c++)
	    {
	      memcpy (&_segments[c - 1], &_segments[c], sizeof (_SEGMENT));
	    }
	  _nb_segment--;

	}
      else
	{
	  seg++;
	}
    }
  // Remove last seg if there is only one frame in it
  if (_nb_segment)
    {
      if (_segments[_nb_segment - 1]._nb_frames == 1)
	{
	  _nb_segment--;
	}
    }
  return 1;

}

//____________________________________
//      Remove empty segments
//____________________________________
uint32_t ADM_Composer::computeTotalFrames (void)
{
  uint32_t
    seg,
    tf =
    0;
  for (seg = 0; seg < _nb_segment; seg++)
    {
      tf += _segments[seg]._nb_frames;

    }

  return tf;
}

//____________________________________
//      Remove empty segments
//____________________________________
void
ADM_Composer::dumpSeg (void)
{
  uint32_t seg;
  printf ("\n________Video______________");
  for (seg = 0; seg < _nb_video; seg++)
    {
//      printf ("\n Video : %"LU", nb video  :%"LU", audio size:%"LU"  audioDuration:%"LU"",
//	      seg, _videos[seg]._nb_video_frames, _videos[seg]._audio_size,_videos[seg]._audio_duration);

    }

  printf ("\n______________________");
  for (seg = 0; seg < _nb_segment; seg++)
    {
      printf
	("\n Seg : %"LU", ref: %"LU" start :%"LU", size:%"LU" audio size : %"LU" audio start : %"LU" duration:%"LLU"",
	 seg, _segments[seg]._reference, _segments[seg]._start_frame,
	 _segments[seg]._nb_frames, _segments[seg]._audio_size,
	 _segments[seg]._audio_start,
	  _segments[seg]._audio_duration
	 );

    }
  printf ("\n_________Clipboard_____________");
  for (seg = 0; seg < _nb_clipboard; seg++)
    {
      printf
	("\n Seg : %"LU", ref: %"LU" start :%"LU", size:%"LU" audio size : %"LU" audio start : %"LU"  duration:%"LLU"\n",
	 seg, _clipboard[seg]._reference, _clipboard[seg]._start_frame,
	 _clipboard[seg]._nb_frames, _clipboard[seg]._audio_size,
	 _clipboard[seg]._audio_start,
	 _segments[seg]._audio_duration);

    }


}

// Clear from position to/from
//
// 0------To------end
//  xxxxxx removed

uint8_t ADM_Composer::removeTo (uint32_t to, uint32_t seg, uint8_t included)
{
  uint32_t
    ref;

  ADM_assert (checkInSeg (seg, to));
  ref = _segments[seg]._start_frame;
  _segments[seg]._start_frame = to;
  if (included)
    _segments[seg]._start_frame++;
  _segments[seg]._nb_frames -= (_segments[seg]._start_frame - ref);


  updateAudioTrack (seg);

//---------------------------------------



  return 1;
}

//
// 0------From------end
//            xxxxxx removed

uint8_t
  ADM_Composer::removeFrom (uint32_t from, uint32_t seg, uint8_t included)
{
  ADM_assert (checkInSeg (seg, from));
  _segments[seg]._nb_frames = (from - _segments[seg]._start_frame);

  if (!included)
    _segments[seg]._nb_frames++;

  updateAudioTrack (seg);
  return 1;
}

//
//      Update the real size of audio track by computing the
// delta between sync @end and sync@begin
// We also upate the duration of the selected part
//

uint8_t ADM_Composer::updateAudioTrack (uint32_t seg)
{
  // audio sync
  uint32_t
    pos_start,
    pos_end,
    off,
    tf;
  uint32_t
    reference;

  reference = _segments[seg]._reference;
  // Mika
  if (!getTrack(reference))
    return 1;


  return 1;

#warning FIXME, does not work if audio track is shorter

}



//__________________________________________________
// check that the given frame is inside the segment
//__________________________________________________
uint8_t ADM_Composer::checkInSeg (uint32_t seg, uint32_t frame)
{
  if (frame < _segments[seg]._start_frame)
    return 0;
  if (frame > (_segments[seg]._nb_frames + _segments[seg]._start_frame))
    return 0;
  return 1;

}
uint8_t	ADM_Composer::isIndexable( void)
{
	if(!_nb_video) ADM_assert(0);
	return _videos[0].decoder->isIndexable();

}

uint8_t ADM_Composer::sanityCheck (void)
{
  uint32_t
    ref,
    seg;

  for (seg = 0; seg < _nb_segment; seg++)
    {
      ref = _segments[seg]._start_frame + _segments[seg]._nb_frames - 1;

    }
  return 1;


}

/**
	Propagate VBR building to underlying segment

*/
void
ADM_Composer::propagateBuildMap (void)
{

}
//_________________________________________
uint8_t		ADM_Composer::setEnv(_ENV_EDITOR_FLAGS newflag)
{
	_internalFlags|=newflag;
	return 1;

}
//_________________________________________
//	Return 1 if the flag was set
//		The flag is reset in all cases!!!!!!!!!!!!!
uint8_t		ADM_Composer::getEnv(_ENV_EDITOR_FLAGS newflag)
{
uint8_t r=0;
		if(_internalFlags&newflag) r=1;
		_internalFlags&=~newflag;
		if(r) { printf("Env override %d used\n",newflag);}
		return r;

}
#if BAZOOKA
//_________________________________________
//    Try indexing the file, return 1 if file successfully indexed
//              0 else
//_________________________________________
//
uint8_t         ADM_Composer::tryIndexing(const char *name, const char *idxname)
{
 unsigned int autoidx = 0;
      prefs->get(FEATURE_TRYAUTOIDX,&autoidx);
      if (!autoidx)
        {
          if (!GUI_Question (QT_TR_NOOP("This looks like mpeg\n Do you want to index it?")))
            {
                return 0;
            }
		}
          char      *idx;
          DMX_TYPE  type;
          uint32_t  nbTrack=0,audioTrack=0;
          MPEG_TRACK *tracks=NULL;
          uint8_t r=1;

                if(!dmx_probe(name,&type,&nbTrack,&tracks))
                {
                        printf("This is not mpeg\n");
                        return 0;
                }


                if(type==DMX_MPG_PS || type==DMX_MPG_TS || type==DMX_MPG_TS2)
                {
                       if(nbTrack>2)
		       if(autoidx)
			{
				printf("Using autoindex\n");
			}
/*                        else
		       {

                        if(!DIA_dmx(name,type,nbTrack,tracks,&audioTrack))
                        {
                                delete [] tracks;
                                return 0;
                        }
		       }
*/
                        audioTrack=0;
                }
		if( idxname ){
			idx=new char[strlen(idxname)];
			strcpy(idx,idxname);
		}else{
                	idx=new char[strlen(name)+5];
                	strcpy(idx,name);
                	strcat(idx,".idx");
		}

                r=dmx_indexer(name,idx,audioTrack,0,nbTrack,tracks);

                if(tracks)
                        delete [] tracks;
                delete [] idx;

                if(!r) GUI_Error_HIG(QT_TR_NOOP("Indexing failed"), NULL);
                return r;
}
#endif
/**
      If a parameter has changed, rebuild the duration of the streams
      It can happen, for example in case of SBR audio such as AAC
      The demuxer says it is xx kHz, but the codec updates it to 2*xx kHz
*/
uint8_t ADM_Composer::rebuildDuration(void)
{
  return 1;
}
/**
    \fn estimatePts
    \brief Get or estimate PTS of given frame
*/
uint64_t    ADM_Composer::estimatePts(uint32_t frame)
{
    uint32_t flags;
    _VIDEOS *vid=&_videos[0];
    vidHeader *demuxer=vid->_aviheader;
    int count=0;
    uint64_t  wantedPts;
	while(1)
    {
        demuxer->getFlags(frame,&flags);
        wantedPts=vid->_aviheader->getTime(frame);
        if((flags & AVI_KEY_FRAME)&&(wantedPts!=ADM_NO_PTS))
        {
                break;
        }
        count++;
        frame--;
    }
    wantedPts+=vid->timeIncrementInUs*count;
    return wantedPts;
}
//
//
