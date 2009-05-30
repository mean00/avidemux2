/* readpic.c, read source pictures                                          */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */
/* Modifications and enhancements (C) 2000/2001 Andrew Stevens */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

/* Modifications and enhancements 
   (C) 2000/2001 Andrew Stevens, Rainer Johanni

 */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */


#include "ADM_default.h" 
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <string.h>
//#include <pthread.h>
#include <errno.h>
#include "global.h"

#include "mpegconsts.h"
#include "yuv4mpeg.h"


   /* NOTE: access toframes_read *must* be read-only in other threads
	  once the chunk-reading worker thread has been started.
   */


static volatile int frames_read = 0;
static int last_frame = -1;


/* Buffers for frame luminance means */
int luminance_mean_C(uint8_t *frame, int w, int h );
int luminance_mean_MMX(uint8_t *frame, int w, int h );


int luminance_mean(uint8_t *frame, int w, int h )
{
#ifdef HAVE_X86CPU
        if(CpuCaps::hasMMX()) return luminance_mean_MMX(frame,w,h);
#endif
        luminance_mean_C(frame,w,h);
}





int luminance_mean_C(uint8_t *frame, int w, int h )
{
	uint8_t *p = frame;
	uint8_t *lim = frame + w*h;
	int sum = 0;
	while( p < lim )
	{
		sum += (p[0] + p[1]) + (p[2] + p[3]) + (p[4] + p[5]) + (p[6] + p[7]);
		p += 8;
	}
	sum=sum/(w*h);
	return sum;
}
#ifdef HAVE_X86CPU
// MEANX
// code borrowed from ffmpeg by means

static int pix_sum16_mmx(uint8_t * pix, int line_size)
{
    const int h=16;
#ifdef HAVE_X86_64_CPU
    long int rline_size=line_size;
    long int sum;
    long int index= -line_size*h;
#else    
    int rline_size=line_size;
    int sum;
    int index= -line_size*h;
#endif

    __asm __volatile(
                "pxor %%mm7, %%mm7		\n\t"
                "pxor %%mm6, %%mm6		\n\t"
                "1:				\n\t"
                "movq (%2, %1), %%mm0		\n\t"
                "movq (%2, %1), %%mm1		\n\t"
                "movq 8(%2, %1), %%mm2		\n\t"
                "movq 8(%2, %1), %%mm3		\n\t"
                "punpcklbw %%mm7, %%mm0		\n\t"
                "punpckhbw %%mm7, %%mm1		\n\t"
                "punpcklbw %%mm7, %%mm2		\n\t"
                "punpckhbw %%mm7, %%mm3		\n\t"
                "paddw %%mm0, %%mm1		\n\t"
                "paddw %%mm2, %%mm3		\n\t"
                "paddw %%mm1, %%mm3		\n\t"
                "paddw %%mm3, %%mm6		\n\t"
                "add %3, %1			\n\t"
                " js 1b				\n\t"
                "movq %%mm6, %%mm5		\n\t"
                "psrlq $32, %%mm6		\n\t"
                "paddw %%mm5, %%mm6		\n\t"
                "movq %%mm6, %%mm5		\n\t"
                "psrlq $16, %%mm6		\n\t"
                "paddw %%mm5, %%mm6		\n\t"
                "movd %%mm6, %0			\n\t"
                "and $0xFFFF, %0		\n\t"
                : "=&r" (sum), "+r" (index)
                : "r" (pix - index), "r" (rline_size)
        );

        return sum;
}
int luminance_mean_MMX(uint8_t *frame, int w, int h )
{
        uint8_t *p = frame;
        int mean=0;
        int x,y;

        for(y=0; y<h; y+=16)
        {
                p=frame+w*y;
                for(x=0; x<w; x+=16)
                {

                        mean+=pix_sum16_mmx(p, w);
                        p+=16;
                }
        }
        __asm__ ("emms");
        mean=mean/(w*h);
        return mean;
}

#endif
// MEANX
#if 0
int piperead(int fd, uint8_t *buf, int len)
{
   int n, r;

   r = 0;

   while(r<len)
   {
      n = read(fd,buf+r,len-r);
      if(n==0) return r;
      r += n;
   }
   return r;
}
#endif


#ifndef PUSH
/*
static void read_chunk(void)
{
   int n, v, h, i,j, y;
   y4m_frame_info_t fi;


   for(j=0;j<READ_CHUNK_SIZE;++j)
   {
	   if( ctl->parallel_read )
	   {
		   // Unlock during the actual I/O filling buffers to allow
		   // the main thread to run if there are still the frames
		   // it needs.  The main thread atomically signals new_chunk_req
		   // before waiting on new_chunk_ack.
		   // This thread atomically signals new_chunk_ack before
		   // waiting on new_chunk_req.
		   // Thus neither can suspend without first
		   // starting the other.
		   //mjpeg_info( "PRO:  releasing frame buf lock @ %d ", frames_read);

		   pthread_mutex_unlock( &frame_buffer_lock );
	   }
      n = frames_read % frame_buffer_size;

      y4m_init_frame_info (&fi);

      if ((y = y4m_read_frame_header (istrm_fd, &fi)) != Y4M_OK)
	  {
		  if( y != Y4M_ERR_EOF )
			  mjpeg_log (LOG_WARN,
						 "Error reading frame header (%d): code%s!",
						 n,
						 y4m_strerr (n));
         goto EOF_MARK;
      }

      v = opt->vertical_size;
      h = opt->horizontal_size;
      for(i=0;i<v;i++)
         if(piperead(istrm_fd,frame_buffers[n][0]+i*opt->phy_width,h)!=h) goto EOF_MARK;
	  lum_mean[n] = luminance_mean(frame_buffers[n][0], opt->phy_width, opt->phy_height );

      v = opt->chroma_format==CHROMA420 ?
		  opt->vertical_size/2 : opt->vertical_size;
      h = opt->chroma_format!=CHROMA444 ?
		  opt->horizontal_size/2 : opt->horizontal_size;
      for(i=0;i<v;i++)
         if(piperead(istrm_fd,frame_buffers[n][1]+i*opt->phy_chrom_width,h)!=h) goto EOF_MARK;
      for(i=0;i<v;i++)
         if(piperead(istrm_fd,frame_buffers[n][2]+i*opt->phy_chrom_width,h)!=h) goto EOF_MARK;


	  if( ctl->parallel_read )
	  {
		  //
		  // Lock to atomically signal the availability of additional
		  // material from a chunk - waking the main thread
		  // if it suspended because a required frame was
		  // unavailable
		  //
		  //mjpeg_info( "PRO:  waiting for frame buf lock @ %d ", frames_read);
		  pthread_mutex_lock( &frame_buffer_lock );
	  }
	  ++frames_read;

	  if( ctl->parallel_read )
	  {
		  //mjpeg_info( "PRO: Signalling new_chunk_ack @ %d", frames_read );
		  pthread_cond_broadcast( &new_chunk_ack );
	  }

   }

   //
   // When we exit we're holding the lock again so we can
   // ensure we're waiting on new_chunk_req if the main thread is
   // currently running.
   //
   return;

   EOF_MARK:
   mjpeg_debug( "End of input stream detected" );
   if( ctl->parallel_read )
   {
	   pthread_mutex_lock( &frame_buffer_lock );
   }
   last_frame = frames_read-1;
   istrm_nframes = frames_read;
   //mjpeg_info( "Signalling last frame = %d", last_frame );
   if( ctl->parallel_read )
   {
	   //mjpeg_info( "PRO: Signalling new_chunk_ack @ %d", frames_read );
	   pthread_cond_broadcast( &new_chunk_ack );
   }

}


*/
#if 0
static void *read_chunks_worker(void *_dummy)
{
	//mjpeg_info("PRO: requesting frame buf lock" );
    //mjpeg_info( "PRO: has frame buf lock @ %d ", frames_read );
    //mjpeg_info( "PRO: Initial fill of frame buf" );
    pthread_mutex_lock( &frame_buffer_lock );
    read_chunk();
	for(;;)
	{
		//mjpeg_info( "PRO: has frame buf lock @ %d ", frames_read );
		//mjpeg_info( "PRO: Waiting for new_chunk_req " );
		pthread_cond_wait( &new_chunk_req, &frame_buffer_lock );
		//mjpeg_info( "PRO: new_chunk_req regained frame buf lock @  %d ", frames_read ); 
		if( frames_read < istrm_nframes ) 
		{
			read_chunk();
		}
	}
	return NULL;
}


static void start_worker(void)
{
	pthread_attr_t *pattr = NULL;

#ifdef HAVE_PTHREADSTACKSIZE
#define MINSTACKSIZE 200000
	pthread_attr_t attr;
	size_t stacksize;
	
	pthread_attr_init(&attr);
	pthread_attr_getstacksize(&attr, &stacksize);
	
	if (stacksize < MINSTACKSIZE) {
		pthread_attr_setstacksize(&attr, MINSTACKSIZE);
	}
	
	pattr = &attr;
#endif


	if( pthread_create( &worker_thread, pattr, read_chunks_worker, NULL ) != 0 )
	{
		mjpeg_error_exit1( "worker thread creation failed: %s", strerror(errno) );

	}

}
#endif
 /*****************************************************
 *
 *  Read another chunk of frames into the frame buffer if the
 *  specified frame is less than a chunk away from the end of the
 *  buffer.  This version is for when frame input reading is not
 *  multi-threaded and just goes ahead and does it.
 *
 * N.b. if ctl->parallel_read is active then read_chunk signals/locks
 * which could cause problems hence the assert!
 *
 *****************************************************/
   
static void read_chunk_seq( int num_frame )
{
    while(frames_read - num_frame < READ_CHUNK_SIZE &&
          frames_read < istrm_nframes ) 
    {
        read_chunk();
    }
}

 /*****************************************************
 *
 * Request read worker thread to read a chunk of frame into the frame
 * buffer if less than a chunk of frames is left after the specified
 * frame.  Wait for acknowledgement of the reading of a chunk (implying
 * at least one extra frame added to the buffer) if the specified frame
 * is not yet in the buffer.
 *
 * N.b. *must* be called with ctl->parallel_read active as otherwise it
 * will thoroughly deadlocked.
 *
 *****************************************************/
#if 0   


static void read_chunk_par( int num_frame)
{
	//mjpeg_info( "CON: requesting frame buf lock");
	pthread_mutex_lock( &frame_buffer_lock);
	for(;;)
	{
		//mjpeg_info( "CON: has frame buf lock @ %d (%d recorded read)", frames_read,  num_frame );
		// Activate reader process "on the fly"
		if( frames_read - num_frame < READ_CHUNK_SIZE && 
			frames_read < istrm_nframes )
		{
			//mjpeg_info( "CON: Running low on frames: signalling new_chunk_req" );

			pthread_cond_broadcast( &new_chunk_req );
		}
		if( frames_read > num_frame  || 
			frames_read >= istrm_nframes )
		{
			//mjpeg_info( "CON:  releasing frame buf lock - enough frames to go on with...");
			pthread_mutex_unlock( &frame_buffer_lock );
			return;
		}
		//mjpeg_info( "CON: waiting for new_chunk_ack - too few frames" );
		pthread_cond_wait( &new_chunk_ack, &frame_buffer_lock );
		//mjpeg_info( "CON: regained frame buf lock @ %d (%d processed)", frames_read,  num_frame );

	}
	
}
#endif
static void load_frame( int num_frame )
{
	printf("Push %d\n",num_frame);	
	if(last_frame>=0 && num_frame>last_frame &&num_frame<istrm_nframes)
	{
		mjpeg_error("Internal:readframe: internal error reading beyond end of frames");
		abort();
	}
	
	if( frames_read == 0)
	{
#if 0	
#ifdef __linux__
		pthread_mutexattr_t mu_attr;
		pthread_mutexattr_t *p_attr = &mu_attr;
		pthread_mutexattr_settype( &mu_attr, PTHREAD_MUTEX_ERRORCHECK );

#else
		pthread_mutexattr_t *p_attr = NULL;		
#endif		
		pthread_mutex_init( &frame_buffer_lock, p_attr );
#endif		

        lum_mean = new int[frame_buffer_size];

		/*
          Pre-fill the buffer with one chunk of frames...
        */
	/*	if( ctl->parallel_read )
        {
			start_worker();
            read_chunk_par( num_frame);
        }
        else*/
        {
            read_chunk_seq( num_frame);
        }
 
	}

   /* Read a chunk of frames if we've got less than one chunk buffered
	*/

/*   if( ctl->parallel_read )
	   read_chunk_par( num_frame );
   else*/
	   read_chunk_seq( num_frame );

   /* We aren't allowed to go too far behind the last read
	  either... */

   if( num_frame+static_cast<int>(frame_buffer_size) < frames_read )
   {
	   mjpeg_error("Internal:readframe: %d internal error - buffer flushed too soon", frame_buffer_size );
	   abort();
   }
	printf("Pushed %d %d\n",num_frame,frames_read);	

	
}
int readframe( int num_frame,
               unsigned char *frame[]
	       	)
{
   int n;

   load_frame( num_frame ); 
   n = num_frame % frame_buffer_size;
   frame[0] = frame_buffers[n][0];
   frame[1] = frame_buffers[n][1];
   frame[2] = frame_buffers[n][2];

   return 0;
}
int frame_lum_mean( int num_frame )
{
	int n = num_frame;
//	printf("lum mean %d\n",num_frame);
    //
    // We use this function to probe for the existence of frames
    // so we clip at the end if the end is already found...
    //
	if(  last_frame > 0 && num_frame > last_frame )
	{
		n = last_frame;
	}
	load_frame( n );
    //
    // We may now know where the last frame is...
    //
    if( last_frame > 0 && n > last_frame )
        n = last_frame;
//	printf("lum mean %d\n",n);
	return lum_mean[n% frame_buffer_size];
}

#endif
#if 0
void read_stream_params( unsigned int *hsize,
                         unsigned int *vsize, 
                         unsigned int *frame_rate_code,
                         unsigned int *interlacing_code,
                         unsigned int *aspect_ratio_code)
{
   int n;
   y4m_ratio_t sar;
   y4m_stream_info_t si;

   y4m_init_stream_info (&si);  
   if ((n = y4m_read_stream_header (istrm_fd, &si)) != Y4M_OK) {
       mjpeg_log(LOG_ERROR, "Could not read YUV4MPEG2 header: %s!",
                 y4m_strerr(n));
      exit (1);
   }

   *hsize = y4m_si_get_width(&si);
   *vsize = y4m_si_get_height(&si);
   *frame_rate_code = mpeg_framerate_code(y4m_si_get_framerate(&si));
   *interlacing_code = y4m_si_get_interlace(&si);

   /* Deduce MPEG aspect ratio from stream's frame size and SAR...
      (always as an MPEG-2 code; that's what caller expects). */
   sar = y4m_si_get_sampleaspect(&si);
   *aspect_ratio_code = mpeg_guess_mpeg_aspect_code(2, sar,
                                                    *hsize, *vsize);
}
#endif

/*
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
