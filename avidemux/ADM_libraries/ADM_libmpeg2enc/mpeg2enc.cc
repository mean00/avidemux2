#if 0
/* mpeg2enc.c, main() and parameter file reading                            */

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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
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
#include "../utils/altivec/altivec_conf.h"
#endif
#include "mpeg2parm.h"
int verbose = 1;

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
mpeg2parm  myParam,*param;
#define MAX(a,b) ( (a)>(b) ? (a) : (b) )
#define MIN(a,b) ( (a)<(b) ? (a) : (b) )

#ifndef O_BINARY
# define O_BINARY 0
#endif


/* Command Parameter values.  These are checked and processed for
   defaults etc after parsing.  The resulting values then set the opt->
   variables that control the actual encoder.
*/


/* Input Stream parameter values that have to be further processed to
   set encoding options */

static mpeg_aspect_code_t strm_aspect_ratio;
static unsigned int strm_frame_rate_code;



uint16_t custom_intra_quantizer_matrix[64];
uint16_t custom_nonintra_quantizer_matrix[64];
int nerr = 0;

/* MeanX stuff */
void y4grabFrame(char *y,char *u,char *v);
void feedOneFrame(char *y, char *u,char *v);
char yBuff[720*576];
char uBuff[720*576];
char vBuff[720*576];
static  int fedPictures=0;
/* MeanX stuff */

static const char	short_options[]=
	"m:a:f:n:b:z:T:B:q:o:S:I:r:M:4:2:Q:X:D:g:G:v:V:F:N:tpdsZHOcCPK:";

static struct option long_options[]={
     { "verbose",           1, 0, 'v' },
     { "format",            1, 0, 'f' },
     { "aspect",            1, 0, 'a' },
     { "frame-rate",        1, 0, 'F' },
     { "video-bitrate",     1, 0, 'b' },
     { "nonvideo-bitrate",  1, 0, 'B' },
     { "intra_dc_prec",     1, 0, 'D' },
     { "quantisation",      1, 0, 'q' },
     { "output",            1, 0, 'o' },
     { "target-still-size", 1, 0, 'T' },
     { "interlace-mode",    1, 0, 'I' },
     { "motion-search-radius", 1, 0, 'r'},
     { "reduction-4x4",  1, 0, '4'},
     { "reduction-2x2",  1, 0, '2'},
     { "min-gop-size",      1, 0, 'g'},
     { "max-gop-size",      1, 0, 'G'},
     { "closed-gop",        1, 0, 'c'},
     { "force-b-b-p", 0, &param->preserve_B, 1},
     { "quantisation-reduction", 1, 0, 'Q' },
     { "quant-reduction-max-var", 1, 0, 'X' },
     { "video-buffer",      1, 0, 'V' },
     { "video-norm",        1, 0, 'n' },
     { "sequence-length",   1, 0, 'S' },
     { "3-2-pulldown",      1, &param->_32_pulldown, 1 },
     { "keep-hf",           0, 0, 'H' },
     { "reduce-hf",         1, 0, 'N' },
     { "sequence-header-every-gop", 0, &param->seq_hdr_every_gop, 1},
     { "no-dummy-svcd-SOF", 0, &param->svcd_scan_data, 0 },
     { "correct-svcd-hds", 0, &param->hack_svcd_hds_bug, 0},
     { "no-constraints", 0, &param->ignore_constraints, 1},
     { "no-altscan-mpeg2", 0, &param->hack_altscan_bug, 1},
     { "playback-field-order", 1, 0, 'z'},
     { "multi-thread",      1, 0, 'M' },
     { "custom-quant-matrices", 1, 0, 'K'},
     { "help",              0, 0, '?' },
     { 0,                   0, 0, 0 }
};


int main( int argc,	char *argv[] )
{
	char *outfilename=0;
	int n;

	/* Set up error logging.  The initial handling level is LOG_INFO
	 */


	memset(&myParam,0,sizeof(myParam));
	myParam.setDefault();
	param=&myParam;
#ifdef HAVE_GETOPT_LONG

    while( (n=getopt_long(argc,argv,short_options,long_options, NULL)) != -1 )
#else
    while( (n=getopt(argc,argv,short_options)) != -1)
#endif
	{
		switch(n) {
        case 0 :                /* Flag setting handled by getopt-long */
            break;
		case 'b':
			param->bitrate = atoi(optarg)*1000;
			break;

		case 'T' :
			param->still_size = atoi(optarg)*1024;
			if( param->still_size < 20*1024 || param->still_size > 500*1024 )
			{
				mjpeg_error( "-T requires arg 20..500" );
				++nerr;
			}
			break;

		case 'B':
			param->nonvid_bitrate = atoi(optarg);
			if( param->nonvid_bitrate < 0 )
			{
				mjpeg_error("-B requires arg > 0");
				++nerr;
			}
			break;

        case 'D':
            param->mpeg2_dc_prec = atoi(optarg)-8;
            if( param->mpeg2_dc_prec < 0 || param->mpeg2_dc_prec > 2 )
            {
                mjpeg_error( "-D requires arg [8..10]" );
                ++nerr;
            }
            break;
        case 'C':
            param->hack_svcd_hds_bug = 0;
            break;

		case 'q':
			param->quant = atoi(optarg);
			if(param->quant<1 || param->quant>32)
			{
				mjpeg_error("-q option requires arg 1 .. 32");
				++nerr;
			}
			break;

        case 'a' :
			param->aspect_ratio = atoi(optarg);
            if( param->aspect_ratio == 0 )
				DisplayAspectRatios();
			/* Checking has to come later once MPEG 1/2 has been selected...*/
			if( param->aspect_ratio < 0 )
			{
				mjpeg_error( "-a option must be positive");
				++nerr;
			}
			break;

       case 'F' :
			param->frame_rate = atoi(optarg);
            if( param->frame_rate == 0 )
				DisplayFrameRates();
			if( param->frame_rate < 0 || 
				param->frame_rate >= mpeg_num_framerates)
			{
				mjpeg_error( "-F option must be [0..%d]", 
						 mpeg_num_framerates-1);
				++nerr;
			}
			break;

		case 'o':
			outfilename = optarg;
			break;

		case 'I':
			param->fieldenc = atoi(optarg);
			if( param->fieldenc < 0 || param->fieldenc > 2 )
			{
				mjpeg_error("-I option requires 0,1 or 2");
				++nerr;
			}
			break;

		case 'r':
			param->searchrad = atoi(optarg);
			if(param->searchrad<0 || param->searchrad>32)
			{
				mjpeg_error("-r option requires arg 0 .. 32");
				++nerr;
			}
			break;

		case 'M':
			param->num_cpus = atoi(optarg);
			if(param->num_cpus<0 || param->num_cpus>32)
			{
				mjpeg_error("-M option requires arg 0..32");
				++nerr;
			}
			break;

		case '4':
			param->_44_red = atoi(optarg);
			if(param->_44_red<0 || param->_44_red>4)
			{
				mjpeg_error("-4 option requires arg 0..4");
				++nerr;
			}
			break;
			
		case '2':
			param->_22_red = atoi(optarg);
			if(param->_22_red<0 || param->_22_red>4)
			{
				mjpeg_error("-2 option requires arg 0..4");
				++nerr;
			}
			break;

		case 'v':
			verbose = atoi(optarg);
			if( verbose < 0 || verbose > 2 )
				++nerr;
			break;
		case 'V' :
			param->video_buffer_size = atoi(optarg);
			if(param->video_buffer_size<20 || param->video_buffer_size>4000)
			{
				mjpeg_error("-v option requires arg 20..4000");
				++nerr;
			}
			break;

		case 'S' :
			param->seq_length_limit = atoi(optarg);
			if(param->seq_length_limit<1 )
			{
				mjpeg_error("-S option requires arg > 1");
				++nerr;
			}
			break;
		case 'p' :
			param->_32_pulldown = 1;
			break;

		case 'z' :
			if( strlen(optarg) != 1 || (optarg[0] != 't' && optarg[0] != 'b' ) )
			{
				mjpeg_error("-z option requires arg b or t" );
				++nerr;
			}
			else if( optarg[0] == 't' )
				param->force_interlacing = Y4M_ILACE_TOP_FIRST;
			else if( optarg[0] == 'b' )
				param->force_interlacing = Y4M_ILACE_BOTTOM_FIRST;
			break;

		case 'f' :
			param->format = atoi(optarg);
			if( param->format < MPEG_FORMAT_FIRST ||
				param->format > MPEG_FORMAT_LAST )
			{
				mjpeg_error("-f option requires arg [%d..%d]", 
							MPEG_FORMAT_FIRST, MPEG_FORMAT_LAST);
				++nerr;
			}
				
			break;

		case 'n' :
			switch( optarg[0] )
			{
			case 'p' :
			case 'n' :
			case 's' :
				param->norm = optarg[0];
				break;
			default :
				mjpeg_error("-n option requires arg n or p, or s.");
				++nerr;
			}
			break;
		case 'g' :
			param->min_GOP_size = atoi(optarg);
			break;
		case 'G' :
			param->max_GOP_size = atoi(optarg);
			break;
        	case 'c' :
            		param->closed_GOPs = true;
            		break;
		case 'P' :
			param->preserve_B = true;
			break;
		case 'N':
                        param->hf_q_boost = atof(optarg);
            		if (param->hf_q_boost <0.0 || param->hf_q_boost > 2.0)
            		   {
                	   mjpeg_error( "-N option requires arg 0.0 .. 2.0" );
                	   ++nerr;
			   param->hf_q_boost = 0.0;
            		   }
			if (param->hf_quant == 0 && param->hf_q_boost != 0.0)
			   param->hf_quant = 1;
			break;
		case 'H':
			param->hf_quant = 2;
            		break;
		case 'K':
			parse_custom_option(optarg,param);
			break;
		case 's' :
			param->seq_hdr_every_gop = 1;
			break;
		case 'd' :
			param->svcd_scan_data = 0;
			break;
		case 'Q' :
			param->act_boost = atof(optarg);
			if( param->act_boost <-4.0 || param->act_boost > 10.0)
			{
				mjpeg_error( "-q option requires arg -4.0 .. 10.0");
				++nerr;
			}
			break;
		case 'X' :
			param->boost_var_ceil = atof(optarg);
			if( param->act_boost <0 || param->act_boost > 50*50 )
			{
				mjpeg_error( "-X option requires arg 0 .. 2500" );
				++nerr;
			}
			break;
		case ':' :
			mjpeg_error( "Missing parameter to option!" );
		case '?':
		default:
			++nerr;
		}
	}

    if( nerr )
		Usage();

	mjpeg_default_handler_verbosity(verbose);


	/* Select input stream */
	if(optind!=argc)
	{
		if( optind == argc-1 )
		{
			istrm_fd = open( argv[optind], O_RDONLY | O_BINARY );
			if( istrm_fd < 0 )
			{
				mjpeg_error( "Unable to open: %s: ",argv[optind] );
				perror("");
				++nerr;
			}
		}
		else
			++nerr;
	}
	else
		istrm_fd = 0; /* stdin */

	/* Read parameters inferred from input stream */
	read_stream_params( &opt->horizontal_size, &opt->vertical_size, 
						&strm_frame_rate_code, &param->input_interlacing,
						&strm_aspect_ratio
						);
	
	if(opt->horizontal_size<=0)
	{
		mjpeg_error("Horizontal size from input stream illegal");
		++nerr;
	}
	if(opt->vertical_size<=0)
	{
		mjpeg_error("Vertical size from input stream illegal");
		++nerr;
	}

	
	/* Check parameters that cannot be checked when parsed/read */

	if(!outfilename)
	{
		mjpeg_error("Output file name (-o option) is required!");
		++nerr;
	}

	

	set_format_presets(param,opt);

	nerr += infer_default_params(param,opt);

	nerr += check_param_constraints(param);

	if(nerr) 
	{
		Usage();
	}


	mjpeg_info("Encoding MPEG-%d video to %s",param->mpeg,outfilename);
	mjpeg_info("Horizontal size: %d pel",opt->horizontal_size);
	mjpeg_info("Vertical size: %d pel",opt->vertical_size);
	mjpeg_info("Aspect ratio code: %d = %s", 
			param->aspect_ratio,
			mpeg_aspect_code_definition(param->mpeg,param->aspect_ratio));
	mjpeg_info("Frame rate code:   %d = %s",
			param->frame_rate,
			mpeg_framerate_code_definition(param->frame_rate));

	if(param->bitrate) 
		mjpeg_info("Bitrate: %d KBit/s",param->bitrate/1000);
	else
		mjpeg_info( "Bitrate: VCD");
	if(param->quant) 
		mjpeg_info("Quality factor: %d (Quantisation = %d) (1=best, 31=worst)",
                   param->quant, 
                   (int)(inv_scale_quant( param->mpeg == 1 ? 0 : 1, 
                                          param->quant))
            );

	mjpeg_info("Field order for input: %s", 
			   mpeg_interlace_code_definition(param->input_interlacing) );

	if( param->seq_length_limit )
	{
		mjpeg_info( "New Sequence every %d Mbytes", param->seq_length_limit );
		mjpeg_info( "Assuming non-video stream of %d Kbps", param->nonvid_bitrate );
	}
	else
		mjpeg_info( "Sequence unlimited length" );

	mjpeg_info("Search radius: %d",param->searchrad);

	/* set params */
	init_mpeg_parms(param,opt);

	/* read quantization matrices */
	init_quantmat(param,opt);

	/* open output file */
	if (!(outfile=fopen(outfilename,"wb")))
	{
		mjpeg_error_exit1("Couldn't create output file %s",outfilename);
	}

	init_encoder(param,opt);
	init_quantizer();
	init_motion();
	init_transform();
	init_predict();
	//putseq();
	push_init();
	putseq_init();
	for(int i=0;i<200;i++)
	{
		y4grabFrame(yBuff,uBuff,vBuff);
	
		feedOneFrame(yBuff,uBuff,vBuff);
	
		fedPictures++;
	
		if(fedPictures<PREFILL) continue;

		printf(">> %d / 100 \n",i);
		putseq_next();
	}
	putseq_end();

	fclose(outfile);
#ifdef OUTPUT_STAT
	if( statfile != NULL )
		fclose(statfile);
#endif
#ifdef ALTIVEC_BENCHMARK
	print_benchmark_statistics();
#endif
	return 0;
}



/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
#endif
