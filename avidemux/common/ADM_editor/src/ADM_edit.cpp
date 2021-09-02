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
#include "GUI_ui.h"

#include <fcntl.h>
#include <errno.h>
#include <map>

#include "fourcc.h"
#include "ADM_edit.hxx"
#include "ADM_edAudioTrackFromVideo.h"
#include "DIA_coreToolkit.h"
#include "prefs.h"

#include "ADM_edPtsDts.h"
#include "ADM_vidMisc.h"
#include "ADM_confCouple.h"
#include "ADM_videoFilters.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_coreDemuxer.h"
#include "ADM_aacinfo.h"

//#define TEST_MPEG2DEC
/**
    \fn ADM_Composer

*/
ADM_Composer::ADM_Composer (void)
{
  _pp=NULL;
  _imageBuffer=NULL;
  _internalFlags=0;
  _currentSegment=0;
  _scratch=NULL;
  _undo_counter=0;
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
    \fn clipboardEmpty
    \brief Return true if we have nothing to paste or append
*/
bool ADM_Composer::clipboardEmpty(void)
{
    return _segments.clipboardEmpty();
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
    \fn checkStdTimeBase
    \brief Try to replace timebase denominator with a standard value.
*/
static bool checkStdTimeBase(uint32_t &num, uint32_t &den)
{
    if(!num || !den)
        return false;
    if(den>num && !(den%num))
    {
        switch(den/num)
        {
            case 24: case 25: case 30: case 50: case 60:
                den/=num;
                num=1000;
                den*=1000;
                return true;
            default:break;
        }
    }
#define MAX_STD_CLOCK_FREQ 90000
    switch(den)
    {
        case 24000: case 25000: case 30000: case 50000: case 60000:
            return true; // nothing to do
        case 48000:
            if(num>1 && !(num&1))
            {
                num>>=1;
                den>>=1;
                return true;
            }
            break;
        case 90000:
            if(num>=18 && !(num%18))
            {
                num/=18;
                num*=5;
                den=25000;
                break;
            }
            if(num>=3 && !(num%3))
            {
                num/=3;
                den=30000;
            }
            break;
        case 180000:
            if(num>1 && !(num&1))
            {
                num>>=1;
                den=90000;
                return checkStdTimeBase(num,den);
            }
            break;
        case 10000000:
            {
                den/=10;
                num/=10;
                if(!num) num=1;
                return checkStdTimeBase(num,den);
            }
            break;
        case 1000000:
            {
                int n,d;
                usSecondsToFrac(num,&n,&d,MAX_STD_CLOCK_FREQ);
                num=n;
                den=d;
            }
            break;
        case 600: // libavformat doesn't like low timebase clocks
            num*=50;
            den=30000;
            break;
        case 1200:
            num*=25;
            den=30000;
            return true;
        case 2400:
            num*=10;
            den=24000;
            return true;
        default: break;
    }
    /* some old AVI samples clock insanely high */
    if(den>=1000000)
    {
        double us=num;
        us*=1000.;
        us/=den;
        us*=1000.;
        us+=0.49;
        int n,d;
        usSecondsToFrac((uint64_t)us,&n,&d,MAX_STD_CLOCK_FREQ);
        num=n;
        den=d;
    }
    return true;
}

/**
    \fn getTimeBase
*/
bool ADM_Composer::getTimeBase(uint32_t *scale, uint32_t *rate, bool copyMode)
{
    /* Using MPEG timescale as fallback */
    *scale=1;
    *rate=MAX_STD_CLOCK_FREQ;
    /* Which segments does the selection span? */
    uint64_t segTime,start,end,total;
    start=getMarkerAPts();
    end=getMarkerBPts();
    total=_segments.getTotalDuration();
    if(!total)
        return false;
    if(end >= total)
        end = total-1;
    if(start>end)
    {
        uint64_t tmp=end;
        end=start;
        start=tmp;
    }
    uint32_t firstSeg,lastSeg;
    if(false==_segments.convertLinearTimeToSeg(start,&firstSeg,&segTime))
        return false;
    if(false==_segments.convertLinearTimeToSeg(end,&lastSeg,&segTime))
        return false;

    /* Which unique ref videos do we need to check for timebase compatibility? */
    std::vector <uint32_t> ListOfRefs;

    for(uint32_t i=firstSeg; i<=lastSeg; i++)
    {
        _SEGMENT *s=_segments.getSegment(i);
        ADM_assert(s);
        bool skip=false;
        for(int i=0;i<ListOfRefs.size();i++)
        {
            if(ListOfRefs.at(i)==s->_reference)
                skip=true;
        }
        if(skip) continue;
        ListOfRefs.push_back(s->_reference);
    }
    if(ListOfRefs.empty())
        return false;
    uint32_t myscale,myrate;
    for(int i=0;i<ListOfRefs.size();i++)
    {
        _VIDEOS *vid=_segments.getRefVideo(ListOfRefs.at(i));
        ADM_assert(vid);
        ADM_info("Ref video %u is %s-encoded, copy mode: %d\n",ListOfRefs.at(i),vid->fieldEncoded? "field" : "frame",copyMode);
        vidHeader *demuxer=vid->_aviheader;
        ADM_assert(demuxer);
        aviInfo info;
        demuxer->getVideoInfo(&info);
        if(!i)
        {
            myscale=info.timebase_num;
            myrate=info.timebase_den;
            if(false==checkStdTimeBase(myscale,myrate))
                return false;
            if(vid->fieldEncoded && !copyMode)
            {
                switch(myrate)
                {
                    case 60000:
                    case 50000:
                    case 48000:
                        ADM_info("Halving timebase denominator %u for field-encoded ref video.\n",myrate);
                        myrate/=2;
                        break;
                    default:
                        ADM_info("Doubling timebase numerator %u for field-encoded ref video.\n",myscale);
                        myscale*=2;
                        break;
                }
            }
            continue;
        }
        /* FIXME properly reduce fractions when checking timebase */
        if(false==checkStdTimeBase(info.timebase_num,info.timebase_den))
            return false;
        if(vid->fieldEncoded && !copyMode)
        {
            switch(info.timebase_den)
            {
                case 60000:
                case 50000:
                case 48000:
                    ADM_info("Halving timebase denominator %u for appended field-encoded ref video.\n",info.timebase_den);
                    info.timebase_den/=2;
                    break;
                default:
                    ADM_info("Doubling timebase numerator %u for appended field-encoded video.\n",info.timebase_num);
                    info.timebase_num*=2;
                    break;
            }
        }
        if(info.timebase_den == myrate)
        {
            if(info.timebase_num < myscale)
            {
                if(info.timebase_num > 1 && !(myscale % info.timebase_num))
                    myscale = info.timebase_num;
                else
                    myscale = 1;
            }
        }else if(info.timebase_den > myrate)
        {
            if(!(info.timebase_den % myrate))
            {
                uint32_t mult = info.timebase_den / myrate;
                myrate = info.timebase_den;
                myscale *= mult;
            }else
            {
                ADM_warning("Timebase mismatch: %u / %u (new) %u / %u (old)\n",info.timebase_num,info.timebase_den,myscale,myrate);
                myrate = MAX_STD_CLOCK_FREQ;
                myscale = 1;
                break;
            }
        }else
        {
            if(myrate % info.timebase_den)
            {
                ADM_warning("Timebase mismatch: %u / %u (new) %u / %u (old)\n",info.timebase_num,info.timebase_den,myscale,myrate);
                myrate = MAX_STD_CLOCK_FREQ;
                myscale = 1;
                break;
            }
        }
    }
    ADM_info("Timebase set to %u / %u\n",myscale,myrate);
    *scale=myscale;
    *rate=myrate;

    return true;
}

/**
    \fn addFile
    \brief	Load or append a file.	The file type is determined automatically and the ad-hoc video decoder is spawned

    @param name: filename
    @return 1 on success, 0 on failure, ADM_IGN when cancelled by user.


*/
uint8_t ADM_Composer::addFile (const char *name)
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

    if(ret==ADM_IGN)
    {
        ADM_info("Cancelled by user\n");
        return ret;
    }

    bool first = !_segments.getNbRefVideos();

   /* check for resolution */
   if(!first)
    {
      /* append operation */
      aviInfo info0, infox;
      _segments.getRefVideo(0)->_aviheader->getVideoInfo (&info0);
      video._aviheader->getVideoInfo (&infox);
      if( info0.width != infox.width || info0.height != infox.height )
        {
         char str[512+1];
         str[0] = '\0';
         int both=-1;
         if( info0.width != infox.width )
         {
            both++;
            strcpy(str,QT_TRANSLATE_NOOP("ADM_Composer","width"));
         }
         if( info0.height != infox.height )
         {
            both++;
            snprintf(str+strlen(str),512-strlen(str),
                QT_TRANSLATE_NOOP("ADM_Composer","%sheight"),
                (both? QT_TRANSLATE_NOOP("ADM_Composer"," and ") : ""));
         }
         snprintf(str+strlen(str),512-strlen(str),
            QT_TRANSLATE_NOOP("ADM_Composer","%sdifferent between first and this video stream"),
            (both? QT_TRANSLATE_NOOP("ADM_Composer"," are ") : QT_TRANSLATE_NOOP("ADM_Composer"," is ")));
         str[512] = '\0';
         GUI_Error_HIG(str,QT_TRANSLATE_NOOP("ADM_Composer","You cannot mix different video dimensions yet. Using the partial video filter later, will not work around this problem. The workaround is:\n1.) \"resize\" / \"add border\" / \"crop\" each stream to the same resolution\n2.) concatenate them together"));
         delete video._aviheader;
         return false;
      }
   }
  // else update info
  video._aviheader->getVideoInfo (&info);
  if(info.width > MAXIMUM_SIZE || info.height > MAXIMUM_SIZE)
  {
        char str[512];
        str[0] = '\0';
        if(info.width > MAXIMUM_SIZE && info.height <= MAXIMUM_SIZE)
        {
            snprintf(str,512,QT_TRANSLATE_NOOP("ADM_Composer","The width of the video %u px exceeds maximum supported width %u.\n"),
                info.width,MAXIMUM_SIZE);
        }else if(info.width <= MAXIMUM_SIZE && info.height > MAXIMUM_SIZE)
        {
            snprintf(str,512,QT_TRANSLATE_NOOP("ADM_Composer","The height of the video %u px exceeds maximum supported height %u.\n"),
                info.height,MAXIMUM_SIZE);
        }else // both
        {
            snprintf(str,512,QT_TRANSLATE_NOOP("ADM_Composer","Video dimensions %ux%u exceed maximum supported size %ux%u.\n"),
                info.width,info.height,MAXIMUM_SIZE,MAXIMUM_SIZE);
        }
        str[511] = '\0';
        GUI_Error_HIG(QT_TRANSLATE_NOOP("ADM_Composer","Unsupported size"),str);
        delete video._aviheader;
        video._aviheader=NULL;
        return 0;
  }
  video._aviheader->setMyName (name);

  // 1st if it is our first video we update postproc
    if(first)
    {
        uint32_t type=0,value=0;
        prefs->get(DEFAULT_POSTPROC_TYPE,&type);
        prefs->get(DEFAULT_POSTPROC_VALUE,&value);

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
      uint32_t extraLen=0;
      uint8_t  *extraData;
      ADM_audioStream *stream;
      WAVHeader *header;

      _VIDEOS *thisVid=&(video);
      // Create streams

      for(int i=0;i<nbAStream;i++)
      {
            ADM_audioStreamTrack *track=new ADM_audioStreamTrack;

            header=thisVid->_aviheader->getAudioInfo(i );

            if(header->encoding==0x706d)
            {
                ADM_info("Mapping codec ID 0x706d to AAC.\n");
                header->encoding=WAV_AAC;
            }

            memcpy(&(track->wavheader),header,sizeof(*header));

            thisVid->_aviheader->getAudioStream(i,&stream);
            ADM_assert(stream);
            track->stream=stream;

            track->duration=stream->getDurationInUs();
            track->size=0;

            stream->getExtraData(&extraLen,&extraData);
            if(extraLen)
            {
                track->extraCopy=new uint8_t[extraLen];
                memcpy(track->extraCopy,extraData,extraLen);
                track->extraCopyLen=extraLen;
            }
            track->codec=getAudioCodec(header->encoding,header,extraLen,extraData);

            // sanity check
#define SKIP_TRACK delete track; track=NULL; continue;
            if(!track->codec) // actually this can't happen, but better safe than sorry
            {
                ADM_warning("Codec is NULL, rejecting track %d\n",i);
                SKIP_TRACK
            }
            if((track->wavheader).frequency < MIN_SAMPLING_RATE || (track->wavheader).frequency > MAX_SAMPLING_RATE)
            {
                ADM_warning("Sampling frequency %" PRIu32" for track %d out of bounds, rejecting track.\n",(track->wavheader).frequency,i);
                SKIP_TRACK
            }
            if(!track->codec->getOutputChannels() || track->codec->getOutputChannels() > MAX_CHANNELS)
            {
                ADM_warning("Invalid number of channels %d for track %d, rejecting track.\n",track->codec->getOutputChannels(),i);
                SKIP_TRACK
            }

            if((track->wavheader).encoding==WAV_AAC || (track->wavheader).encoding==WAV_DTS)
            {
                checkSamplingFrequency(track);
                track->codec->reconfigureCompleted();
            }

            if((track->wavheader).encoding==WAV_AAC)
            {
                uint32_t samples=AAC_DEFAULT_FRAME_LENGTH;
                AacAudioInfo aac;
                if(track->isbr || (ADM_getAacInfoFromConfig(extraLen,extraData,aac) && aac.sbr))
                    samples<<=1;
                stream->setSamplesPerPacket(samples);
            }

            thisVid->audioTracks.push_back(track);
            if(first) // 1st video..
            {
                ADM_edAudioTrackFromVideo *trackFromVideo=new ADM_edAudioTrackFromVideo(track,i,this);
                audioTrackPool.addInternalTrack(trackFromVideo);
            }
      }
      if(first) // only for 1st video
      {
        activeAudioTracks.clear();
        for(int i=0;i<audioTrackPool.size();i++)
            activeAudioTracks.addTrack(i,audioTrackPool.at(i)); // default add 1st track of video pool
      }
    }

    // ugly hack
    bool fpsTooHigh=false;
    // we switch to 25 fps hardcoded in addReferenceVideo and update info here
    if (info.fps1000 > 2000 * 1000)
    {
        ADM_warning("FPS too high, switching to 25 fps hardcoded\n");
        fpsTooHigh=true;
        info.fps1000 = 25 * 1000;
    }

    if(false == _segments.addReferenceVideo(&video))
    {
        ADM_warning("Cannot add video.\n");
        delete video._aviheader;
        video._aviheader = NULL;
        for(uint32_t i = 0; i < video.audioTracks.size(); i++)
        {
            ADM_audioStreamTrack *track = video.audioTracks[i];
            video.audioTracks[i] = NULL;
            if(track)
                delete track;
            track = NULL;
        }
        video.audioTracks.clear();
        if(first)
            cleanup();
        return 0;
    }

    if(fpsTooHigh)
        updateVideoInfo(&info);

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
#define FCC_MATCHES(x) fourCC::check(info.fcc,(uint8_t *)x)
            if(isMpeg4Compatible(info.fcc) || isMpeg12Compatible(info.fcc) || FCC_MATCHES("VC1 ") || FCC_MATCHES("WMV3"))
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
    if(lastVideo && (isH264Compatible(info.fcc) || FCC_MATCHES("WVC1")))
    {
        ADM_info("%s sometimes has invalid timestamps which confuse avidemux, checking\n",fourCC::tostring(info.fcc));
        checkForValidPts(_segments.getSegment(lastVideo-1)); 
    }
    if(!video.fieldEncoded && checkForDoubledFps(video._aviheader,video.timeIncrementInUs))
    {
        ADM_info("Halving Fps...\n");
        _segments.halfFps();
    }
    endOfStream=false;
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
    \fn checkSamplingFrequency
    \brief Check AAC for implicit SBR
*/
bool ADM_Composer::checkSamplingFrequency(ADM_audioStreamTrack *track)
{
    if(!track) return false;
    if(!track->codec) return false;
    if(track->codec->isDummy()) return false;

    WAVHeader *hdr=&(track->wavheader);

    uint32_t len=hdr->channels*hdr->frequency; // 1 sec, should be enough
    uint32_t max=ADM_EDITOR_PACKET_BUFFER_SIZE;

    notStackAllocator inbuf(max);
    uint8_t *in=inbuf.data;

    uint32_t inlen,samples;
    uint64_t dts;
    if(false==track->stream->getPacket(in,&inlen,max,&samples,&dts) || !inlen)
        return false;

    notStackAllocator outbuf(len*sizeof(float));
    float *out=(float *)outbuf.data;

    uint32_t nbOut;
    if(false==track->codec->run(in,inlen,out,&nbOut))
        return false;

    if(!nbOut)
    {
        uint32_t extraLen=0;
        uint8_t *extraData;
        track->stream->getExtraData(&extraLen,&extraData);
        if(extraLen)
        {
            /* The worst outcome: the sampling frequency and number of channels
            are invalid, but we probably don't have the right extradata to
            decode it and to get the info we need in the first place. */
            ADM_warning("Could not decode audio packet, wrong extradata?\n");
            return false;
        }
        ADM_warning("Could not decode audio packet to verify sampling frequency and number of channels.\n");
        return false;
    }

    uint32_t fq=track->codec->getOutputFrequency();
    if(fq && fq!=hdr->frequency)
    {
        ADM_warning("Updating sampling frequency from %u to %u\n",hdr->frequency,fq);
        hdr->frequency=fq;
        track->isbr=true;
    }
    uint32_t chan=track->codec->getOutputChannels();
    if(chan && chan!=(uint32_t)hdr->channels)
    {
        ADM_warning("Updating number of channels from %u to %u\n",hdr->channels,chan);
        hdr->channels=(uint16_t)chan;
    }
    return true;
}
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
uint64_t ADM_Composer::getFrameIncrement(bool copyMode)
{
    int nb = _segments.getNbRefVideos();
    if(!nb) return 0;
    uint64_t minIncrement = 0;
    for(int i=0; i<nb; i++)
    {
        _VIDEOS *vid = _segments.getRefVideo(i);
        ADM_assert(vid);
        uint64_t inc = vid->timeIncrementInUs;
        if(!inc) continue;
        if(!copyMode && vid->fieldEncoded)
        {
            ADM_info("Doubling frame increment for field-encoded ref video %d\n",i);
            inc *= 2;
        }
        if(!minIncrement | (inc < minIncrement))
            minIncrement = inc;
    }
    return minIncrement;
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
  _currentSegment = 0;
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
    if(!_segments.getNbRefVideos()) return false;
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
