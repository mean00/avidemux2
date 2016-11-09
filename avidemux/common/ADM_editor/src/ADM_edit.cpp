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
#include "ADM_cpp.h"
#include "ADM_default.h"
#include "A_functions.h"
#include "ADM_audioFilterInterface.h"
#include "audioEncoderApi.h"
#include "ADM_muxerProto.h"
#include "GUI_ui.h"
#include "ADM_coreVideoFilterFunc.h"

#include <fcntl.h>
#include <errno.h>
#include <map>

#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackFromVideo.h"
#include "DIA_coreToolkit.h"
#include "prefs.h"

//#include "ADM_outputfmt.h"
#include "ADM_edPtsDts.h"
#include "ADM_vidMisc.h"
#include "ADM_confCouple.h"
#include "ADM_videoFilters.h"
#include "ADM_videoEncoderApi.h"
#include "ADM_videoFilterApi.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_coreDemuxer.h"

//#define TEST_MPEG2DEC
/**
    \fn ADM_Composer

*/
ADM_Composer::ADM_Composer (void)
{
uint32_t type,value;


  _pp=NULL;
  _imageBuffer=NULL;
  _internalFlags=0;
  _currentSegment=0;
  _scratch=NULL;
  currentProjectName=std::string("");

  _currentPts = 0;
  markerAPts = 0;
  markerBPts = 0;
  stats.reset();
}
/**
	Remap 1:1 video to segments

*/
#define YOURAUDIO(x) _videos[x].audioTracks[_videos[x].currentAudioStream]

bool        ADM_Composer::copyToClipBoard(uint64_t startTime, uint64_t endTime)
{
    return _segments.copyToClipBoard(startTime,endTime);
    
}
bool        ADM_Composer::pasteFromClipBoard(uint64_t currentTime)
{
    return _segments.pasteFromClipBoard(currentTime);
    
}
/**
    \fn appendFromClipBoard
    \brief Append the content of the clipboard to the end of the video
*/
bool ADM_Composer::appendFromClipBoard(void)
{
    return _segments.appendFromClipBoard();
}

/**
    \fn resetSeg
    \brief Redo a 1:1 mapping between ref video and segment
*/
uint8_t ADM_Composer::resetSeg( void )
{
	_segments.resetSegment();
	return 1;
}
/**
    \fn getExtraHeaderData
    \brief	Return extra Header info present in avi chunk that are needed to initialize
	the video codec
	It is assumed that there is only one file or can share the same init data
	(huffyuv for example)
*/
bool ADM_Composer::getExtraHeaderData (uint32_t * len, uint8_t ** data)
{
    if(_segments.getNbRefVideos())
        return _segments.getRefVideo(0)->_aviheader->getExtraHeaderData (len, data);
    *len=0;
    *data=NULL;
    return false;
}


/**
    \fn ADM_Composer
*/
ADM_Composer::~ADM_Composer ()
{

    cleanup();

    if(_pp) delete _pp;
    _pp=NULL;
   

}
/**
    \fn getProjectName
*/
const std::string &ADM_Composer::getProjectName(void)
{
    return currentProjectName;
}

/**
    \fn setProjectName
*/
bool ADM_Composer::setProjectName(const std::string &pj)
{
    currentProjectName=pj;
    return true;
}
/**
    \fn addFile
    \brief	Load or append a file.	The file type is determined automatically and the ad-hoc video decoder is spawned

    @param name: filename
    @return 1 on success, 0 on failure


*/
bool ADM_Composer::addFile (const char *name)
{

  uint8_t    ret =    0;
  aviInfo    info;
  uint32_t   magic;
  _VIDEOS video;
  bool		 avisynth_used = false;

	if(!strcmp(name, AVS_PROXY_DUMMY_FILE))
	{
        magic=0;
        avisynth_used = true;
	}
    else
    {
        FILE *f=ADM_fopen(name,"r");
        uint8_t buffer[4];
        if(!f) return 0;
        fread(buffer,4,1,f);
        fclose(f);
        magic=(buffer[3]<<24)+(buffer[2]<<16)+(buffer[1]<<8)+(buffer[0]);
    }

	// First find the demuxer....
	video._aviheader=ADM_demuxerSpawn(magic,name);
	if(!video._aviheader)
	{
		char str[512+1];
		snprintf(str,512,QT_TRANSLATE_NOOP("ADM_Composer","Cannot find a demuxer for %s"), name);
		str[512] = '\0';
		GUI_Error_HIG(str,NULL);
		return false;
	}

	// use "file name" to share the port number (using little endian)
	if(avisynth_used)
	{
		uint32_t portValue = 0;
		bool useCmdLine = A_getCommandLinePort(portValue);
		if (!useCmdLine)
			if(!prefs->get(AVISYNTH_AVISYNTH_DEFAULTPORT, &portValue))
				portValue = 9999;

		bool askPortAvisynth = false;
		prefs->get(AVISYNTH_AVISYNTH_ALWAYS_ASK, &askPortAvisynth);
		if(askPortAvisynth)
		{
			uint32_t localPort = 0;
			if (useCmdLine || !prefs->get(AVISYNTH_AVISYNTH_LOCALPORT, &localPort) || localPort == 0)
				localPort = portValue;
			if (UI_askAvisynthPort(localPort)) {
				portValue = localPort;
				prefs->set(AVISYNTH_AVISYNTH_LOCALPORT, localPort);
			}
		}
		uint8_t dummy[] = { (uint8_t)portValue ,(uint8_t)( portValue >> 8)};
		ret = video._aviheader->open((char *)dummy);
	}
	else
		ret = video._aviheader->open(name);

   // check opening was successful
   if (ret == 0) {
     char str[512+1];
     snprintf(str,512,QT_TRANSLATE_NOOP("ADM_Composer","Attempt to open %s failed!"), name);
      str[512] = '\0';
      GUI_Error_HIG(str,NULL);
      video._aviheader=NULL;
      return false;
   }

   /* check for resolution */
   if( _segments.getNbRefVideos())
    {
      /* append operation */
      aviInfo info0, infox;
      _segments.getRefVideo(0)->_aviheader->getVideoInfo (&info0);
      video._aviheader->getVideoInfo (&infox);
      if( info0.width != infox.width || info0.height != infox.height )
        {
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
         GUI_Error_HIG(str,QT_TRANSLATE_NOOP("ADM_Composer","You cannot mix different video dimensions yet. Using the partial video filter later, will not work around this problem. The workaround is:\n1.) \"resize\" / \"add border\" / \"crop\" each stream to the same resolution\n2.) concatenate them together"));
         delete video._aviheader;
         return false;
      }
   }
  // else update info
  video._aviheader->getVideoInfo (&info);
  video._aviheader->setMyName (name);

  // Printf some info about extradata

    uint32_t l=0;
    uint8_t *d=NULL;
    video._aviheader->getExtraHeaderData(&l,&d);
    if(l && d)
    {
        printf("[Editor]The video codec has some extradata (%d bytes)\n",l);
        mixDump(d,l);
        printf("\n");
    }

  // 1st if it is our first video we update postproc
    if(!_segments.getNbRefVideos())
    {
        uint32_t type=0,value=0;
#if 0
        if(!prefs->get(DEFAULT_POSTPROC_TYPE,&type)) type=3;
        if(!prefs->get(DEFAULT_POSTPROC_VALUE,&value)) value=3;
#endif
        if(_pp) delete _pp;
        _pp=new ADM_PP(info.width,info.height);
        _pp->postProcType=type;
        _pp->postProcStrength=value;
        _pp->forcedQuant=0;
        _pp->update();

        if(_imageBuffer)
        {
            if(_imageBuffer->quant)
                delete [] _imageBuffer->quant;
            _imageBuffer->quant=NULL;
            delete _imageBuffer;
            _imageBuffer=NULL;
        }
        _imageBuffer=new ADMImageDefault(info.width,info.height);
        _imageBuffer->_qSize= ((info.width+15)>>4)*((info.height+15)>>4);
        _imageBuffer->quant=new uint8_t[_imageBuffer->_qSize];
        memset(_imageBuffer->quant,0,_imageBuffer->_qSize);
        _imageBuffer->_qStride=(info.width+15)>>4;

        // We also clear the filter queue...
        ADM_info("Clearing video filters\n");
        ADM_vf_clearFilters();
    }


  // Update audio infos
  // an spawn the appropriate decoder
  //_________________________
   uint32_t nbAStream=video._aviheader->getNbAudioStreams();

  if (!nbAStream)
    {
      printf ("[Editor] *** NO AUDIO ***\n");
      video.currentAudioStream=0;
    }
  else
    {
        // Read and construct the audio tracks for that videos
      audioInfo *info;
      uint32_t extraLen;
      uint8_t  *extraData;
      ADM_audioStream *stream;
      WAVHeader *header;

      _VIDEOS *thisVid=&(video);
      // Create streams

      for(int i=0;i<nbAStream;i++)
      {
            ADM_audioStreamTrack *track=new ADM_audioStreamTrack;

            header=thisVid->_aviheader->getAudioInfo(i );
            memcpy(&(track->wavheader),header,sizeof(*header));

            thisVid->_aviheader->getAudioStream(i,&stream);
            ADM_assert(stream);
            track->stream=stream;

            track->duration=stream->getDurationInUs();
            track->size=0;

            stream->getExtraData(&extraLen,&extraData);
            track->codec=getAudioCodec(header->encoding,header,extraLen,extraData);

            thisVid->audioTracks.push_back(track);
            if(!_segments.getNbRefVideos()) // 1st video..
            {
                ADM_edAudioTrackFromVideo *trackFromVideo=new ADM_edAudioTrackFromVideo(track,i,this);
                audioTrackPool.addInternalTrack(trackFromVideo);
            }
      }
      if(!_segments.getNbRefVideos()) // only for 1st video
      {
        activeAudioTracks.clear();
        for(int i=0;i<nbAStream;i++)
            activeAudioTracks.addTrack(i,audioTrackPool.at(i)); // default add 1st track of video pool
      }
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
    
    _segments.addReferenceVideo(&video);
    
    // we only try if we got everything needed...
    // Verify DTS is monotonous
    ADM_verifyDts(video._aviheader,video.timeIncrementInUs);
    if(!video.decoder)
    {
        ADM_info("[Editor] no decoder to check for B- frame\n");
    }else
    {
        if(video.decoder->bFramePossible())
        {
            printf("[Editor] B- frame possible with that codec \n");
            if(isMpeg4Compatible(info.fcc) || isMpeg12Compatible(info.fcc) || isVC1Compatible(info.fcc))
            {
                ADM_info("[Editor] It is mpeg4-SP/ASP, try to guess all PTS\n");
                uint64_t delay;
                ADM_computeMP124MissingPtsDts(video._aviheader,video.timeIncrementInUs,&delay);
                _segments.updateRefVideo();


            }else
            {
                if(isH264Compatible(info.fcc))
                {
                    uint64_t delay;
                    ADM_info("[Editor] This is H264, check if we can fill missing PTS\n");
                    ADM_setH264MissingPts(video._aviheader,video.timeIncrementInUs,&delay);
                }else
                    ADM_info("[Editor] Not H264\n");
            }
        }
        else
        {
                printf("[Editor] No B frame with that codec, PTS=DTS\n");
                setPtsEqualDts(video._aviheader,video.timeIncrementInUs);
                _segments.updateRefVideo();
        }
     }
    int lastVideo=_segments.getNbSegments();
    if(lastVideo && video._aviheader->unreliableBFramePts()&& isH264Compatible(info.fcc))
    {
        ADM_info("H264 in mp4 sometimes has invalid timestamps which confuse avidemux, checking\n");
        checkForValidPts(_segments.getSegment(lastVideo-1)); 
    }
    if(true==checkForDoubledFps( video._aviheader,video.timeIncrementInUs))
    {
        ADM_info("Halving Fps...\n");
        _segments.halfFps();
    }
  return 1;
}
#if 0
/**
    \fn hasVBRAudio
        If one of the videos has VBR audio we handle the whole editor audio has VBR
        If it is CBR, it is not harmful
        and it avoid loosing the VBR info in case we do VBR time map upon loading
*/
bool ADM_Composer::hasVBRAudio(void)
{
    int n=_segments.getNbRefVideos();
        for(int i=0;i<n;i++)
        {
                ADM_audioStreamTrack *trk=getTrack(i);
                if(trk)
                    if(trk->vbr) return 1;
        }
        return 0;
}
#endif
/**
    \fn getPARWidth

*/
uint32_t ADM_Composer::getPARWidth()
{
  if (_segments.getNbRefVideos())
  {
    return _segments.getRefVideo(0)->decoder->getPARWidth();
  }
  return 1;

}
/**
    \fn getPARHeight

*/

uint32_t ADM_Composer::getPARHeight()
{
  if (_segments.getNbRefVideos())
  {
    return _segments.getRefVideo(0)->decoder->getPARHeight();
  }
  return 1;

}
/**
    \fn getFrameIncrement
*/
uint64_t ADM_Composer::getFrameIncrement(void)
{
    if (!_segments.getNbSegments()) return 0;
    return _segments.getRefVideo(0)->timeIncrementInUs;
}
/**
	Set decoder settings (post process/swap u&v...)
	for the segment referred by frame

*/
bool ADM_Composer::setDecodeParam (uint64_t time)
{
uint32_t ref;
  if (_segments.getNbRefVideos())
  {
    if(false==_segments.getRefFromTime(time,&ref))
    {
        ADM_warning("Cannot get ref from time %" PRId64" ms\n",time/1000);
        return false;
    }
    _segments.getRefVideo(ref)->decoder->setParam ();
  }
  return true;

}

/**
	Free all allocated memory and destroy all editors object


*/
uint8_t ADM_Composer::cleanup (void)
{
    if(_scratch)
        delete  _scratch;
    _scratch=NULL;
    if(_imageBuffer)
     {
         if(_imageBuffer->quant)
             delete [] _imageBuffer->quant;
         _imageBuffer->quant=NULL;
         delete  _imageBuffer;
         _imageBuffer=NULL;
     }  
  
  _segments.deleteAll();
  _currentPts = 0;
  markerAPts = 0;
  markerBPts = 0;
  audioTrackPool.clear();
  activeAudioTracks.clear();
  return 1;
}

/**
    \fn getAudioStreamsInfo
    \brief Returns a copy of all audio trackes at frame frame
    call delete [] infos when you dont need them anymore
*/
bool ADM_Composer::getAudioStreamsInfo(uint64_t xtime,uint32_t *nbStreams, audioInfo **infos)
{

uint32_t ref;

    if(false==_segments.getRefFromTime(xtime,&ref))
    {
        ADM_warning("[Editor] getAudioStreamsInfo failed for time %" PRId64" ms\n",xtime);
        return false;
    }

    _VIDEOS *v=_segments.getRefVideo(ref);
    ADM_assert(v);
    int nb=v->audioTracks.size();
    if(!nb)
    {
        *nbStreams=0;
        *infos=NULL;
        return true;
    }

    *nbStreams=nb;
    *infos=new audioInfo[nb];
    for(int i=0;i<nb;i++)
    {
        WAVHeader *wav=&(v->audioTracks[i]->wavheader);
        audioInfo *t=(*infos)+i;
        t->bitrate=(wav->byterate*8)/1000;
        t->channels=wav->channels;
        t->encoding=wav->encoding;
        t->frequency=wav->frequency;
        t->av_sync=0;
     }
    return true;
}
/**
    \fn getCurrentAudioStreamNumber
    \brief Get stream number used for frame at position xtime

*/
uint32_t ADM_Composer::getCurrentAudioStreamNumber(uint64_t  xtime)
{
uint32_t ref;

        if(false==_segments.getRefFromTime(xtime,&ref))
        {
            ADM_warning("[Editor::getCurrentAudioStreamNumber] Cannot get ref video for time %" PRId64" ms\n",xtime/1000);
            return 0;
        }

        return _segments.getRefVideo(ref)->currentAudioStream;
}
/**
        \fn changeAudioStream

*/
bool ADM_Composer::changeAudioStream(uint64_t xtime,uint32_t newstream)
{
double     duration;
WAVHeader *wav;
aviInfo    info;
uint32_t ref;
        int n=_segments.getNbRefVideos();
        for(int i=0;i<n;i++)
        {
            _VIDEOS *v=_segments.getRefVideo(i);
            uint32_t nb=v->audioTracks.size();
            if(newstream>=nb)
            {
                ADM_warning("[Editor::changeAudioStream] New stream exceeds # of stream (%d/%d)\n",(int)newstream,(int)nb);
                continue;
            }
            v->currentAudioStream=newstream;
            if(!i) // update general info for track 0
            {
//                wavHeader.frequency=v->audioTracks[newstream]->wavheader.frequency;
//                wavHeader.channels=v->audioTracks[newstream]->wavheader.channels;
            }
            ADM_info("Switched to track %d for video %d\n",newstream,i);
        }
        return true;
}

//
//      Update the real size of audio track by computing the
// delta between sync @end and sync@begin
// We also upate the duration of the selected part
//

uint8_t ADM_Composer::updateAudioTrack (uint32_t seg)
{
#if 0
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

#endif
  return 1;

//#warning FIXME, does not work if audio track is shorter

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

/**
    \fn rebuildDuration
    \brief      If a parameter has changed, rebuild the duration of the streams
      It can happen, for example in case of SBR audio such as AAC
      The demuxer says it is xx kHz, but the codec updates it to 2*xx kHz

    Should not be needed with 2.6
*/
bool  ADM_Composer::rebuildDuration(void)
{
  return true;
}
/**
    \fn getCurrentFramePts
    \brief Get the PTS of current frame

*/
uint64_t    ADM_Composer::getCurrentFramePts(void)
{
   return _currentPts;
}
/**
    \fn remove
    \brief Remove part of the video
*/
bool            ADM_Composer::remove(uint64_t start,uint64_t end)
{
    return _segments.removeChunk(start,end);
}
/**
    \fn truncate
    \brief Truncate video
*/
bool ADM_Composer::truncate(uint64_t start)
{
    return _segments.truncateVideo(start);
}
/**
    \fn dumpEditing
    \brief Dump segment, video & al
*/
void            ADM_Composer::dumpSegments(void)
{
    _segments.dump();
}
void                ADM_Composer::dumpSegment(int i)
{
    _segments.dumpSegment(i);
}
/**

*/
bool ADM_Composer::dumpRefVideos(void)
{
    _segments.dumpRefVideos();
    return true;
}
/**
    \fn dumpTiming
*/
bool               ADM_Composer::dumpTiming(void)
{
    if(!_segments.getNbRefVideos()) return true;
    _VIDEOS *v=_segments.getRefVideo(0);

    aviInfo info;
    v->_aviheader->getVideoInfo(&info);
    int nb=info.nb_frames;
    for(int i=0;i<nb;i++)
    {
        uint64_t pts,dts;
        uint32_t flags;

            v->_aviheader->getFlags(i,&flags);
            v->_aviheader->getPtsDts(i,&pts,&dts);
            printf("%" PRIu32" flags:%04" PRIx32" ",i,flags);
            printf("pts :%s ",ADM_us2plain(pts));
            printf("dts :%s \n",ADM_us2plain(dts));
    }
    return true;
}
/**
     \fn getVideoPtsDts
*/
bool                ADM_Composer::getVideoPtsDts(uint32_t frame, uint32_t *flags,uint64_t *pts, uint64_t *dts)
{
    if(!_segments.getNbRefVideos()) return true;
    _VIDEOS *v=_segments.getRefVideo(0);

    aviInfo info;
    v->_aviheader->getVideoInfo(&info);
    int nb=info.nb_frames;
    if(frame>=nb)
    {
        return false;
    }
    v->_aviheader->getFlags(frame,flags);
    v->_aviheader->getPtsDts(frame,pts,dts);
    return true;
}
/**
    \fn getVideoDecoderName
*/
const char          *ADM_Composer::getVideoDecoderName(void)
{
    if(!_segments.getNbRefVideos()) return "????";
     _VIDEOS *v=_segments.getRefVideo(0);
    if(!v) return "????";
    if(!v->decoder) return "????";
    return v->decoder->getDecoderName();
}
/**
 *      \fn getFrameSize
 *      \brief return frame size in bytes of frame "frame"
 */
uint32_t           ADM_Composer::getFrameSize(int frame)
{
    if(!_segments.getNbRefVideos()) return 0;
    _VIDEOS *v=_segments.getRefVideo(0);
    if(!v) return 0;
    aviInfo info;
    v->_aviheader->getVideoInfo(&info);
    int nb=info.nb_frames;
    if(frame>=nb)
    {
        return 0;
    }
    uint32_t sz=0;
    v->_aviheader->getFrameSize(frame,&sz);
    return sz;
}

static  std::map<std::string, std::string> scriptEnv;

/**
 * \fn setVar
 * @param key
 * @param value
 * @return 
 */
bool            ADM_Composer::setVar(const char *key, const char *value)
{
    std::map<std::string,std::string>::iterator it;
    it=scriptEnv.find(std::string(key));
    if(scriptEnv.end() !=it)
    {
        scriptEnv.erase(it);
    }
    scriptEnv.insert(std::pair<std::string, std::string>(key,value));
    return true;
}
/**
 * \fn getVar
 * @param key
 * @return 
 */
const char      *ADM_Composer::getVar(const char *key)
{
    std::map<std::string,std::string>::iterator it;
    it=scriptEnv.find(std::string(key));
    if(scriptEnv.end() !=it)
    {
        return scriptEnv[key].c_str(); // ??
    }
    return NULL;
}
/**
 * \fn printEnv
 * @param 
 * @return 
 */
bool       ADM_Composer::printEnv(void)
{
    std::map<std::string,std::string>::iterator it;
    for(it=scriptEnv.begin();it!=scriptEnv.end();it++)
    {
        printf("%s => %s\n",it->first.c_str(),it->second.c_str());
    }
    return true;
}

// EOF
