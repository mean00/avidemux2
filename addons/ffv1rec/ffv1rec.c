/***************************************************************************
	Video capture program based on nuppelrev by
		roman Roman HOCHLEITNER

    begin                : Tue Jul  2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr

    Improvements also by
    	E Strickland 
	Michael Teske
	Levente Novak
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
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/soundcard.h>
#include <linux/wait.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <linux/videodev.h>

#include "nuppelvideo.h"
//#include "avcodec.h"
#include "../../avidemux/ADM_library/default.h"
#include "../../avidemux/ADM_osSupport/ADM_cpuCap.h"
#include "ffv1.h"
#include "frequencies.h"

//adm_fast_memcpy myMemcpy;

#define _VERSION_ "0.2.4 (2.2 branch)"


/* Globals */
extern "C"
{
extern uint8_t ADM_InitMemcpy(void);
}
static void parseRcFile( void );

int recordaudio=1;
int quiet;
pthread_t pid_audio, pid_video, pid_buffer, pid_write;
int CpuCaps::myCpuCaps=0;

typedef  void *PTHR(void *);

struct video_audio origaudio;

		v4linfo info={352,288,0,0,1,0,200,1,3,0};
		audioInfo ainfo;

#define ERROR(PARAM) { fprintf(stderr, "\n%s\n", PARAM); exit(1); }

// ----------------------------------------------------------------------



char *audiodevice = "/dev/dsp";

struct vidbuffertype *videobuffer=NULL;
struct audbuffertype *audiobuffer=NULL;


int video_buffer_count=0;   // should be a setting from command line  6.3MB 384x288x40
                            //                                       23.2MB 704x576x40
int audio_buffer_count=0;   // should be a setting from command line

long video_buffer_size=0;
int do_split=0;

struct timeval now, stm;
struct timezone tzone;

static int initBuffers( void );

// ----------------------------------------------------------------------

void sighandler(int i)
{
  if (!quiet) fprintf(stderr, "\n"); // preserve status line
  closeVideoDev();
  exit(0);
}

void sigexithandler (int i) 
{
  closeVideoDev();
  exit(0);
}

// ----------------------------------------------------------
// -- USAGE -------------------------------------------------

void usage()
{
   fprintf(stderr, "\nFFV1 Recording Tool v0.52    (c)Mean/Roman HOCHLEITNER\n");
   fprintf(stderr, "\nA nuvrec derivative, compressing with lavcodec ffv1 codec\n");
   fprintf(stderr, "Usage: nuvrec [options] filename \n\n");
   fprintf(stderr, "options: -q n .......... Quality 2-31                  ]\n");
   fprintf(stderr, "options: -Z   .......... Use mpeg quant                ]\n");
   fprintf(stderr, "         -M n .......... Motion estimation (1-6) [1]   ]\n");
   fprintf(stderr, "         -d n .......... K frame distance(1-500)[200]  ]\n");
   fprintf(stderr, "         -l n .......... Luminance Threshold   0-20 [1]\n");
   fprintf(stderr, "         -c n .......... Chrominance Threshold 0-20 [1]\n");
   fprintf(stderr, "         -W n .......... Width       [352 PAL, 352 NTSC]\n");
   fprintf(stderr, "         -H n .......... Height      [288 PAL, 240 NTSC]\n");
   fprintf(stderr, "         -t min ........ Length (3.5 = 3m 30s) [forever]\n");
   fprintf(stderr, "         -S n .......... Source (0 can be Televison) [0]\n");
   fprintf(stderr, "         -f n.n ........ Tunerfrequency      [no change]\n");
   fprintf(stderr, "         -x n .......... Video buffers in mb  [l.8/b.14]\n");
   fprintf(stderr, "         -y n .......... Audio buffers in mb         [2]\n");
   fprintf(stderr, "         -V dev ........ Videodevice       [/dev/video0]\n");
   fprintf(stderr, "         -A dev ........ Audiodevice          [/dev/dsp]\n");
   fprintf(stderr, "         -T ............ use time for drop calculation\n");
   fprintf(stderr, "         -P program..... switch tuner to program loaded from xawdecode \n");
   fprintf(stderr, "         -L ............ list programs loaded from xawdecode \n");   
   fprintf(stderr, "         -p ............ PAL [default] \n");
   fprintf(stderr, "         -n ............ NTSC\n");
   fprintf(stderr, "         -s ............ SECAM\n");
   fprintf(stderr, "         -Q ............ shut up\n");
   fprintf(stderr, "         -z ............ video only (i.e. no audio)\n");
   fprintf(stderr, "         -C ............ select video codec (MJPEG|HUFFYUV|FFHUFF|FFV1|MPEG1|MPEG2|MPEG4|XVID)\n");
   fprintf(stderr, "         -b ............ set the audio frequency [44100]\n");
   fprintf(stderr, "         -2 ............ split every  2 Gbytes [NO]\n");
   fprintf(stderr, "         -h ............ this help\n");
   fprintf(stderr, "\n");
   exit(-1);
}
/*--------------------------------------------------------*/

static char *videodevice = "/dev/video0";
static double drec= -1.0;
static int videomegs = -1;
static int audiomegs = -1;

 extern "C" {
     extern void     avcodec_init(void );
     extern  void 	avcodec_register_all(void );
                       };

// ----------------------------------------------------------
// -- MAIN --------------------------------------------------

int main(int argc, char** argv)
{

  char *outfilename = "outfile";
  int reclength= -1; // reclength in secs
 char c;

	 CpuCaps::init	();
	ADM_InitMemcpy();

  /////////////////////////////////////////////////////
  //  CHECKING AND INTERPRETING COMMAND LINE SWITCHES
  /////////////////////////////////////////////////////
 printf("\n*********************************************************************\n");
 printf("\n FFV1rec based on nuppelrec by Roman Hochleitner");
 printf("\n version "_VERSION_"\n");
 printf("\n*********************************************************************\n");
  quiet = 0;
  
	avcodec_init();
	avcodec_register_all();
	
  recordaudio = 1;
  memset(&ainfo,0,sizeof(ainfo));
  ainfo.frequency=44100;

  tzone.tz_minuteswest=-60; // whatever
  tzone.tz_dsttime=0;
  now.tv_sec=0; // reset
  now.tv_usec=0; // reset

  parseRcFile();

  while ((c=getopt(argc,argv,"d:b:M:q:l:c:C:S:W:H:t:NTV:A:a:srf:pnZb:x:y:zQ2P:L")) != -1) {
    switch(c) {
      case '2': do_split=1;break;
      case 'b': ainfo.frequency=atoi(optarg);break;
      case 'q': info.quality = atoi(optarg); break;
      case 'd': if(!(info.keydist = atoi(optarg))) info.keydist=1; break;
      case 'M': info.me = atoi(optarg); break;
      case 'Z': info.quant=1;break;
      case 'S': info.channel = atoi(optarg); break;
      case 'W': info.width = atoi(optarg);  break;
      case 'H': info.height = atoi(optarg);  break;
      case 't': drec = atof(optarg);  break;
      case 'x': videomegs = atoi(optarg);  break;
      case 'C': if(!FFV1_selectByName(optarg))
      				{
					printf("\n cannot find this codec\n");
					exit(0);
				};break;
      case 'y': audiomegs = atoi(optarg);  break;
      case 'P':  
        {
          char buf[33];
          info.frequency = get_chan_frequency(optarg, buf);
          fprintf(stderr, "switching to program %s\n", buf);
        }
        break;
      case 'L':
        print_channels();
        exit(0);
        break;
      case 'p': info.ntsc = 0;  break;
      case 'n': info.ntsc = 1;  break;
      case 's': info.ntsc = 0; info.secam=1;  break;
      case 'f': info.frequency = atof(optarg); break;
      case 'z': recordaudio = 0;printf("\n Audio disabled\n");   break;
      case 'A': audiodevice = optarg;   break;
      case 'V': videodevice = optarg;   break;
      case 'Q': quiet = 1;   break;
      case 'h': usage();  break;

      default: usage();
    }
  }

  if (optind==argc) usage();
  else outfilename=argv[optind];

  if (drec != -1.0) {
    reclength = (int)(drec*60);
  }

 if(info.width==0)
 {
 	info.width=352;
}
if(info.height==0)
{
	if(info.ntsc) 	info.height=240;
	else			info.height=288;
  }
  /////////////////////////////////////////////
  //  CALCULATE BUFFER SIZES
  /////////////////////////////////////////////

  video_buffer_size=(info.height*info.width*3)>>1  ;


  if (videomegs == -1) {
    if (info.width>=480 || info.height>288) {
      videomegs = 64; //drRocket -> helps  megs for big ones
    } else {
      videomegs = 14;  // normally we don't need more than that
    }
  }
  video_buffer_count = (videomegs*1024*1024)/video_buffer_size;

  // we have to detect audio_buffer_size, too before initshm()
  // or we crash horribly later, if no audio is recorded we
  // don't change the value of 16384
  if (recordaudio)
  {
    if (!audioDevPreinit(audiodevice,&ainfo))
     {
      fprintf(stderr,"error: could not detect audio blocksize, audio recording disabled\n");
      recordaudio = 0;
    }
  }

  if (audiomegs==-1) audiomegs=2;

  if (ainfo.bufferSize!=0) audio_buffer_count = (audiomegs*1000*1000)/ainfo.bufferSize;
                       else audio_buffer_count = 1;

  fprintf(stderr,
          "we are using %dx%ldB video (frame-) buffers and\n %dx%ldB audio blocks\n audio fq=%ld\n",
                  video_buffer_count, video_buffer_size,
                  audio_buffer_count, ainfo.bufferSize,
		  ainfo.frequency );



	initBuffers();
  /////////////////////////////////////////////
  //  NOW START WRITER/ENCODER PROCESS
  /////////////////////////////////////////////

	writeInit(&info);

	PTHR *wr;
	wr=(PTHR *)&write_process;

	pthread_create(&pid_write,0,wr,outfilename);
  ///////////////////////////
  // now save the start time
  gettimeofday(&stm, &tzone);

// #ifndef TESTINPUT
  /////////////////////////////////////////////
  //  NOW START AUDIO RECORDER
  /////////////////////////////////////////////

  if (recordaudio)
   {
	printf("Initializing audio...\n");
      	if(!	initAudioDev(audiodevice,&ainfo))
	{
		fprintf(stderr, "cannot initialize audio!\n");
		exit(-1);
	}
	PTHR *aud;
	aud=(PTHR *)&captureAudioDev;
	pthread_create(&pid_audio,0,aud,NULL);
 }
 else
 {
	printf("No audio.\n");
 }

// #endif

  /////////////////////////////////////////////
  //  NOW START VIDEO RECORDER
  /////////////////////////////////////////////

  // only root can do this
  if (getuid()==0) nice(-10);

  if (reclength != -1)
  {
    signal(SIGALRM, sighandler);
    alarm(reclength);
  }

	if( !initVideoDev(videodevice, &info ))
	{
		printf(" Cannot init video input\n");
		exit(0);
	}

        signal(SIGINT, sigexithandler);
        signal(SIGTERM, sigexithandler);
        signal(SIGQUIT, sigexithandler);
	captureVideoDev();

	return 0;
}

int initBuffers( void )
{
int i;

unsigned char *startvideo;
unsigned char *startaudio;

	startvideo=(unsigned char *)malloc(video_buffer_size*video_buffer_count );
	startaudio=(unsigned char *)malloc( ainfo.bufferSize*audio_buffer_count );

	printf("\n audio buffer : %d\n",audio_buffer_count*sizeof(audbuffertyp));
	printf("\n video buffer : %d\n",video_buffer_count*sizeof(vidbuffertyp));

	videobuffer= (struct vidbuffertype  *)malloc( video_buffer_count*sizeof(vidbuffertyp));
	audiobuffer=(struct audbuffertype *)malloc(audio_buffer_count*sizeof(audbuffertyp));

  	for (i=0; i<video_buffer_count; i++)
   	{
        	videobuffer[i].sample=0;              // not relevant as long freeToEncode==0
        	videobuffer[i].freeToEncode=0;        // empty at first
        	videobuffer[i].freeToBuffer=1;        // fillable == empty 2vars --> no locking!!
        	videobuffer[i].buffer_offset = startvideo + i*video_buffer_size;
      	}
      	for (i=0; i<audio_buffer_count; i++)
	{
        	audiobuffer[i].sample=0;              // not relevant as long freeToEncode==0
        	audiobuffer[i].freeToEncode=0;        // empty at first
        	audiobuffer[i].freeToBuffer=1;        // fillable == empty 2vars --> no locking!!
        	audiobuffer[i].buffer_offset = startaudio + i*ainfo.bufferSize;
      	}
	return 1;
}
/*---------------------------------
	The RC file are like this
	# comment
	X val
	For example
	q 4
	2
	b 48000 etc....

------------------------------------*/
void parseRcFile( void )
{
   char rcfile[1024];
   char string[1024];
   char *home;
   char *optarg;
   FILE *fd;

	if( ! (home=getenv("HOME")) ){
		fprintf(stderr,"can't determine $HOME.\n");
		exit(0);
	}
	snprintf(rcfile, 1024, "%s/.ffv1recrc", home);
	rcfile[1023] = '\0';
	fd=fopen(rcfile,"rt");
	if(!fd)
	{
		printf("No ~/.ffv1recrc found...\n");
		return;
	}

	while(fgets(string,1000,fd))
	{
		optarg=NULL;
		if(  (string[strlen(string)-1]==0xa)
				||
		(string[strlen(string)-1]==0xd) )
		{
			string[strlen(string)-1]=0;
		}
		switch(strlen(string))
		{
			case 0:
					break;
			case 1: // No arg
			case 2:
					switch(string[0])
					{
							case '#': break;
							case 'Z': info.quant=1;break;
							case '2': do_split=1;break;
   							case 'p': info.ntsc = 0;  break;
      							case 'n': info.ntsc = 1;  break;
      							case 's': info.ntsc = 0; info.secam=1;  break;
   							case 'z': recordaudio = 0;printf("\n Audio disabled\n");   break;
   							case 'Q': quiet = 1;   break;
							default : printf("Unknown command : %c\n",string[0]);
					}
					break;
			case 3:
			default:
					if(string[1]!='=' && string[0]!='#')
					{
						printf("\n ** The format is x=y not x y!!! (%c)\n",string[0]);
						break;
					}
					optarg=string+2;
					switch(string[0])
					{

      						case 'b': ainfo.frequency=atoi(optarg);break;
      						case 'q': info.quality = atoi(optarg); break;
      						case 'd': if(!(info.keydist = atoi(optarg))) info.keydist=1; break;
      						case 'M': info.me = atoi(optarg); break;
      						case 'S': info.channel = atoi(optarg); break;
      						case 'W': info.width = atoi(optarg);  break;
      						case 'H': info.height = atoi(optarg);  break;
      						case 't': drec = atof(optarg);  break;
      						case 'x': videomegs = atoi(optarg);  break;
      						case 'C': if(!FFV1_selectByName(optarg))
      								{
										printf("\n cannot find this codec <%s>\n",optarg);
										exit(0);
								};break;
      						case 'y': audiomegs = atoi(optarg);  break;
      						case 'f': info.frequency = atof(optarg); break;
      						case 'A': audiodevice = optarg;   break;
      						case 'V': videodevice = optarg;   break;
      						case 'h': usage();  break;

					}
					break;
	}
  }
fclose(fd);
}


