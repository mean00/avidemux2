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
/*

	MeanX : This is the equivalent of readpic.cc
	put push frame insted of pulling them
	We prebuff PREFILL frame to not have problems
		then do 1 in/ 1 out.
*/
#include <ADM_default.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "global.h"

#include "mpegconsts.h"
#include "yuv4mpeg.h"
extern int piperead(int fd, uint8_t *buf, int len);
extern int luminance_mean(uint8_t *frame, int w, int h );
static volatile int frames_read = 0;
int push_init(void)
{
        lum_mean = new int[frame_buffer_size];
	frames_read=0;
	return 0;

}
int push_cleanup( void )
{
	if(lum_mean)
	{
		delete [] lum_mean;
	}
	lum_mean=NULL;

}
int pushframe( int num_frame, unsigned char *frame[])
{
	int n;

   //load_frame( num_frame  
	feedframe_buffer(num_frame);
      //-----------------
   n = num_frame % frame_buffer_size;
   frame[0] = frame_buffers[n][0];
   frame[1] = frame_buffers[n][1];
   frame[2] = frame_buffers[n][2];
   
   

   return 0;
}
#if 0
void feedframe_buffer(int num_frame)
{
   int n, v, h, i,j, y;
   y4m_frame_info_t fi;
   printf("Push : %d %d\n",num_frame,frames_read);
while(frames_read - num_frame < READ_CHUNK_SIZE )
   {
      n = frames_read % frame_buffer_size;

      y4m_init_frame_info (&fi);

      y = y4m_read_frame_header (istrm_fd, &fi);
      
      v = opt->vertical_size;
      h = opt->horizontal_size;
      for(i=0;i<v;i++)
         piperead(istrm_fd,frame_buffers[n][0]+i*opt->phy_width,h);
	  lum_mean[n] = luminance_mean(frame_buffers[n][0], opt->phy_width, opt->phy_height );

      v = opt->chroma_format==CHROMA420 ? 
		  opt->vertical_size/2 : opt->vertical_size;
      h = opt->chroma_format!=CHROMA444 ? 
		  opt->horizontal_size/2 : opt->horizontal_size;
      for(i=0;i<v;i++)
         piperead(istrm_fd,frame_buffers[n][1]+i*opt->phy_chrom_width,h);
      for(i=0;i<v;i++)
         piperead(istrm_fd,frame_buffers[n][2]+i*opt->phy_chrom_width,h);


	  ++frames_read;
   }
   printf("Pushed : %d %d\n",num_frame,frames_read);

}
#else
void feedframe_buffer(int num_frame)
{
	//if(num_frame>frames_read+PREFILL)
	{
		//printf("!!!!! FRAME UNDERFLOW frame asked : %d read:%d prefill:%d !!!\n",num_frame,frames_read,PREFILL);
	//	exit( -1);
	}
}

#endif
int frame_lum_mean( int num_frame )
{
	int n = num_frame;
//	printf("lum mean %d\n",num_frame);
	feedframe_buffer( n );
//	printf("lum mean %d\n",n);
	return lum_mean[n% frame_buffer_size];
}
void feedOneFrame(char *y, char *u,char *vv)
{
	int n,v,h,i;
      	n = frames_read % frame_buffer_size;

      	v = opt->vertical_size;
      	h = opt->horizontal_size;
	memcpy(frame_buffers[n][0],y,v*h);

	lum_mean[n] = luminance_mean(frame_buffers[n][0], opt->phy_width, opt->phy_height );

	memcpy(frame_buffers[n][1],u,(v*h)>>2);
	memcpy(frame_buffers[n][2],vv,(v*h)>>2);

	++frames_read;	  

}
