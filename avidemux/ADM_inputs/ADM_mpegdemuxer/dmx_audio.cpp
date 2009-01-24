/***************************************************************************
                          dmx_audio
                          Audio mpeg demuxer
                             -------------------
    
    copyright            : (C) 2008 by mean
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
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "config.h"
#include <math.h>

#include "ADM_default.h"
#include "avifmt.h"
#include "avifmt2.h"

#include "ADM_Video.h"
#include "ADM_audio/aviaudio.hxx"

#include "fourcc.h"
#include "ADM_assert.h"


#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_MPEG_DEMUX
#include "ADM_osSupport/ADM_debug.h"

#include "dmx_audio.h"
#include "ADM_mp3info.h"
#include "ADM_a52info.h"
#include "ADM_dcainfo.h"
#include "ADM_aacinfo.h"
#define MAX_LINE 4096
#define PROBE_SIZE (4096*4)
#define DMX_MIN_AUDIO_DETECTED PROBE_SIZE //we must have at least that bytes to consider the audio as valid
//___________________________________________________
//___________________________________________________


//___________________________________________________
//___________________________________________________
ADM_audioAccessMpeg::~ADM_audioAccessMpeg ()
{
  if (_index)
    delete[] _index;

  if (demuxer)
    delete demuxer;

  if(_tracks)
      delete [] _tracks;

  _index = NULL;
  demuxer = NULL;

}
/**
    \fn    ADM_audioAccessMpeg
    \brief Constructor
*/
ADM_audioAccessMpeg::ADM_audioAccessMpeg (const char *name)
{
  demuxer = NULL;
  nbTrack = 0;
  _tracks=NULL;
  currentTrack=0;
  _index=NULL;
  FILE *file;
  uint32_t dummy;		//,audio=0;
  char string[MAX_LINE];	//,str[1024];;
  uint32_t w = 720, h = 576, fps = 0;
  uint8_t type, progressif;
  char realname[1024];
  uint32_t aPid, vPid, aPes,mainAudio;
  uint32_t nbGop, nbFrame,nbAudioStream;
  int multi;
  char payload[MAX_LINE];
  uint32_t oldIndex=0;
  
  
 file=fopen(name,"rt");
 if(!file) ADM_assert(0);

  printf ("\n  opening dmx file for audio track : %s\n", name);
  fgets (string, MAX_LINE, file);	// File header
  if (strncmp (string, "ADMY", 4))
    {
       if (strncmp (string, "ADMX", 4))
       {
          fclose (file);
          printf ("This is not a mpeg index G2\n");
          ADM_assert (0);
       }
       oldIndex=1;
    }
    


  fgets (string, MAX_LINE, file);
  sscanf (string, "Type     : %c\n", &type);	// ES for now

char *start;
  fgets (string, MAX_LINE, file);
  //sscanf (string, "File     : %s\n", realname);
  start=strstr(string,":");
  ADM_assert(start);
  strcpy(realname,start+2);

  int l=strlen(realname)-1;
  while(l&&(realname[l]==0x0a || realname[l]==0x0d))
  {
           realname[l]=0;
           l--;
  }


  fgets (string, MAX_LINE, file);
  sscanf(string,"Append   : %d\n",&multi);

  fgets (string, MAX_LINE, file);
  sscanf (string, "Image    : %c\n", &progressif);	// Progressive

  fgets (string, MAX_LINE, file);
  sscanf (string, "Picture  : %u x %u %u fps\n", &w, &h, &fps);	// width...
  if(!oldIndex)
  {
   fgets (string, MAX_LINE, file);
   payload[0]=0;
   sscanf (string, "Payload  : %s\n",payload);	// FIXME ! overflow possible
  }
  fgets (string, MAX_LINE, file);
  sscanf (string, "Nb Gop   : %u \n", &nbGop);	// width...

  fgets (string, MAX_LINE, file);
  sscanf (string, "Nb Images: %u \n", &nbFrame);	// width...

  fgets (string, MAX_LINE, file);
  sscanf(string,"Nb Audio : %u\n",&nbAudioStream); 

  fgets(string,MAX_LINE,file);
  sscanf(string,"Main aud : %u\n",&mainAudio); 

  if(!nbAudioStream)
  {
_abrt:
         fclose (file);
        ADM_assert(0);
  }
  _tracks=new dmxAudioTrack[nbAudioStream];
  memset(_tracks,0,sizeof(dmxAudioTrack)*nbAudioStream);
  nbTrack=nbAudioStream;
  _index=new dmxAudioIndex[nbGop+1];
  fgets (string, MAX_LINE, file);

  char *needle,*hay;
  uint32_t i=0;
/*
    Extrcat the PES/PID for each audio track from the line
        A(PES):(PID) A(PES):(PID) ....
*/
  hay=string;
  while(1)
  {
        needle=strstr(hay," A");
        if(!needle) goto _nnx;
        sscanf(needle," A%x:%x",&aPid,&aPes);
        ADM_assert(i<nbAudioStream);
        _tracks[i].myPid=aPid;
        _tracks[i].myPes=aPes;
        needle[0]='.';
        needle[1]='.';
        hay=needle;
        i++;
  }
_nnx:
  ADM_assert(i==nbAudioStream);
  currentTrack=mainAudio;
  // Now build the demuxers
  // We build only one demuxer, then we will use changePid
  // To go from one track to another
MPEG_TRACK track;
                track.pes=_tracks[mainAudio].myPes;
                track.pid=_tracks[mainAudio].myPid;
  switch (type)
    {
    case 'M':
                demuxer = new dmx_demuxerMSDVR (1,&track,0);
                break;
    case 'P':
                demuxer = new dmx_demuxerPS (1,&track,multi);
                break;
    case 'T':
                demuxer = new dmx_demuxerTS (1,&track,0,DMX_MPG_TS);
                break;
    case 'S':
                demuxer = new dmx_demuxerTS (1,&track,0,DMX_MPG_TS2);
                break;

    default:
      ADM_assert (0);
    }
  if(!demuxer->open(realname))
        {
                printf("DMX audio : cannot open %s\n");
                
                fclose(file);                
                ADM_assert(0);
        }
  // Now build the index
  nbIndex = nbGop;
 
  printf ("Building index with %u sync points\n", nbGop);

  uint32_t read = 0, img, count;
  uint64_t abs;

  
  while (read < nbGop)
    {
      if (!fgets (string, MAX_LINE, file))
	break;
      if (string[0] != 'A')
	continue;

      sscanf (string, "A %u %"LLX, &img, &abs); //FIXME read all audio tracks and pick the one we want
        hay=strstr(string,":");
        ADM_assert(hay)
        i=0;
       while(1)
        {
                needle=strstr(hay,":");
                if(!needle) goto _nxt;;
                sscanf(needle,":%lx",&count);
                needle[0]='.';                
                hay=needle;
                _index[read].count[i] = count;
                i++;
        }
_nxt:


      _index[read].img = img;
      _index[read].start = abs;
      
      read++;
    }
    // now read offset
    {
    int trackNo,offset,pts;
    while(1)
    {
        if (!fgets (string, MAX_LINE, file))
            break;
        if(strncmp(string,"# track ",8)) continue;
        if(3!=sscanf(string,"# track %d PTS : %d  delta=%d ms",&trackNo,&pts,&offset))
        {
            printf("Error reading time offset for line [%s]\n",string);
            break;
        }
        ADM_assert(trackNo);
        ADM_assert(trackNo<nbTrack+1);
        trackNo--;
        _tracks[trackNo].avSync=offset;
        
    }
    } // /read offset
  fclose (file);
  nbIndex = read;
  if (!read)
    {
      printf ("No audio at all!\n");              
      ADM_assert(0);
    }
printf("Filling audio header\n");
  // now fill in the header

  for(int i=0;i<nbTrack;i++)
  {
    WAVHeader *hdr;
    hdr=&(_tracks[i].wavHeader);
    memset (hdr, 0, sizeof (WAVHeader));

    // put some default value
    hdr->bitspersample = 16;
    hdr->blockalign = 4;
    //demuxer->setPos (0, 0);
  }
  printf("Audio index loaded, probing...\n");
  if(!probeAudio()) ADM_assert(0);
        
  demuxer->changePid(_tracks[currentTrack].myPid,_tracks[currentTrack].myPes);
  demuxer->setPos (0, 0);

  printf ("With %lu sync point\n", nbIndex);
  int found=-1;
  // Only take the audio track if it has enough bytes
  // else we can have a track present in the PMT that is not actually present
  for(int i=0;i<nbTrack;i++)
  {
        int test=(mainAudio+i)%nbTrack;
        if(_index[nbIndex-1].count[test]>DMX_MIN_AUDIO_DETECTED)
        {
            found=test;
            break;
        }
        else printf("[DmxAudio] Skipping track %d, not enough audio\n",test);
  }
    if(found!=-1)
        mainAudio=found;
    changeAudioTrack(found);
}

/**
    \fn goToTime
    \brief Go to time
*/
bool ADM_audioAccessMpeg::goToTime (uint64_t goTo)
{
    return true;
}

/**
    \fn     getPacket
    \brief 
*/
bool    ADM_audioAccessMpeg::getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)
{
uint32_t mDts,mPts;
    if(!demuxer->readPes(buffer, size, &mDts,&mPts)) return false;
    *dts=(uint64_t)mDts;
    return true;
}


#if 0
uint8_t dmxAudioStream::probeAudio (void)
{
uint32_t read,offset,offset2,fq,br,chan,myPes,blocksize;          
uint8_t buffer[PROBE_SIZE];
MpegAudioInfo mpegInfo;
WAVHeader *hdr;

      for(int i=0;i<nbTrack;i++)
      {
        hdr=&(_tracks[i].wavHeader);
        // Change demuxer...

        demuxer->changePid(_tracks[i].myPid,_tracks[i].myPes);
        demuxer->setPos(0,0);
        printf("Probing track:%d, pid: %x pes:%x\n",i,_tracks[i].myPid,_tracks[i].myPes);
         //
        if(PROBE_SIZE!=(blocksize=demuxer->read(buffer,PROBE_SIZE)))
        {
           printf("DmxAudio: Reading for track %d failed (%u/%u)\n",i,blocksize,PROBE_SIZE);
           continue;
        }
        myPes=_tracks[i].myPes;
        // Try mp2/3
        if(myPes>=0xC0 && myPes<=0xC9)
        {
                if(getMpegFrameInfo(buffer,PROBE_SIZE,&mpegInfo,NULL,&offset))
                {
                        if(getMpegFrameInfo(buffer+offset,PROBE_SIZE-offset,&mpegInfo,NULL,&offset2))
                                if(!offset2)
                                {
                                        hdr->byterate=(1000*mpegInfo.bitrate)>>3;
                                        hdr->frequency=mpegInfo.samplerate;

                                        if(mpegInfo.mode!=3) hdr->channels=2;
                                        else hdr->channels=1;

                                        if(mpegInfo.layer==3) hdr->encoding=WAV_MP3;
                                        else hdr->encoding=WAV_MP2;
                                        continue;
                                }
                }
        }
        // Try AC3
        if(myPes<9)
        {
                if(ADM_AC3GetInfo(buffer,PROBE_SIZE,&fq,&br,&chan,&offset))
                {
                        if(ADM_AC3GetInfo(buffer+offset,PROBE_SIZE-offset,&fq,&br,&chan,&offset2))
                        {
                                hdr->byterate=br; //(1000*br)>>3;
                                hdr->frequency=fq;                                
                                hdr->encoding=WAV_AC3;
                                hdr->channels=chan;
                                continue;
                        }
                }
        }

        if(myPes<=0x49 && myPes>=0x40)
        {
             
              uint32_t flags,samplerate,bitrate,framelength,syncoff,chan,nbs;
//int ADM_DCAGetInfo(uint8_t *buf, uint32_t len, uint32_t *fq, uint32_t *br, uint32_t *chan,uint32_t *syncoff,uint32_t *flags);
              if(ADM_DCAGetInfo(buffer,PROBE_SIZE,&samplerate,&bitrate,&chan,&syncoff,&flags,&nbs))
              {
                                hdr->byterate=bitrate/8;
                                hdr->frequency=samplerate;
                                hdr->encoding=WAV_DTS;
                                hdr->channels=chan;
                                continue;
               }
        }
        // Default 48khz stereo lpcm
        if(myPes>=0xA0 && myPes<0xA9)
        {
                hdr->byterate=(48000*4);
                hdr->frequency=48000;                                
                hdr->encoding=WAV_LPCM;
                hdr->channels=2;
                continue;
        }
         //AAC, can happen in TS file with H264
        if(myPes>=0xB0 && myPes<0xB9)
        {
          AacAudioInfo info;
          if(!getAACFrameInfo(buffer,blocksize, &info,NULL,&offset))
          {
            printf("\n Cannot get AAC sync info (not ADTS ?)\n");
                hdr->byterate=(128000)>>3;
                hdr->frequency=44100;
                hdr->encoding=WAV_AAC;
                hdr->channels=2;
                continue;
          }
          /*
          uint32_t layer;		// 0 mpeg4, 1 mpeg2 
	uint32_t profile;	// 0 Main/1 LC
	uint32_t samplerate;	// i.e. Frequency
	uint32_t channels;	// # channels
	uint32_t nbBlock;	// Packet size including header
	uint32_t size;		// size of complete frame
	uint32_t samples;	// # of sample in this packet
          */
              
                hdr->byterate=(128000)>>3;
                hdr->frequency=info.samplerate;                                
                hdr->encoding=WAV_AAC;
                hdr->channels=info.channels;
                continue;
        }
  }
  return 1;
}

uint8_t dmxAudioStream::changeAudioTrack(uint32_t newtrack)
{
      ADM_assert(newtrack<nbTrack);
      currentTrack=newtrack;

      _length=_index[nbIndex-1].count[currentTrack];
      _wavheader=&(_tracks[currentTrack].wavHeader);
      // Set demuxer...
      demuxer->changePid(_tracks[currentTrack].myPid,_tracks[currentTrack].myPes);
      //
      return 1;
}
uint8_t                 dmxAudioStream::getAudioStreamsInfo(uint32_t *nbStreams, audioInfo **infos)
{
    *nbStreams=0;
    *infos=NULL;
    if(!nbTrack) return 1;
    *nbStreams=nbTrack;
    *infos=new audioInfo [nbTrack];
    for(int i=0;i<nbTrack;i++)
    {
     //   (*infos)[i]=_tracks[i].wavHeader.encoding;
        WAV2AudioInfo(&(_tracks[i].wavHeader),&((*infos)[i]));
        (*infos)[i].av_sync=_tracks[i].avSync;
    }
    return 1;
}
#endif

 //
