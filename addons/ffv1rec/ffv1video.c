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
#include "nuppelvideo.h"

#define MEAN_DO_NO_WANT_V4L2
#include <linux/videodev.h>



#include "ffv1.h"
// we need the BTTV_FIELDNR, so we really know how many frames we lose
#define BTTV_FIELDNR            _IOR('v' , BASE_VIDIOCPRIVATE+2, unsigned int)

static  struct video_mmap mm;
static  struct video_mbuf vm;
static struct video_channel vchan;
static  struct video_audio va;
static  struct video_tuner vt;
static struct video_audio origaudio;
static  unsigned char *buf;

extern int quiet;

extern struct vidbuffertype *videobuffer;
extern int				 video_buffer_count;   // should be a setting from command line  6.3MB 384x288x40
				                            //                                       23.2MB 704x576x40
extern long int  			video_buffer_size;


static int fd;
static void bufferit(unsigned char *buf);
static int usebttv=0;
static struct timeval now, stm;
static struct timezone tzone;
static v4linfo vinfo;
static unsigned int tf=0;
/*
	Init the video input
*/

int initVideoDev(char *videodevice, v4linfo *info )
{
long int v4lfrequency=0;
int volume=-1;

	memcpy(&vinfo,info,sizeof(vinfo));
 	fd = open(videodevice, O_RDWR|O_CREAT);
  	if(fd<=0)
  	{
    		perror("open");
    		return 0;
  	}

  	if(ioctl(fd, VIDIOCGMBUF, &vm)<0)
  	{
  		perror("VIDIOCMCAPTUREi0");
		return 0;
  	}
  	if(vm.frames<2)
  	{
	  	fprintf(stderr, "stoopid prog want min 2 cap buffs!\n");
  		return 0;
  	}

  // fprintf(stderr, "We have vm.frames=%d\n", vm.frames);

  	buf = (unsigned char*)mmap(0, vm.size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  	if (buf<=0)
  	{
   		perror("mmap");
   		return 0;
  	}


  	vchan.channel = info->channel;
  	if(ioctl(fd, VIDIOCGCHAN, &vchan)<0) perror("VIDIOCGCHAN");

  	// choose the right input
  	if(ioctl(fd, VIDIOCSCHAN, &vchan)<0) perror("VIDIOCSCHAN");

  	// if channel has a audio/tuner then activate it
        // this was a check for VIDEO_VC_AUDIO, but somehow my saa7134 card only reports
        // VIDEO_VC_TUNER, and it definitely has audio. Maybe one could omit this check completely?!?
  	if ( (vchan.flags & VIDEO_VC_TUNER)==VIDEO_VC_TUNER) {
    	// we assume only a channel with audio can have a tuner therefore
    	// we only tune here if we are supposed to
    	if (info->frequency != 0.0)
	{
      		v4lfrequency  = ((unsigned long)info->frequency)*16;
      		v4lfrequency |= ((unsigned long)( (info->frequency-(v4lfrequency/16))*100 )*16)/100; // ??????
      		if (ioctl(fd, VIDIOCSFREQ, &v4lfrequency)<0) perror("VIDIOCSFREQ");
      		if (!quiet) fprintf(stderr, "tuner frequency set to '%5.4f' MHz.\n", info->frequency);
    	}
    	if (!quiet) fprintf(stderr, "%s\n", "unmuting tv-audio");
    	// audio hack, to enable audio from tvcard, in case we use a tuner
    	va.audio = 0; // use audio channel 0
    	if (ioctl(fd, VIDIOCGAUDIO, &va)<0) perror("VIDIOCGAUDIO");
    	origaudio = va;
    	if (!quiet) fprintf(stderr, "audio volume was '%d'\n", va.volume);
    	va.audio = 0;
    	va.flags &= ~VIDEO_AUDIO_MUTE; // now this really has to work

    	if ((volume==-1 && va.volume<32768) || volume!=-1) {
      	if (volume==-1)
	{
        	va.volume = 32768;            // no more silence 8-)
      	}
	 else
	 {
        	va.volume = volume;
      	}
      	if (!quiet) fprintf(stderr, "audio volume set to '%d'\n", va.volume);
    	}
    	if (ioctl(fd, VIDIOCSAUDIO, &va)<0) perror("VIDIOCSAUDIO");
  } else {
    if (!quiet) fprintf(stderr, "channel '%d' has no tuner (composite)\n", info->channel);
  }

  // setting video mode
  vt.tuner = 0;
  if(ioctl(fd, VIDIOCGTUNER, &vt)<0) perror("VIDIOCGTUNER");
  if (info->ntsc)         { vt.flags |= VIDEO_TUNER_NTSC;  vt.mode |= VIDEO_MODE_NTSC; }
    else if (info->secam) { vt.flags |= VIDEO_TUNER_SECAM; vt.mode |= VIDEO_MODE_SECAM; }
      else          { vt.flags |= VIDEO_TUNER_PAL;   vt.mode |= VIDEO_MODE_PAL; }
  vt.tuner = 0;
  if(ioctl(fd, VIDIOCSTUNER, &vt)<0) perror("VIDIOCSTUNER");

  // make sure we use the right input
  if(ioctl(fd, VIDIOCSCHAN, &vchan)<0) perror("VIDIOCSCHAN");

  mm.height = info->height;
  mm.width  = info->width;
  mm.format = VIDEO_PALETTE_YUV420P    ; /* YCrCb422 */

  mm.frame  = 0;
  if(ioctl(fd, VIDIOCMCAPTURE, &mm)<0) perror("VIDIOCMCAPTUREi0");
  mm.frame  = 1;
  if(ioctl(fd, VIDIOCMCAPTURE, &mm)<0) perror("VIDIOCMCAPTUREi1");

  	printf("Video init successfull\n");
	return 1;
  }


void closeVideoDev (void)
{
  	// if channel has a audio then activate it
  	if ((vchan.flags & VIDEO_VC_TUNER)==VIDEO_VC_TUNER) {
      printf("resetting audio!\n");
      if (ioctl(fd, VIDIOCSAUDIO, &origaudio)<0) perror("VIDIOCSAUDIO");

    }
}
/*
	Main loop for capturing video

*/
  void captureVideoDev( void )
  {
  int frame;
  	while(1) {
    		frame=0;
    		mm.frame  = 0;
    		if(ioctl(fd, VIDIOCSYNC, &frame)<0) perror("VIDIOCSYNC0");
    		else
		{
      			if(ioctl(fd, VIDIOCMCAPTURE, &mm)<0) perror("VIDIOCMCAPTURE0");
      			DP("Captured 0er");
      			bufferit(buf+vm.offsets[0]);
    		}
    		frame=1;
    		mm.frame  = 1;
    		if(ioctl(fd, VIDIOCSYNC, &frame)<0) perror("VIDIOCSYNC1");
    		else
		{
      			if(ioctl(fd, VIDIOCMCAPTURE, &mm)<0) perror("VIDIOCMCAPTURE1");
      			DP("Captured 1er");
      			bufferit(buf+vm.offsets[1]);
    		}
  }
}
/*-----------------------------------------------------------*/
void bufferit(unsigned char *buf)
{
 static long int act_video_buffer=0;
 int act;
 long tcres;
 //static int firsttc=0;
 static int oldtc=0;
 int fn;

 act = act_video_buffer;

 if (! videobuffer[act].freeToBuffer) {
   // we have to skip the current frame :-(
   // fprintf(stderr, "\rran out of free VIDEO framepages :-(");
   // the fprint only made things slower
   return;
 }

 // get current time for timecode
 gettimeofday(&now, &tzone);

 tcres = (now.tv_sec-stm.tv_sec)*1000 + now.tv_usec/1000 - stm.tv_usec/1000;

 if (usebttv) {
   // i hate it when interfaces changes and a non existent ioctl doesn't make an error
   // and doesn't return -1, returning 0 instead and making no error is really weird
   if (ioctl(fd, BTTV_FIELDNR, &tf)) {
     perror("BTTV_FIELDNR");
     usebttv = 0;
     fprintf(stderr, "\nbttv_fieldnr not supported by bttv-driver"
                     "\nuse insmod/modprobe bttv card=YOURCARD field_nr=1 to activate f.n."
                     "\nfalling back to timecode routine to determine lost frames\n");
   }
   if (tf==0) {
     usebttv = 0;
     fprintf(stderr, "\nbttv_fieldnr not supported by bttv-driver"
                     "\nuse insmod/modprobe bttv card=YOURCARD field_nr=1 to activate f.n."
                     "\nfalling back to timecode routine to determine lost frames\n");
   }
 }

 // here is the non preferable timecode - drop algorithm - fallback
 if (!usebttv) {

   if (tf==0) {
     tf = 2;
   } else {
     fn = tcres - oldtc;

     // the difference should be less than 1,5*timeperframe or we have
     // missed at least one frame, this code might be inaccurate!

     if (vinfo.ntsc) {
       fn = fn/33;
     } else {
       fn = fn/40;
     }
     if (fn==0) fn=1;
     tf+= 2*fn; // two fields
   }
 }
 oldtc = tcres;

 if (! videobuffer[act].freeToBuffer) {
   return; // we can't buffer the current frame
 }

 videobuffer[act].sample = tf;
 videobuffer[act].timecode = tcres;

DP("buffered frame");

 memcpy(videobuffer[act].buffer_offset, buf, video_buffer_size);

 videobuffer[act].freeToBuffer = 0;
 act_video_buffer++;
 if (act_video_buffer >= video_buffer_count) act_video_buffer = 0; // cycle to begin of buffer
 videobuffer[act].freeToEncode = 1; // last setting to prevent race conditions

 return;
}

