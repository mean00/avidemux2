/***************************************************************************
                          audiodeng_buildfilters.cpp  -  description
                             -------------------
    begin                : Mon Dec 2 2002
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
#include "ADM_default.h"
#include "DIA_coreToolkit.h"

#include "audioencoder.h"

#include "ADM_audiofilter/audiofilter_limiter_param.h"
#include "audiofilter_normalize_param.h"
#include "audio_encoderWrapper.h"

#include "audioprocess.hxx"
#include "ADM_audiofilter/audioeng_buildfilters.h"
#include "ADM_audiofilter/audio_raw.h"
#include "ADM_editor/ADM_edit.hxx"
extern ADM_Composer *video_body;

/* ************* Encoder *********** */

#include "ADM_audiocodec/ADM_audiocodeclist.h"


#include "prefs.h"


/* ************ Filters *********** */
#include "audiofilter_bridge.h"
#include "audiofilter_mixer.h"
#include "audiofilter_normalize.h"
#include "audiofilter_limiter.h"
#include "audiofilter_SRC.h"
#include "audiofilter_film2pal.h"

/* ************ Conf *********** */
//#include "audioencoder_config.h"


#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_AUDIO_FILTER
#include "ADM_osSupport/ADM_debug.h"

#define MAX_AUDIO_FILTER 10


static AUDMAudioFilter *filtersFloat[MAX_AUDIO_FILTER];
static uint32_t filtercount = 0;

extern uint32_t audioProcessMode(void);

/******************************************************
  Configuration variables for filters
******************************************************/

extern AUDIOENCODER  activeAudioEncoder;

extern GAINparam audioGain;
extern int  audioFreq;
extern int  audioDRC;
extern FILMCONV audioFilmConv;
extern CHANNEL_CONF audioMixing;
extern int audioMP3mode;
extern int audioMP3bitrate;
extern RESAMPLING  audioResampleMode;
extern int   audioShift ;
extern int   audioDelay;

static DRCparam drcSetup=
{
  1,
  0.001,//double   mFloor;
  0.2, //double   mAttackTime;
  1.0, //double   mDecayTime;
  2.0, //double   mRatio;
  -12.0 ,//double   mThresholdDB;
};



//
// Build audio filters
// Starttime : starttime in ms
// mode : 0 for playback,  1 for audio save
//_______________________________________


AUDMAudioFilter *buildInternalAudioFilter(ADM_audioStream *currentaudiostream,uint32_t starttime)
{

  AUDMAudioFilter *firstFilter = NULL;
  AUDMAudioFilter *lastFilter = NULL;
  
//  deleteAudioFilter(NULL);
  int32_t timeShiftMs=audioDelay*audioShift;
  
  
  firstFilter = new AUDMAudioFilter_Bridge(NULL,video_body, starttime,timeShiftMs);
  filtercount = 0;
  lastFilter = firstFilter;
  filtersFloat[filtercount++] = firstFilter;



//_______________________________________________________
      if( (audioMixing!=CHANNEL_INVALID ))
          {
            AUDMAudioFilter *mixer;
            mixer=new AUDMAudioFilterMixer( lastFilter,audioMixing);
            lastFilter = mixer;
            filtersFloat[filtercount++] = lastFilter;
          } 
#if 0
    if (audioDRC)
          {
            AUDMAudioFilterLimiter *pdrc = NULL;
            printf("\n  DRC activated...\n");
            pdrc = new AUDMAudioFilterLimiter(lastFilter,&drcSetup);
            lastFilter = (AUDMAudioFilter *)pdrc;
            filtersFloat[filtercount++] = lastFilter;
          
          }

      switch(audioResampleMode)
          {
                  
            case RESAMPLING_NONE: break;
            case RESAMPLING_CUSTOM:
            {
                      AUDMAudioFilterSrc  *resample=NULL;
                      resample = new AUDMAudioFilterSrc(lastFilter, audioFreq);
                      lastFilter = resample;
                      filtersFloat[filtercount++] = lastFilter;	
            }
                      break;

            default:
                      ADM_assert(0);
          }

      switch(audioFilmConv)
        {
          default:
                        ADM_assert(0);
          case FILMCONV_NONE:
                        break;
          case FILMCONV_PAL2FILM:		
                  AUDMAudioFilterPal2Film *p2f;
                        p2f = new AUDMAudioFilterPal2Film(lastFilter);
                        lastFilter = p2f;
                        filtersFloat[filtercount++] = lastFilter;	
                        break;
                        
                
          case FILMCONV_FILM2PAL:
                    AUDMAudioFilterFilm2Pal *f2p;
                        f2p = new AUDMAudioFilterFilm2Pal(lastFilter);
                        lastFilter = f2p;
                        filtersFloat[filtercount++] = lastFilter;	
                        break;
                
                        
        }   

      if ( audioGain.mode!=ADM_NO_GAIN)	// Normalize activated ?
      {
        printf("\n  normalize activated..\n");
      
        AUDMAudioFilterNormalize *normalize = new AUDMAudioFilterNormalize(lastFilter,&audioGain);
        lastFilter = normalize;
        filtersFloat[filtercount++] = lastFilter;
      
      }
#endif
//_______________________________________________________




    return lastFilter;
}
/**

    \fn buildPlaybackFilter
    \brief Warning : starttime is in ms, not us!
    @param currentaudiostream : audio stream to build playback from
    @param starttime starting time in ms
    @param duration duration of stream in ms
*/
AUDMAudioFilter *buildPlaybackFilter(ADM_audioStream *currentaudiostream, uint32_t starttime, uint32_t duration)
{
  AUDMAudioFilter *lastFilter=NULL;
  int32_t sstart;
  uint32_t channels;
        // Do we need to go back
  sstart=(int32_t)starttime;
  int32_t timeShiftMs=audioDelay*audioShift;
        
//  deleteAudioFilter(NULL);
  
  lastFilter = new AUDMAudioFilter_Bridge(NULL,video_body,sstart,timeShiftMs);
        filtercount = 0;
        filtersFloat[filtercount++] = lastFilter;
        
        
        // Downmix for local playback ?
  
        uint32_t downmix;
        
        if(prefs->get(DOWNMIXING_PROLOGIC,&downmix)!=RC_OK)
        {       
          downmix=0;
        }
        channels=lastFilter->getInfo()->channels;
        if( downmix && channels>2)
        {
          CHANNEL_CONF mix;
          switch (downmix) {
		case 1:
			printf("Downmixing to stereo\n");
			mix=CHANNEL_STEREO;
		break;
		case 2:
			printf("Downmixing to prologic\n");
			mix=CHANNEL_DOLBY_PROLOGIC;
		break;
		case 3:
			printf("Downmixing to prologic2\n");
			mix=CHANNEL_DOLBY_PROLOGIC2;
		break;
		default:
			ADM_assert(0);
          }
          AUDMAudioFilterMixer *mixer;
          mixer=new AUDMAudioFilterMixer( lastFilter,mix);
          lastFilter = mixer;
          filtersFloat[filtercount++] = lastFilter;
        }	

        return lastFilter;
}
/*
*******************************************************************************************************************

*******************************************************************************************************************
*/

ADM_audioStream *buildAudioFilter(ADM_audioStream *currentaudiostream,  uint32_t starttime)
{
  AUDMAudioFilter         *lastFilter=NULL;
  ADM_audioStream         *output=NULL;
  AUDMEncoder             *tmpfilter=NULL;
	// if audio is set to copy, we just return the first filter
  if(!audioProcessMode())
  {
    int32_t timeShiftMs=audioDelay*audioShift;
//    deleteAudioFilter(NULL);
//    output = new AVDMProcessAudio_RawShift(currentaudiostream, starttime, timeShiftMs);
    return output;

  }



// else we build the full chain
  lastFilter=buildInternalAudioFilter(currentaudiostream,starttime);
  
// and add encoder...


//_______________________________________________________
  uint8_t init;
  
  if(!lastFilter)
  {
    printf(" buildInternalAudioFilter failed\n");
    return 0;
  }
  if(lastFilter->getInfo()->channels > audioFilter_getMaxChannels())
  {
    GUI_Error_HIG(QT_TR_NOOP("Codec Error"),QT_TR_NOOP("The number of channels is greater than what the selected audio codec can do.\n"
        "Either change codec or use the mixer filter to have less channels."));
//    deleteAudioFilter(NULL);
    return 0; 
  }

  tmpfilter=audioEncoderCreate(lastFilter);
  if(!tmpfilter || !tmpfilter->initialize())
  {
    if(tmpfilter) delete tmpfilter;
    tmpfilter=NULL;
    GUI_Error_HIG(QT_TR_NOOP("[BuildChain] Encoder initialization failed"), QT_TR_NOOP("Not activated."));
  }
  ADM_audioEncoderWrapper *wrapper=new ADM_audioEncoderWrapper(tmpfilter);
//  output=wrapper;

  ADM_assert(output);
  return output;
}


/*
*******************************************************************************************************************
     delete audio filters
*******************************************************************************************************************
*/
#if 0
void deleteAudioFilter(AVDMGenericAudioStream *in)
{
  for (uint32_t i = 0; i < filtercount; i++)
  {
    delete filtersFloat[i];
    filtersFloat[i] = NULL;
  }
  if(in)
    delete in;
  filtercount = 0;
  if (currentaudiostream)
    currentaudiostream->endDecompress();

}
#endif
/**
    \fn     audioFilter_MP3DisableReservoir
    \brief  Set/unset the disable reservoir bit, usefull for strict mp3 frame boundaries(FLV)

*/
void audioFilter_MP3DisableReservoir(int onoff)
{
#if 0
      if(activeAudioEncoder!=AUDIOENC_MP3) return;
      ADM_audioEncoderDescriptor *desc=getAudioDescriptor( activeAudioEncoder);
      ADM_assert(desc);
      LAME_encoderParam *param=(LAME_encoderParam *)desc->param;
      ADM_assert(param);
      param->disableReservoir=onoff;
#endif
}

/**********************************************/





