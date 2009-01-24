//
// C++ Implementation: op_ogaudio
//
// Description: 
//
//
// This file handles the audio track in OGM output plugin
// The difficult part with audio is that the counter is in
//  audio packet hence depends on the codec itself
//
// We will always try to put complete audio packet in the stream
// as the muxer is a bit stupid anyway
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "fourcc.h"
#include "avi_vars.h"
#include "ADM_assert.h"


//#include "avilist.h"

#include "ADM_videoFilter.h"

#include "ADM_encoder/ADM_vidEncode.hxx"

#include "ADM_audio/aviaudio.hxx"
#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_audiofilter/audioeng_buildfilters.h"
#include "ADM_audio/ADM_a52info.h"

#include "op_ogsave.h"

// should be enougth, at 500 kbps / 25 fps -> 3kbyte per slice
#define OGM_AUDIO_BUFFER 64000

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_OGM_WRITE
#include "ADM_osSupport/ADM_debug.h"

extern const char *getStrFromAudioCodec( uint32_t codec);	

//__________________________________________________
uint8_t		ADM_ogmWrite::initAudio(void)
{
uint32_t 	tstart;
WAVHeader	*info=NULL;
                audioStream=NULL;
		if(!currentaudiostream)
		{
			audioFilter=NULL;	
			_audioBuffer=NULL;
			encoding_gui->setAudioCodec(QT_TR_NOOP("None"));
			return 1;
		}
		if(audioProcessMode() && currentaudiostream->isCompressed() 
                        && !currentaudiostream->isDecompressable())
		{
                    return 0;
		}
                audioStream=new ogm_page(_fd,2);		//
                audioFilter=buildAudioFilter(currentaudiostream, video_body->getTime (frameStart));
                if(!audioFilter) return 0;
		if(audioProcessMode())
		{
			uint16_t fcc;
			
			fcc=audioFilter->getInfo()->encoding;
			encoding_gui->setAudioCodec(getStrFromAudioCodec(fcc));
		}
		else	// Copymode
		{
			encoding_gui->setAudioCodec(QT_TR_NOOP("Copy"));
			
		}
		//______________ Write headers/ Audio..._____________________
		
		
		_audioBuffer=new uint8_t[OGM_AUDIO_BUFFER*2]; 
		
		char string[40];
		stream_header header;
		
		info=audioFilter->getInfo();
		
		if(audioFilter->getInfo()->encoding!=WAV_OGG)
		{
		
			memset(&header,0,sizeof(header));
		
			uint64_t d64;
			uint32_t d32;
			uint16_t d16;
			
			memcpy(&(header.streamtype),"audio\0\0\0",8);
			memset(&(header.subtype),0,4);
			sprintf(string,"%04X",info->encoding);
			//memcpy(&(header.subtype),&(info->encoding),2);
			memcpy(&(header.subtype),string,4);
			printf("audio encoding:%x\n",info->encoding);
			
			
#define DO(x,y,z) {d##z=y;MEMCPY(&header.x,&d##z,z>>3);}
			DO(size,sizeof(header),32);
			//header.size=sizeof(header);
			//header.audio.channels=info->channels;
			DO(audio.channels,info->channels,16);
			// not reliable header.audio.blockalign=info->blockalign;
			uint16_t bps;
			uint16_t blkalign;
			switch(info->encoding)
			{
				case WAV_MP3:
				case WAV_MP2:
					bps=0;
					blkalign=1152;break;
				case WAV_AC3:	
					bps=2;
					blkalign=1536;break;
				default:
					blkalign=info->blockalign;
					bps=0;
					break;
			}
		
			DO(audio.blockalign,blkalign,16);
			DO(bits_per_sample,bps,16);
			
			//header.audio.avgbytespersec=info->byterate;
			DO(audio.avgbytespersec,info->byterate,32);
		
			//header.time_unit=(int64_t)10000000;
			DO(time_unit,10000000LL,64);
			//header.samples_per_unit=info->frequency;
			DO(samples_per_unit,info->frequency,64);
		
			//header.buffersize=info->frequency; // half a sec
			DO(buffersize,info->frequency,32); // half a sec
			
			//header.default_len=1;
			DO(default_len,1,32);
			audioStream->writeHeaders(sizeof(header),(uint8_t *)&header); // +4 ?
			_audioTarget=_audioCurrent=0;
			return 1;	

		}
		else
		{
			uint32_t exlen;
			uint8_t  *exdata;
			if(!audioProcessMode())
			{
			currentaudiostream->extraData(&exlen,&exdata);
			}
			else
			{
			audioFilter->extraData(&exlen,&exdata);
			}
			uint32_t *p;
			p=(uint32_t *)exdata;
			
			if(!exlen || !exdata)
			{
				delete audioFilter;
				audioFilter=NULL;
				delete audioStream;
				audioStream=NULL;
				printf("Vorbis audio setup failed (len:%lu)\n",exlen);
				return 0;
			}
			
			exdata+=3*sizeof(uint32_t);
			
			audioStream->writeDirect(p[0],exdata); // Header
			
			exdata+=p[0];
			audioStream->writeDirect(p[1],exdata); // Comment
			
			exdata+=p[1];
			audioStream->writeDirect(p[2],exdata); // Codebook
			_audioTarget=_audioCurrent=0;
			return 1;	
		
		}
		
	
}
//__________________________________________________
uint8_t		ADM_ogmWrite::writeAudio(uint32_t vframe)
{
uint32_t chunk,red,sample;
double tgt;
uint32_t len,packetLen,packets=0;
uint32_t totalsample=0;
uint32_t lastPacket;
uint32_t header_offset;


	if(!audioStream) return 1; // nothing to do
	if(!audioFilter) return 1;
		
	tgt=vframe+1;
	tgt/=_fps1000;
	tgt*=1000*audioFilter->getInfo()->frequency;
	
	_audioTarget=(uint32_t )floor(tgt);
	//printf("Frame :%lu audio current : %lu target:%lu \n",vframe,_audioCurrent,_audioTarget);
	len=0;
	sample=0;
	
	if(_audioTarget<_audioCurrent)
	{
		aprintf("OgWr: Enough audio in the tank..\n");
		return 1;
	}	
	// _audioTarget is the # of sample we want
	// Here we are handling MP3 or AC3
	if(audioFilter->getInfo()->encoding==WAV_OGG) header_offset=0;
		else	header_offset=1;
		
	while(_audioCurrent<_audioTarget)
	{
		_audioBuffer[0]=0;
		if(!audioFilter->getPacket(_audioBuffer+header_offset,&packetLen,&sample))
		{
			printf("OGMWR:Could not read packet\n");
			break;
		}
		
		_audioCurrent+=sample;
		totalsample+=sample;		
		audioStream->writeRawData(packetLen+header_offset,_audioBuffer,_audioCurrent);
		
		packets++;
		len+=packetLen;
	}
//	encoding_gui->feedAudioFrame(len);
	if(totalsample)
	{
		audioStream->flush();
		aprintf("OGMW: Found %lu packet sample :%lu len=%lu, curoffset:%lu targetoffset=%lu\n",
			packets,totalsample,len,_audioCurrent,_audioTarget);
	}	
	return 1;
}

//________________________________________
uint8_t		ADM_ogmWrite::endAudio(void)
{
	if(!audioStream) return 1;
#if 0		
		double l;
		
		l=_audioCurrent+_audioTail-_audioHead;
		l=l/audioFilter->getInfo()->byterate;
		l=l*audioFilter->getInfo()->frequency;
		
	audioStream->write(_audioTail-_audioHead,_audioBuffer+_audioHead,AVI_KEY_FRAME,(uint64_t)(floor(l)));
#endif	
	if(audioStream) audioStream->flush();
	if(audioFilter) 
	{
                deleteAudioFilter(audioFilter);
		audioFilter=NULL;
	}
	return 1;

}
//________
