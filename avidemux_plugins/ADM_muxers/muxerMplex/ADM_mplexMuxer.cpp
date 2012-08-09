//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//



/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <sys/stat.h>

#include "ADM_default.h"
#include "ADM_threads.h"

#include "interact.hpp"

#undef malloc
#undef realloc
#undef free


#include "ADM_audiofilter/audioprocess.hxx"
#include "ADM_audio/ADM_a52info.h"
#include "avifmt.h"
#include "avifmt2.h"
#include "ADM_editor/ADM_Video.h"
#include "ADM_outputs/ADM_lavformat.h"

#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME MODULE_LAVFORMAT
#include "ADM_osSupport/ADM_debug.h"

#include "cpu_accel.h"
#include "mjpeg_types.h"
#include "mjpeg_logging.h"
#include "mpegconsts.h"

#include "bits.hpp"
#include "outputstrm.hpp"
#include "multiplexor.hpp"

#include "ADM_inout.h"
#include "multiplexor.hpp"

/* 
        Define class to handle output stream(s)
      
*/


//#define VERBOSE_GOP


/***************************************************************************/


static PacketQueue     *channelaudio=NULL;
static PacketQueue     *channelvideo=NULL;
static FileOutputStream *outputStream=NULL;
static IFileBitStream   *audioin=NULL;
static IFileBitStream   *videoin=NULL;
static uint32_t         fps1000;
static int              mux_format;
static int              slaveRunning=0;

static  vector<IBitStream *> inputs;

static int slaveThread( WAVHeader *audioheader );

admMutex mutex_slaveThread_problem("mutex_slaveThread_problem");
admCond  *cond_slaveThread_problem;
char * kind_of_slaveThread_problem;
unsigned int kind_of_slaveThread_problem_rc;
extern uint8_t DIA_quota( char * msg );

typedef  void * (*THRINP)(void *p);
//___________________________________________________________________________
mplexMuxer::mplexMuxer( void )
{
        _running=0;
        _restamp=0;
	cond_slaveThread_problem = new admCond(&mutex_slaveThread_problem);
	kind_of_slaveThread_problem = 0;

	mjpeg_default_handler_verbosity(1);
}
//___________________________________________________________________________
mplexMuxer::~mplexMuxer()
{
        close();
}
//___________________________________________________________________________
uint8_t mplexMuxer::audioEof(void)
{
        channelaudio->Abort();
}
uint8_t mplexMuxer::videoEof(void)
{
  channelvideo->Abort();
}

//___________________________________________________________________________
uint8_t mplexMuxer::open(const char *filename, uint32_t inbitrate,ADM_MUXER_TYPE type, aviInfo *info, WAVHeader *audioheader)
{
        printf("Opening mplex muxer (%s)\n",filename);
        _running=1;

        
        channelaudio=new PacketQueue(  "Mplex audioQ",TRANSFERT_SLOT,TRANSFERT_BUFFER);
        channelvideo=new PacketQueue(  "Mplex videoQ",TRANSFERT_SLOT,TRANSFERT_BUFFER);
        
        outputStream=new FileOutputStream ( filename );
        
        fps1000=info->fps1000;
        
        switch(type)
        {
                case MUXER_DVD: mux_format=MPEG_FORMAT_DVD_NAV;break; //FIXME
                case MUXER_VCD: mux_format=MPEG_FORMAT_VCD;break;
                case MUXER_SVCD:mux_format=MPEG_FORMAT_SVCD;break;
                default:
                        printf("Unknown muxing type\n");
                        ADM_assert(0);
        }
        printf("mplex type is :%d\n",mux_format);
       
       
        
        printf("creating slave thread\n");
        
        pthread_t slave;
        slaveRunning=1;
        ADM_assert(!pthread_create(&slave,NULL,(THRINP)slaveThread,audioheader));

        ADM_usleep(1000*50); // Allow slave thread to start
        
        printf("Init ok\n");
        return 1;
}

static uint8_t wavToStreamType(WAVHeader *hdr,mplexStreamDescriptor *desc)
{
    ADM_assert(hdr);
    desc->frequency=hdr->frequency;
    desc->channel=hdr->channels;
    switch(hdr->encoding)
    {
        case WAV_LPCM:  desc->kind= LPCM_AUDIO;break;
        case WAV_AC3:   desc->kind=  AC3_AUDIO;;break;
        case WAV_MP2: case WAV_MP3:   desc->kind=  MPEG_AUDIO;;break;
        case WAV_DTS:    desc->kind=  DTS_AUDIO;;break;
        default: return 0;
    }
  return 1;
}

int slaveThread( WAVHeader *audioheader )
{
        MultiplexJob job;
        mplexStreamDescriptor audioDesc;
        mplexStreamDescriptor videoDesc;

        printf("[Muxer Slave Thread] Creating job & muxer\n");
        wavToStreamType(audioheader,&audioDesc);

        printf("output file created\n");
        audioin=new IFileBitStream(channelaudio,&audioDesc);
        
        printf("audio done (%s), creating video bitstream\n",getStrFromAudioCodec(audioheader->encoding));
        videoDesc.kind=MPEG_VIDEO;

        videoin=new IFileBitStream(channelvideo,&videoDesc);
        
        printf("Both stream ready\n");
         
        inputs.push_back( videoin );
        inputs.push_back( audioin );
        
        job.mux_format=mux_format;
        job.SetupInputStreams( inputs );

        Multiplexor mux(job, *outputStream);
               
        printf("[Muxer Slave Thread] Muxing\n");
        mux.Multiplex();

        slaveRunning=0;
        printf("[Muxer Slave Thread] Exiting\n");
        pthread_exit(0);
}        
//___________________________________________________________________________
uint8_t mplexMuxer::writeAudioPacket(uint32_t len, uint8_t *buf)
{
        
        return channelaudio->Push(buf,len,0);
}
//___________________________________________________________________________
uint8_t mplexMuxer::needAudio( void )
{
        
       return 1;
}
static uint8_t seq_start_code [] = {0x00, 0x00, 0x01, 0xB3};
static uint8_t gop_start_code [] = {0x00, 0x00, 0x01, 0xB8};

//___________________________________________________________________________
uint8_t mplexMuxer::writeVideoPacket(ADMBitstream *bitstream)
{
uint8_t r=0;   
uint16_t a1,a2,a3,a4,ff;           
        if(_restamp) // restamp timecode ?
        {
            if ( !memcmp(bitstream->data, seq_start_code, 4) || !memcmp(bitstream->data, gop_start_code, 4) )
                {
                        uint8_t *ptr;
                        uint32_t size;
                        
                        ptr=bitstream->data;
                        size=bitstream->len;
                        // There is a gop a or seq header
                        if(bitstream->data[3]==0xb3) // Seq
                        {
                                while((ptr[0] || ptr[1] || ptr[2]!=1 || ptr[3]!=0xB8 ) && size>0)
                                {
                                        ptr++;
                                        size--;
                                }
                        
                        }
                        if(!size || size < 8) 
                        {
                                printf("Mplex:Gop/seq inconsistency\n");
                        }
                        else
                        {       // Now we are at gop start with a packet size
                                // Compute the current gop timestamp
                                double newtime=bitstream->ptsFrame;
                                uint32_t hh,mm,ss,ms;
                                
                                ptr+=4; // skip gop header go to timestamp
                                
#ifdef VERBOSE_GOP              
                               
                                                       
                                a1=*ptr;
                                a2=*(ptr+1);
                                a3=*(ptr+2);
                                a4=*(ptr+3);
                                hh=(a1>>2)&0x1f;
                                mm=((a1&3)<<4)+(a2>>4);
                                ss=((a2&7)<<3)+(a3>>5);
                                ff=((a3&0x1f)<<1)+(a4>>7);

                                printf("Old : h:%02d m:%02d s:%02d f:%02d\n",hh,mm,ss,ff);
#endif  
                                
                                newtime=(newtime*1000);
                                newtime/=fps1000; // in seconds
                                
                                hh=(uint32_t)newtime/3600;
                                newtime-=hh*3600;
                                mm=(uint32_t)newtime/60;
                                newtime-=mm*60;
                                ss=(uint32_t)newtime;
                                newtime-=ss;
                                newtime*=1000;
                                ms=(uint32_t)newtime;
                                
                                *(ptr+0)=(hh<<2)+(mm>>4);
                                *(ptr+1)=((mm&0xf)<<4)+8+(ss>>3);
                                *(ptr+2)= ((ss&7)<<5)+(ms>>1);
                                *(ptr+3)&=0x7f;
                                *(ptr+3)+=(ms&1) <<7;
                                
#ifdef VERBOSE_GOP              
                               
                                                       
                                a1=*ptr;
                                a2=*(ptr+1);
                                a3=*(ptr+2);
                                a4=*(ptr+3);
                                hh=(a1>>2)&0x1f;
                                mm=((a1&3)<<4)+(a2>>4);
                                ss=((a2&7)<<3)+(a3>>5);
                                ff=((a3&0x1f)<<1)+(a4>>7);

                                printf("New : h:%02d m:%02d s:%02d f:%02d\n",hh,mm,ss,ff);
#endif  
                                
                        
                        }
        
                }
        }

        if( cond_slaveThread_problem->iswaiting() )
        {
               kind_of_slaveThread_problem_rc = DIA_quota(kind_of_slaveThread_problem);
               cond_slaveThread_problem->wakeup();
         }
        // Check for overflow
        // Should not happen on audio
#warning the value is set also in mplex as BitStreamBuffering::BUFFER_SIZE
#define INPUT_MAX_BLOCK (64*1024-1)
        uint8_t *ptr=bitstream->data;
        uint32_t len=bitstream->len;

        while(len)
        {
          if(len>INPUT_MAX_BLOCK)
          {
              channelvideo->Push(ptr,INPUT_MAX_BLOCK,0);
              len-=INPUT_MAX_BLOCK;
              ptr+=INPUT_MAX_BLOCK;
          }
          else
          {
            channelvideo->Push(ptr,len,0);
            len=0;
          }
        }
        return 1;
}
//___________________________________________________________________________
uint8_t mplexMuxer::forceRestamp(void)
{
        _restamp=1;
        return 1;
}
//___________________________________________________________________________
uint8_t mplexMuxer::close( void )
{
        if(_running)
        {
                _running=0;
                channelvideo->Abort();
                channelaudio->Abort();
                while(slaveRunning)
                {
                        printf("Waiting for slave thread to end\n");
                        ADM_usleep(100*1000);
                }
                        // Flush
                        // Cause deadlock :
                delete audioin;
                delete videoin;
                delete channelvideo;
                delete channelaudio;
                delete outputStream;
                inputs.erase( inputs.begin(), inputs.end() );
                printf("Mplex : All destroyed\n");
        }
        return 1;
}
//___________________________________________________________________________
uint8_t mplexMuxer::audioEmpty( void)
{
        return 0;
}


//EOF
