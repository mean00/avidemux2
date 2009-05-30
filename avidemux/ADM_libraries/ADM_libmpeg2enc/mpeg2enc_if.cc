/*
	Strongly derivated from mpeg2enc, mean
	 Modifications and enhancements (C) 2000/2001 Andrew Stevens


 * These modifications are free software; you can redistribute it
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

#include <ADM_default.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif


#define GLOBAL /* used by global.h */
#include "global.h"
#include "motionsearch.h"
#include "predict_ref.h"
#include "transfrm_ref.h"
#include "quantize_ref.h"
#include "format_codes.h"
#include "mpegconsts.h"
#include "fastintfns.h"
#ifdef HAVE_ALTIVEC
/* needed for ALTIVEC_BENCHMARK and print_benchmark_statistics() */
//#include "../utils/altivec/altivec_conf.h"
#endif
#include "mpeg2parm.h"
#include "ADM_codecs/ADM_codec.h"
//int verbose = 1;
#define aprintf printf
/* private prototypes */
extern void set_format_presets(mpeg2parm *param,Mpeg2Settings *opt);
extern int  infer_default_params(mpeg2parm *param,Mpeg2Settings *opt);
extern int  check_param_constraints(mpeg2parm *param);
extern void init_mpeg_parms(mpeg2parm *param,Mpeg2Settings *opt);
extern int  infer_mpeg1_aspect_code(  mpeg2parm *param );
extern void init_encoder(void);
extern void init_quantmat(mpeg2parm *param,Mpeg2Settings *opt);
extern void border_mark( uint8_t *frame,
                         int w1, int h1, int w2, int h2);
extern void init_encoder(mpeg2parm *param,Mpeg2Settings *opt);
extern void DisplayFrameRates(void);
extern void DisplayAspectRatios(void);
extern int  parse_custom_matrixfile(char *fname, int dbug);
extern void Usage(void);
extern void parse_custom_option(char *arg,mpeg2parm *param);

extern int push_init( void );
extern int push_cleanup( void );
extern void mpeg_freebuffers(void);

mpeg2parm  myParam,*param;
t_control  myControl;
 int rateCtlDisablePadding;
#define MAX(a,b) ( (a)>(b) ? (a) : (b) )
#define MIN(a,b) ( (a)<(b) ? (a) : (b) )


#ifndef O_BINARY
# define O_BINARY 0
#endif
static int  mpeg_cleanup( void );

/* Command Parameter values.  These are checked and processed for
   defaults etc after parsing.  The resulting values then set the opt->
   variables that control the actual encoder.
*/


/* Input Stream parameter values that have to be further processed to
   set encoding options */


static unsigned int strm_frame_rate_code;



uint16_t custom_intra_quantizer_matrix[64];
uint16_t custom_nonintra_quantizer_matrix[64];
int nerr = 0;

/* MeanX stuff */
void feedOneFrame(char *y, char *u,char *v);

static  int fedPictures=0;
/* MeanX stuff */
 unsigned char *mpeg2enc_buffer=NULL;
int mpegenc_init(mpeg2parm *incoming,int  width, int  height, int  fps1000)
{
/*
	Reset a whole bunch of vars
*/
istrm_nframes=lum_buffer_size=chrom_buffer_size=block_count=mb_width=mb_height=mb_height2=0;
qsubsample_offset=fsubsample_offset=0;
mb_per_pict=0;

/*-----------------------------------------*/

	if(incoming->noPadding) 
	{
		rateCtlDisablePadding=1;
		printf("Padding disabled\n");
	}
		else rateCtlDisablePadding=0;
	fedPictures=0;
	memset(&myControl,0,sizeof(myControl));
	ctl=&myControl;
	// 1 we memset the opt
	memset(opt,0,sizeof(*opt));
	param=&myParam;
	memcpy(param,incoming, sizeof(*param));
#define MPG_FILM 	1
#define MPG_PAL 	3
#define MPG_NTSC	4

	if(fps1000 > 25500 || fps1000 < 24000)
	{

					param->norm='n';
					if(fps1000<25000)
					{
						param->frame_rate=MPG_FILM;
						printf("Detecting FILM format\n");
						if(param->format!=1) // not VCD -> SVCD or DVD
						{
							param->_32_pulldown=1;
							printf("****Activating pulldown\n");
						}
					}
					else
					{

						param->frame_rate=MPG_NTSC;

						printf("Detecting NTSC format\n");
					}
	}
	else
	{
					param->norm='p';
					param->frame_rate=MPG_PAL;
					printf("Detecting PAL format\n");

	}


	opt->horizontal_size=width;
	opt->vertical_size=height;
//	param->fieldenc=0; // progressive

	strm_frame_rate_code=2;
	opt->frame_rate_code=1;



	set_format_presets(param,opt);

	infer_default_params(param,opt);

	check_param_constraints(param);



	aprintf("[mpeg2enc]Encoding MPEG-%d video \n",param->mpeg);
        aprintf("[mpeg2enc]Horizontal size: %d pe \nl",opt->horizontal_size);
        aprintf("[mpeg2enc]Vertical size: %d pel \n",opt->vertical_size);
        aprintf("[mpeg2enc]Aspect ratio code: %d = %s \n",
			param->aspect_ratio,
			mpeg_aspect_code_definition(param->mpeg,param->aspect_ratio));
        aprintf("[mpeg2enc]Frame rate code:   %d = %s \n",
			param->frame_rate,
			mpeg_framerate_code_definition(param->frame_rate));

	if(param->bitrate)
          aprintf("[mpeg2enc]Bitrate: %d KBit/s \n",param->bitrate/1000);
	else
          aprintf( "[mpeg2enc]Bitrate: VCD \n");
	if(param->quant)
          aprintf("[mpeg2enc]Quality factor: %d (Quantisation = %d) (1=best, 31=worst) \n",
                   param->quant,
                   (int)(inv_scale_quant( param->mpeg == 1 ? 0 : 1,
                                          param->quant))
            );

        aprintf("[mpeg2enc]Field order for input: %s \n",
			   mpeg_interlace_code_definition(param->input_interlacing) );

	if( param->seq_length_limit )
	{
          aprintf( "[mpeg2enc]New Sequence every %d Mbytes \n", param->seq_length_limit );
          aprintf( "[mpeg2enc]Assuming non-video stream of %d Kbps \n", param->nonvid_bitrate );
	}
	else
          aprintf( "[mpeg2enc]Sequence unlimited length \n" );

        aprintf("[mpeg2enc]Search radius: %d \n",param->searchrad);

	/* set params */
	init_mpeg_parms(param,opt);

	/* read quantization matrices */
	init_quantmat(param,opt);

	init_encoder(param,opt);
	init_quantizer();
	init_motion();
	init_transform();
	init_predict();
	//putseq();
	push_init();
	putseq_init();
	aprintf("opt->enc_height2 :%d opt->enc_width: %d opt->enc_height2:%d \n",
		opt->enc_height2,opt->enc_width,opt->enc_height);
	return 1;
}
int mpegenc_encode(  char *in,   char *out, int *size,int *flags,int *quant)
{
		int type;

		mpeg2enc_buffer=(unsigned char *)out;
		*size=0;


		feedOneFrame(in,
					in+((opt->horizontal_size*opt->vertical_size*5)>>2),
					in+(opt->horizontal_size*opt->vertical_size)

					);

		fedPictures++;

		if(fedPictures<PREFILL)
		{
			*size=mpeg2enc_buffer-(unsigned char *)out;
			*quant=2;
			return 1;;
		}
		putseq_next(&type,quant);
		#warning : Approximate..
		*quant=map_non_linear_mquant[*quant];
		*size=mpeg2enc_buffer-(unsigned char *)out;
		switch(type)
		{
			case I_TYPE:
						*flags=AVI_KEY_FRAME;
						break;
			case B_TYPE:
						*flags=AVI_B_FRAME;
						break;
			default:
						*flags=0;
						break;
		}
		return 1;

}
int mpegenc_end(void)
{
  uint8_t out[20]; // Temporary buffer to store SEQ_END_CODE
         mpeg2enc_buffer=(unsigned char *)out;
	 putseq_end();
	 mpeg_cleanup(  );
	 mpeg_freebuffers();
	 return 1;
}
/*
	Release (most) memory consumed by mpeg2enc

*/
int  mpeg_cleanup( void )
{
int n,i;
	if(!frame_buffers)
	{
		printf("Trying to clean already cleaned frame_buffers!!!\n");
		return 0;
	}
	// frame buffers is cleaned by mpeg_free
	frame_buffers=NULL;
	push_cleanup();
	if(opt->motion_data)
	{
		delete [] opt->motion_data;
		opt->motion_data=NULL;
	}

	if(lum_mean) delete [] lum_mean;
	lum_mean=NULL;
	printf("frame_buffers cleaned up\n");
	return 1;
}
int mpegenc_setQuantizer(int newQz)
{
	if(newQz<2) newQz=2;
	if(newQz>31) newQz=31;

	param->quant=newQz;
	ctl->quant_floor = inv_scale_quant( param->mpeg == 1 ? 0 : 1,
                                           param->quant );
	return 1;
}
