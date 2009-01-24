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
#include <signal.h>

#include "nuppelvideo.h"

#define MEAN_DO_NO_WANT_V4L2
#include <linux/videodev.h>

#include "ffv1.h"

static unsigned char *buffer;
static struct timeval now, stm;
static struct timezone tzone;
static   int afd;
static  audioInfo *info;
static int act_audio_buffer=0;

extern  int act_audio_encode;
extern struct audbuffertype *audiobuffer;
extern int audio_buffer_count;   // should be a setting from command line


int audioDevPreinit(char *audiodevice,audioInfo *info)
{
  int afmt, afd;
  int frag, channels, rate, blocksize;

  if (-1 == (afd = open(audiodevice, O_RDONLY))) {
    fprintf(stderr, "\n%s\n", "Cannot open DSP, exiting");
    return(0);
  }

  ioctl(afd, SNDCTL_DSP_RESET, 0);

  frag=(8<<16)|(10);//8 buffers, 1024 bytes each
  ioctl(afd, SNDCTL_DSP_SETFRAGMENT, &frag);

  afmt = AFMT_S16_LE;
  ioctl(afd, SNDCTL_DSP_SETFMT, &afmt);
  if (afmt != AFMT_S16_LE) {
    fprintf(stderr, "\n%s\n", "Can't get 16 bit DSP, exiting");
    return(0);
  }

  channels = 2;
  ioctl(afd, SNDCTL_DSP_CHANNELS, &channels);

  /* sample rate */
  rate = info->frequency;
  ioctl(afd, SNDCTL_DSP_SPEED,    &rate);

  if (-1 == ioctl(afd, SNDCTL_DSP_GETBLKSIZE,  &blocksize)) {
    fprintf(stderr, "\n%s\n", "Can't get DSP blocksize, exiting");
    return(0);
  }
  blocksize *= 4;
  fprintf(stderr, "\naudio blocksize = '%d'\n",blocksize);

  // close audio, audio_capture_process opens again later
  close(afd);

  info->bufferSize = blocksize;

  return(1); // everything is ok
}

/***********************************************
// -- AUDIO CAPTURE PROCESS --------
*************************************************/

int initAudioDev( char *audiodevice,audioInfo *myinfo  )
{
  int afmt,trigger;
  int frag, channels, rate, blocksize;

  info=myinfo;
  signal(SIGINT, sighandler); // install sighaendler

  if (-1 == (afd = open(audiodevice, O_RDONLY)))
  {
    fprintf(stderr, "\n%s\n", "Cannot open DSP, exiting");
    return 0;
  }

  ioctl(afd, SNDCTL_DSP_RESET, 0);

  frag=(8<<16)|(10);//8 buffers, 1024 bytes each
  ioctl(afd, SNDCTL_DSP_SETFRAGMENT, &frag);

  afmt = AFMT_S16_LE;
  ioctl(afd, SNDCTL_DSP_SETFMT, &afmt);
  if (afmt != AFMT_S16_LE)
  {
  	fprintf(stderr, "\n%s\n", "Can't get 16 bit DSP, exiting");
	return 0;
  }

  channels = 2;
  ioctl(afd, SNDCTL_DSP_CHANNELS, &channels);

  /* sample rate */
  rate = myinfo->frequency;
  //printf("\n**** Audio set to %d Hz\n\n",rate);
  ioctl(afd, SNDCTL_DSP_SPEED,    &rate);

  if (-1 == ioctl(afd, SNDCTL_DSP_GETBLKSIZE,  &blocksize))
  {
    fprintf(stderr, "\n%s\n", "Can't get DSP blocksize, exiting");
    return 0;
  }

  blocksize*=4;  // allways read 4*blocksize

  if (blocksize != info->bufferSize) {
    	fprintf(stderr, "\nwarning: audio blocksize = '%d' audio_buffer_size='%ld'\n",
                    blocksize, info->bufferSize);
   //FIXME exit(1);
  }


  buffer = (unsigned char *)malloc(info->bufferSize);
  /* trigger record */
  trigger = ~PCM_ENABLE_INPUT;
  ioctl(afd,SNDCTL_DSP_SETTRIGGER,&trigger);

  trigger = PCM_ENABLE_INPUT;
  ioctl(afd,SNDCTL_DSP_SETTRIGGER,&trigger);

  return 1;
}
  /***************************************

  *****************************************/
  void captureAudioDev(void *param)
  {
  // now we can record AUDIO

  int  act, lastread;



  long tcres;

  long long act_audio_sample=0;
  while(1) {
    // get current time for timecode before the samples are 'delivered'
    // so we don't have to calculate the time for recording it, which
    // depends on the audio_buffer_size
    // FIXME? if we are late and the kernel 64k audio buffer is almost
    // full, the timestimp will be wrong

    gettimeofday(&now, &tzone);

    if (info->bufferSize != (lastread = read(afd,buffer,info->bufferSize )))
    {
      fprintf(stderr, "only read %d from %ld bytes from audio device\n", lastread, info->bufferSize );
      perror("read /dev*audiodevice");
      //return buffer;
      // storage old buffer ?
    }
    // fprintf(stderr, "*");

    act = act_audio_buffer;

    if (! audiobuffer[act].freeToBuffer)
    {
      // we have to skip the current frame :-(
      fprintf(stderr, "ran out of free AUDIO buffers :-(\n");
      act_audio_sample ++;
      continue;
    }
DP("buffered audio frame");

    tcres = (now.tv_sec-stm.tv_sec)*1000 + now.tv_usec/1000 - stm.tv_usec/1000;
    audiobuffer[act].sample = act_audio_sample;
    audiobuffer[act].timecode = tcres;

    memcpy(  audiobuffer[act].buffer_offset,
                             			buffer, info->bufferSize );

    audiobuffer[act].freeToBuffer = 0;
    act_audio_buffer++;
    if (act_audio_buffer >= audio_buffer_count) act_audio_buffer = 0; // cycle to begin of buffer
    audiobuffer[act].freeToEncode = 1; // last setting to prevent race conditions

    act_audio_sample ++; // we should get this from the kernel
                         // then we would know when we have lost audio FIXME TODO

    //usleep(10); // dont need to sleep (read is blocking)
    // exit(0); // this stupid process doesn't die!!!!!!!!!!!!!!!!!!!
  }
}

