#include "../../config.h"
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
#include <math.h>

#include "default.h"
#include "nuppelvideo.h"


#define MEAN_DO_NO_WANT_V4L2
#include <linux/videodev.h>

#include "ffv1.h"
static int create_nuppelfile(char *fname, int number, int w, int h);


extern struct audbuffertype *audiobuffer;
extern void *sharedbuffer;
extern int audio_buffer_count;   // should be a setting from command line
extern struct vidbuffertype *videobuffer;
extern int video_buffer_count;   // should be a setting from command line  6.3MB 384x288x40
                            //                                       23.2MB 704x576x40
extern long video_buffer_size;
extern int do_split;
extern int quiet;
extern	audioInfo ainfo;
static FILE *ofd;
static unsigned int byteswritten;
static unsigned long long audiobytes;
static int effectivedsp;
static int act_video_encode=0;
static long long input_len=0;
static long long ftot=0;
static long stot=0;
static long dropped=0;
static long copied=0;
static long lf=0;
static long fsize;
static v4linfo winfo;
static 	unsigned char 			enc_buffer[720*576*3];
static 	 char 			RTheader[100];

/*--shared --*/

static int act_audio_encode=0;


#define KEYFRAMEDIST 30
#define MAXBYTES       2000000000
#define MAXBYTESFORCE  2100000000


void writeitaudio(unsigned char *buf, int fnum, int timecode);
void writeit(unsigned char *buf, int fnum, int timecode);
/*----------------------------------------------------------*/

void writeInit(v4linfo *info)
{
 memcpy(&winfo,info,sizeof(winfo));
 memset(enc_buffer,0,720*576*3);
}
void write_process(void *vname)
{
  int act;
  int actfile=0;
  int videofirst;
  char *fname=(char *)vname;


  fsize=video_buffer_size;
  // init compression lzo ------------------------------
  if (!FFV1_Init(&winfo) )
  {
    fprintf(stderr,"%s\n", "FFV1init failed !!!");
    exit(3);
  }


  printf("Creating file...\n");
  if (0 != create_nuppelfile(fname, 0, winfo.width, winfo.height)) {
    fprintf(stderr, "cannot open %s.nuv for writing\n", fname);
    exit(1);
  }

  printf("Started...\n\n");
  while(1) {
    act = act_video_encode;
    if (!videobuffer[act].freeToEncode && !audiobuffer[act_audio_encode].freeToEncode) {
      // we have no frames in our cycle buffer
//fprintf(stderr,"*");

  if(do_split)
  {
      if (byteswritten > MAXBYTES) {
        actfile++;
        if (0 != create_nuppelfile(fname, actfile, winfo.width, winfo.height)) {
          fprintf(stderr, "cannot open %s-%d.nuv for writing\n", fname, actfile);
          exit(1);
        }
      }
  }
      sync(); 		// Do not buffer too much (dopez)
      usleep(50); // wait a little for next frame and give time to other processes
      continue;   // check for next frame
    }
  if(do_split)
  {
    if (byteswritten > MAXBYTESFORCE) {
      actfile++;
      if (0 != create_nuppelfile(fname, actfile, winfo.width, winfo.height))
      {
        fprintf(stderr, "cannot open %s-%d.nuv for writing\n", fname, actfile);
        exit(1);
      }
    }
 }
// unluckily i didn't find any information at the kernel interface level that
// reports for lost audio :-( -- didn't happen right now, anyway

    if (videobuffer[act].freeToEncode) {
      if (audiobuffer[act_audio_encode].freeToEncode) {
        // check which first
        videofirst = (videobuffer[act].timecode <= audiobuffer[act_audio_encode].timecode);
      } else {
        videofirst = 1;
      }
    } else {
      videofirst = 0;
    }

    if (videofirst) {
      if (videobuffer[act].freeToEncode) {
        DP("before write frame");
        // we have at least 1 frame --> encode and write it :-)
        writeit(videobuffer[act].buffer_offset,
                                  videobuffer[act].sample,
                                  videobuffer[act].timecode);
        DP("after write frame");
        videobuffer[act].sample = 0;
        act_video_encode++;
        if (act_video_encode >= video_buffer_count) act_video_encode = 0; // cycle to begin of buffer
        videobuffer[act].freeToEncode = 0;
        videobuffer[act].freeToBuffer = 1; // last setting to prevent race conditions
      }
    } else {

      // check audio afterward FIXME buffercount for audio/video buffers
      // to work against bufferoverflows!!
      // FIXME check for lost audiobuffers!!!!!!!!!!!

      if (audiobuffer[act_audio_encode].freeToEncode) {
        DP("before write audio frame");
        // we have at least 1 frame --> write it :-)
        writeitaudio( audiobuffer[act_audio_encode].buffer_offset,
                                      audiobuffer[act_audio_encode].sample,
                                      audiobuffer[act_audio_encode].timecode);
        DP("after write audio frame");
        audiobuffer[act_audio_encode].sample = 0;
        audiobuffer[act_audio_encode].freeToEncode = 0;
        audiobuffer[act_audio_encode].freeToBuffer = 1; // last setting to prevent race conditions
        act_audio_encode++;
        if (act_audio_encode >= audio_buffer_count) act_audio_encode = 0; // cycle to begin of buffer
      }

    }
  }
}

/*
-----------------------------

*/
int create_nuppelfile(char *fname, int number, int w, int h)
{
  struct rtfileheader fileheader;
  struct rtframeheader frameheader;
  char realfname[255];

  static const char finfo[12] = "NuppelVideo";
  static const char vers[5]   = "0.05";
  unsigned long int fourcc, datalen;
 char *data;
  // fprintf(stderr, "sizeof(fileheader)=%d  sizeof(frameheader)=%d\n",
  //                  sizeof(fileheader),sizeof(frameheader));

  if (number==0) {
    //RTjpeg_init_compress(tbls, w, h, Q);
    //RTjpeg_init_mcompress();
    snprintf(realfname, 250, "%s.nuv", fname);
  } else {
    snprintf(realfname, 250, "%s-%d.nuv", fname, number);
  }
  ofd=fopen(realfname,"wb");

  if (!ofd)
  {
  	return(-1);
  }

  memcpy(fileheader.finfo, finfo, sizeof(fileheader.finfo));
  memcpy(fileheader.version, vers, sizeof(fileheader.version));
  fileheader.width  = w;
  fileheader.height = h;
  fileheader.desiredwidth  = 0;
  fileheader.desiredheight = 0;
  fileheader.pimode = 'P';
  fileheader.aspect = 1.0;
  if (winfo.ntsc) fileheader.fps = 29.97;
      else  fileheader.fps = 25.0;
  fileheader.videoblocks = -1;
    fileheader.audioblocks = -1;
  fileheader.textsblocks = 0;
  fileheader.keyframedist = KEYFRAMEDIST;
  // write the fileheader
  fwrite( &fileheader, FILEHEADERSIZE,1,ofd);

  /*-----------FFV1 video packet header ----------*/
//
//	Basically fourCC of codec + extraData if needed
// 	For huffman and FFV1 for example
  fourcc=getVideoFourCC();
  datalen=getVideoData( &data );;


  frameheader.frametype = 'M'; // compressor data
  frameheader.comptype  = '0'; // compressor data for RTjpeg
  frameheader.packetlength = 8+datalen;
  // compression configuration header
  fwrite(&frameheader, FRAMEHEADERSIZE,1,ofd);
  // compression configuration data
  fwrite( &fourcc, 4,1,ofd);
  fwrite( &datalen, 4,1,ofd);
  if(datalen)
  {
  	 fwrite(data,datalen,1,ofd);
	 printf("**With %lu extra datas\n\n",datalen);
  }
  //fflush(ofd);
/*----------------------------------------------*/
 /*-----------FFV1 audio packet header ----------*/
//
//	Frequency of capture
// + size of extenstion (0 today)
 if(ainfo.frequency!=44100)
 {
	frameheader.frametype = 'N'; // compressor data
  	frameheader.comptype  = '0'; // compressor data for RTjpeg
  	frameheader.packetlength = 8;
  	// compression configuration header
  	fwrite(&frameheader, FRAMEHEADERSIZE,1,ofd);
  	// compression configuration data
  	fwrite( &ainfo.frequency, 4,1,ofd);
  	datalen=0;
  	fwrite( &datalen, 4,1,ofd);
}

/*----------------------------------------------*/

  byteswritten = FILEHEADERSIZE + 8+datalen + FRAMEHEADERSIZE;
  lf = 0; // that resets framenumber so that seeking in the
          // continues parts works too

  ftot = stot = input_len = 1; // reset the values for compression ratio calculations

  return(0);
}
/*--------------------------------------------------------------------------*/
void writeit(unsigned char *buf, int fnum, int timecode)
{
  uint32_t   out_len;
  struct rtframeheader frameheader;
  int xaa, freecount=0;
  static int startnum=0;
  static int frameofgop=0;
  static int lasttimecode=0;
  int timeperframe=40;


  if (lf==0) { // this will be triggered every new file
    lf=fnum-2;
    startnum=fnum;
  }

  // if (startnum==0) startnum=fnum;

  // count free buffers -- FIXME this can be done with less CPU time!!
  for (xaa=0; xaa < video_buffer_count; xaa++) {
    if (videobuffer[xaa].freeToBuffer) freecount++;
  }





  // see if it's time for a seeker header, sync information and a keyframe

  frameheader.keyframe  = frameofgop;             // no keyframe defaulted

  if (((fnum-startnum)>>1) % winfo.keydist == 0) {
    frameheader.keyframe=0;
    frameofgop=0;
    strcpy(RTheader, "RTjjjjjjjjjjjjjjjjjjjjjjjj");
    fwrite(RTheader, FRAMEHEADERSIZE,1,ofd);
    frameheader.frametype    = 'S';           // sync frame
    frameheader.comptype     = 'V';           // video sync information
    frameheader.filters      = 0;             // no filters applied
    frameheader.packetlength = 0;             // no data packet
    frameheader.timecode     = (fnum-startnum)>>1; // frame number of following frame
    // write video sync info
    fwrite( &frameheader, FRAMEHEADERSIZE,1,ofd);
    frameheader.frametype    = 'S';           // sync frame
    frameheader.comptype     = 'A';           // video sync information
    frameheader.filters      = 0;             // no filters applied
    frameheader.packetlength = 0;             // no data packet
    frameheader.timecode     = effectivedsp;  // effective audio dsp frequency
    // write audio sync info
    fwrite( &frameheader, FRAMEHEADERSIZE,1,ofd);
    byteswritten += 3*FRAMEHEADERSIZE;
    FFV1_videoForceKeyFrame();
  }
	/* compress to FFV1 */
	if(!FFV1_Compress(buf,enc_buffer,&out_len))
	{
		exit(-1);
	}

  frameheader.frametype = 'V'; // video frame
  frameheader.timecode  = timecode;
  frameheader.filters   = 0;             // no filters applied

  dropped = (((fnum-lf)>>1) - 1); // should be += 0 ;-)

  // examples: (10 - 6)/2 -1 = 2-1 = 1 frame to copy  (8)
  //           (10 - 8)/2 -1 = 1-1 = 0 => OK
  //           (10 - 4)/2 -1 = 3-1 = 2 frames to copy (6,8)
  //           (10 - 2)/2 -1 = 4-1 = 3 frames to copy (4,6,8)
  // odd ex.   ( 9 - 5)/2 -1 = 2-1 = 1 frame  to copy (7)
  //           ( 9 - 1)/2 -1 = 4-1 = 3 frames to copy (3,5,7)

  if (dropped>0)
  {
    timeperframe = (timecode - lasttimecode)/(dropped + 1); // the one we got => +1
    frameheader.timecode = lasttimecode + timeperframe;
    lasttimecode = frameheader.timecode;
  }

  // compr ends here
    frameheader.comptype  = 'F';
    frameheader.packetlength = out_len;
    fwrite( &frameheader, FRAMEHEADERSIZE,1,ofd);
    fwrite( enc_buffer, out_len,1,ofd);
    byteswritten += out_len + FRAMEHEADERSIZE;
    stot+=out_len;
    input_len += winfo.width*winfo.height;

  frameofgop++;

  ftot+=(fsize+FRAMEHEADERSIZE); // actual framesize + sizeof header

  // if we have lost frames we insert "copied" frames until we have the
  // exact count because of that we should have no problems with audio
  // sync, as long as we don't loose audio samples :-/

  while (dropped > 0) {
    frameheader.timecode = lasttimecode + timeperframe;
    lasttimecode = frameheader.timecode;
    frameheader.keyframe  = frameofgop;             // no keyframe defaulted
    frameheader.packetlength =  0;   // no additional data needed
    frameheader.frametype    = 'V';  // last frame (or nullframe if it is first)
    frameheader.comptype    = 'L';
    fwrite( &frameheader, FRAMEHEADERSIZE,1,ofd);
    byteswritten += FRAMEHEADERSIZE;
    // we don't calculate sizes for lost frames for compression computation
    dropped--;
    copied++;
    frameofgop++;
  }

  // now we reset the last frame number so that we can find out
  // how many frames we didn't get next time

  lf=fnum;
  lasttimecode = timecode;

  if (!quiet) {
    if ((fnum % 128) == 0)
    {
	uint32_t hh,mm,ss,start,fr;
	double db;

	db= (fnum-startnum)>>1;
	fr= (fnum-startnum)>>1;
	if(winfo.ntsc)
		{
			db=db/29.87;
		}
	else
		{
			db=db/25.;
		}

	start=(uint32_t)ceil(db);
	hh=start/3600;
	start=start-hh*60;
	mm=start/60;
	start=start-mm*60;
	ss=start;

        fprintf(stderr,
                "\rcopied=%ld buf=%d Time : %02lu:%02lu:%02lu f#=%08lu ratio=%3.2f  effdsp=%d  ",
                copied, freecount,
		hh,mm,ss,
		fr,
                (float)((double)ftot)/((double)stot),
                 effectivedsp);
    }
  }
}

// ---------------------------------------------------------------

void writeitaudio(unsigned char *buf, int fnum, int timecode)
{
  struct rtframeheader frameheader;
  static int last_block   = 0;
  static int audio_behind = 0;
  static int firsttc      = -1;
  double mt;
  double eff;
  double abytes;

  //return;

  if (last_block != 0) {
    if (fnum != (last_block+1)) {
      audio_behind = fnum - (last_block+1);
    }
  }

  frameheader.frametype = 'A'; // audio frame
  frameheader.comptype  = '0'; // audio is uncompressed
  frameheader.timecode = timecode;
  frameheader.packetlength = ainfo.bufferSize;

  if (firsttc==-1) {
    firsttc = timecode;
fprintf(stderr, "first timecode=%d\n", firsttc);
  } else {
    timecode -= firsttc; // this is to avoid the lack between the beginning
                         // of recording and the first timestamp, maybe we
                         // can calculate the audio-video +-lack at the beginning too

    abytes = (double)audiobytes; // - (double)audio_buffer_size; // wrong guess ;-)

    // need seconds instead of msec's
    //mt = (double)timecode/1000.0;
    mt = (double)timecode;
    if (mt > 0.0) {
      //eff = (abytes/4.0)/mt;
      //effectivedsp=(int)(100.0*eff);
      eff = (abytes/mt)*((double)100000.0/(double)4.0);
      effectivedsp=(int)eff;
      //fprintf(stderr,"timecode = %d  audiobutes=%d  effectivedsp = %d\n",
      //                timecode, audiobytes, effectivedsp);
    }
  }

  fwrite( &frameheader, FRAMEHEADERSIZE,1,ofd);
  fwrite( buf, ainfo.bufferSize,1,ofd);
  byteswritten += ainfo.bufferSize + FRAMEHEADERSIZE;
  audiobytes += ainfo.bufferSize; // only audio no header!!

  // this will probably never happen and if there would be a
  // 'uncountable' video frame drop -> material==worthless

  if (audio_behind > 0) {
    frameheader.frametype = 'A'; // audio frame
    frameheader.comptype  = 'N'; // output a nullframe with
    frameheader.packetlength = 0;
    fwrite( &frameheader, FRAMEHEADERSIZE,1,ofd);
    byteswritten += FRAMEHEADERSIZE;
    audiobytes += ainfo.bufferSize;
    audio_behind --;
  }

  last_block = fnum;
}
