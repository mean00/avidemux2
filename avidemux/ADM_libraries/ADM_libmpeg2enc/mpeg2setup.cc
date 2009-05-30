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
/* Split away conf from command line parsing : MEANX 

	Here we only use param and opt structure
*/
#define NO_GLOBAL
#include <ADM_default.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif


#include "global.h"
#include "motionsearch.h"
#include "predict_ref.h"
#include "transfrm_ref.h"
#include "quantize_ref.h"
#include "format_codes.h"
#include "mpegconsts.h"
#include "fastintfns.h"
#include "mpeg2parm.h"

#define MAX(a,b) ( (a)>(b) ? (a) : (b) )
#define MIN(a,b) ( (a)<(b) ? (a) : (b) )

void init_mpeg_parms(mpeg2parm *param,Mpeg2Settings *opt);
int quant_hfnoise_filt(int orgquant, int qmat_pos,mpeg2parm *param );
void init_quantmat(mpeg2parm *param,Mpeg2Settings *opt);
void set_format_presets(mpeg2parm *param,Mpeg2Settings *opt);
/* TODO FIXME FIXME */
extern uint16_t custom_intra_quantizer_matrix[64];
extern uint16_t custom_nonintra_quantizer_matrix[64];
/* TODO FIXME FIXME */

void set_format_presets(mpeg2parm *param,Mpeg2Settings *opt)
{
	switch( param->format  )
	{
	case MPEG_FORMAT_MPEG1 :  /* Generic MPEG1 */
		mjpeg_info( "Selecting generic MPEG1 output profile");
		if( param->video_buffer_size == 0 )
			param->video_buffer_size = 46;
		if( param->bitrate == 0 )
			param->bitrate = 1151929;
		break;

	case MPEG_FORMAT_VCD :
		param->mpeg = 1;
		param->bitrate = 1151929;
		param->video_buffer_size = 46;
        param->preserve_B = true;
        param->min_GOP_size = 9;
		param->max_GOP_size = param->norm == 'n' ? 18 : 15;
		mjpeg_info("VCD default options selected");
		
	case MPEG_FORMAT_VCD_NSR : /* VCD format, non-standard rate */
		mjpeg_info( "Selecting VCD output profile");
		param->mpeg = 1;
		param->svcd_scan_data = 0;
		param->seq_hdr_every_gop = 1;
		if( param->bitrate == 0 )
			param->bitrate = 1151929;
		if( param->video_buffer_size == 0 )
			param->video_buffer_size = 46 * param->bitrate / 1151929;
        if( param->seq_length_limit == 0 )
            param->seq_length_limit = 700;
        if( param->nonvid_bitrate == 0 )
            param->nonvid_bitrate = 230;
		break;
		
	case  MPEG_FORMAT_MPEG2 : 
		param->mpeg = 2;
		mjpeg_info( "Selecting generic MPEG2 output profile");
		param->mpeg = 2;
		if( param->fieldenc == -1 )
			param->fieldenc = 1;
		if( param->video_buffer_size == 0 )
			param->video_buffer_size = 46 * param->bitrate / 1151929;
		break;

	case MPEG_FORMAT_SVCD :
		mjpeg_info("SVCD standard settings selected");
//		param->bitrate = 2500000;
		param->min_GOP_size=param->max_GOP_size = param->norm == 'n' ? 18 : 15;
                if(param->ignore_constraints)
                {
                  param->video_buffer_size = 4000;
                }else
		param->video_buffer_size = 230;

	case  MPEG_FORMAT_SVCD_NSR :		/* Non-standard data-rate */
		mjpeg_info( "Selecting SVCD output profile");
		param->mpeg = 2;
		if( param->fieldenc == -1 )
			param->fieldenc = 1;
		/*if( param->quant == 0 )
			param->quant = 8;*/
		if( param->svcd_scan_data == -1 )
			param->svcd_scan_data = 1;
		if( param->min_GOP_size == -1 )
            param->min_GOP_size = 9;
        param->seq_hdr_every_gop = 1;
        if( param->seq_length_limit == 0 )
            param->seq_length_limit = 700;
        if( param->nonvid_bitrate == 0 )
            param->nonvid_bitrate = 230;
        break;

	case MPEG_FORMAT_VCD_STILL :
		mjpeg_info( "Selecting VCD Stills output profile");
		param->mpeg = 1;
		/* We choose a generous nominal bit-rate as its VBR anyway
		   there's only one frame per sequence ;-). It *is* too small
		   to fill the frame-buffer in less than one PAL/NTSC frame
		   period though...*/
		param->bitrate = 8000000;

		/* Now we select normal/hi-resolution based on the input stream
		   resolution. 
		*/
		
		if( opt->horizontal_size == 352 && 
			(opt->vertical_size == 240 || opt->vertical_size == 288 ) )
		{
			/* VCD normal resolution still */
			if( param->still_size == 0 )
				param->still_size = 30*1024;
			if( param->still_size < 20*1024 || param->still_size > 42*1024 )
			{
				mjpeg_error_exit1( "VCD normal-resolution stills must be >= 20KB and <= 42KB each");
			}
			/* VBV delay encoded normally */
			param->vbv_buffer_still_size = 46*1024;
			param->video_buffer_size = 46;
			param->pad_stills_to_vbv_buffer_size = 0;
		}
		else if( opt->horizontal_size == 704 &&
				 (opt->vertical_size == 480 || opt->vertical_size == 576) )
		{
			/* VCD high-resolution stills: only these use vbv_delay
			 to encode picture size...
			*/
			if( param->still_size == 0 )
				param->still_size = 125*1024;
			if( param->still_size < 46*1024 || param->still_size > 220*1024 )
			{
				mjpeg_error_exit1( "VCD normal-resolution stills should be >= 46KB and <= 220KB each");
			}
			param->vbv_buffer_still_size = param->still_size;
			param->video_buffer_size = 224;
			param->pad_stills_to_vbv_buffer_size = 1;			
		}
		else
		{
			mjpeg_error("VCD normal resolution stills must be 352x288 (PAL) or 352x240 (NTSC)");
			mjpeg_error_exit1( "VCD high resolution stills must be 704x576 (PAL) or 704x480 (NTSC)");
		}
		param->quant = 0;		/* We want to try and hit our size target */
		
		param->seq_hdr_every_gop = 1;
		param->seq_end_every_gop = 1;
		param->min_GOP_size = 1;
		param->max_GOP_size = 1;
		break;

	case MPEG_FORMAT_SVCD_STILL :
		mjpeg_info( "Selecting SVCD Stills output profile");
		param->mpeg = 2;
		if( param->fieldenc == -1 )
			param->fieldenc = 1;
		/* We choose a generous nominal bit-rate as its VBR anyway
		   there's only one frame per sequence ;-). It *is* too small
		   to fill the frame-buffer in less than one PAL/NTSC frame
		   period though...*/

		param->bitrate = 2500000;
		param->video_buffer_size = 230;
		param->vbv_buffer_still_size = 220*1024;
		param->pad_stills_to_vbv_buffer_size = 0;

		/* Now we select normal/hi-resolution based on the input stream
		   resolution. 
		*/
		
		if( opt->horizontal_size == 480 && 
			(opt->vertical_size == 480 || opt->vertical_size == 576 ) )
		{
			mjpeg_info( "SVCD normal-resolution stills selected." );
			if( param->still_size == 0 )
				param->still_size = 90*1024;
		}
		else if( opt->horizontal_size == 704 &&
				 (opt->vertical_size == 480 || opt->vertical_size == 576) )
		{
			mjpeg_info( "SVCD high-resolution stills selected." );
			if( param->still_size == 0 )
				param->still_size = 125*1024;
		}
		else
		{
			mjpeg_error("SVCD normal resolution stills must be 480x576 (PAL) or 480x480 (NTSC)");
			mjpeg_error_exit1( "SVCD high resolution stills must be 704x576 (PAL) or 704x480 (NTSC)");
		}

		if( param->still_size < 30*1024 || param->still_size > 200*1024 )
		{
			mjpeg_error_exit1( "SVCD resolution stills must be >= 30KB and <= 200KB each");
		}


		param->seq_hdr_every_gop = 1;
		param->seq_end_every_gop = 1;
		param->min_GOP_size = 1;
		param->max_GOP_size = 1;
		break;


	case MPEG_FORMAT_DVD :
	case MPEG_FORMAT_DVD_NAV :
		printf( "Selecting DVD output profile\n");
		
		if( param->bitrate == 0 )
			param->bitrate = 7500000;
		if( param->fieldenc == -1 )
			param->fieldenc = 1;
                if(param->ignore_constraints)
                {
                  param->video_buffer_size = 4000;
                }else
		  param->video_buffer_size = 230;
		param->mpeg = 2;
		if( param->quant == 0 )
			param->quant = 8;
		param->seq_hdr_every_gop = 1;
		printf("Br: %ld\n",param->bitrate);
		break;
	}

    switch( param->mpeg )
    {
    case 1 :
        if( param->min_GOP_size == -1 )
            param->min_GOP_size = 12;
        if( param->max_GOP_size == -1 )
            param->max_GOP_size = 12;
        break;
    case 2:
        if( param->min_GOP_size == -1 )
            param->min_GOP_size = 9;
        if( param->max_GOP_size == -1 )
            param->max_GOP_size = (param->norm == 'n' ? 18 : 15);
        break;
    }
	if( param->svcd_scan_data == -1 )
		param->svcd_scan_data = 0;
}
/********************************************************/
int check_param_constraints(mpeg2parm *param)
{
	int nerr = 0;
	if( param->_32_pulldown )
	{
		if( param->mpeg == 1 )
			mjpeg_error_exit1( "MPEG-1 cannot encode 3:2 pulldown (for transcoding to VCD set 24fps)!" );

		if( param->frame_rate != 4 && param->frame_rate != 5  )
		{
			if( param->frame_rate == 1 || param->frame_rate == 2 )
			{
				param->frame_rate += 3;
				mjpeg_info("3:2 movie pulldown with frame rate set to decode rate not display rate");
				mjpeg_info("3:2 Setting frame rate code to display rate = %d (%2.3f fps)", 
						   param->frame_rate,
						   Y4M_RATIO_DBL(mpeg_framerate(param->frame_rate)));

			}
			else
			{
				mjpeg_error( "3:2 movie pulldown not sensible for %2.3f fps dispay rate",
							Y4M_RATIO_DBL(mpeg_framerate(param->frame_rate)));
				++nerr;
			}
		}
		if( param->fieldenc == 2 )
		{
			mjpeg_error( "3:2 pulldown only possible for frame pictures (-I 1 or -I 0)");
			++nerr;
		}
	}
	


	if(  param->aspect_ratio > mpeg_num_aspect_ratios[param->mpeg-1] )
	{
		mjpeg_error("For MPEG-%d aspect ratio code  %d > %d illegal", 
					param->mpeg, param->aspect_ratio, 
					mpeg_num_aspect_ratios[param->mpeg-1]);
		++nerr;
	}
		


	if( param->min_GOP_size > param->max_GOP_size )
	{
		mjpeg_error( "Min GOP size must be <= Max GOP size" );
		++nerr;
	}

	if( param->preserve_B && 
		( param->min_GOP_size % param->Bgrp_size != 0 ||
		  param->max_GOP_size % param->Bgrp_size != 0 )
		)
	{
		mjpeg_error("Preserving I/P frame spacing is impossible if min and max GOP sizes are" );
		mjpeg_error_exit1("Not both divisible by %d", param->Bgrp_size );
	}

	switch( param->format )
	{
	case MPEG_FORMAT_SVCD_STILL :
	case MPEG_FORMAT_SVCD_NSR :
	case MPEG_FORMAT_SVCD : 
		if( param->aspect_ratio != 2 && param->aspect_ratio != 3 )
			mjpeg_error_exit1("SVCD only supports 4:3 and 16:9 aspect ratios");
		if( param->svcd_scan_data )
		{
			mjpeg_warn( "Generating dummy SVCD scan-data offsets to be filled in by \"vcdimager\"");
			mjpeg_warn( "If you're not using vcdimager you may wish to turn this off using -d");
		}
		break;
	}
	return nerr;
}
/**********************************************/
/*********************
 *
 * Mark the border so that blocks in the frame are unlikely
 * to match blocks outside the frame.  This is useful for
 * motion estimation routines that, for speed, are a little
 * sloppy about some of the candidates they consider.
 *
 ********************/

void border_mark( uint8_t *frame,
                         int w1, int h1, int w2, int h2)
{
  int i, j;
  uint8_t *fp;
  uint8_t mask = 0xff;
  /* horizontal pixel replication (right border) */
 
  for (j=0; j<h1; j++)
  {
    fp = frame + j*w2;
    for (i=w1; i<w2; i++)
    {
      fp[i] = mask;
      mask ^= 0xff;
    }
  }
 
  /* vertical pixel replication (bottom border) */

  for (j=h1; j<h2; j++)
  {
    fp = frame + j*w2;
    for (i=0; i<w2; i++)
    {
        fp[i] = mask;
        mask ^= 0xff;
    }
  }
}


/**************************************/
int f_code( int max_radius )
{
	int c=5;
	if( max_radius < 64) c = 4;
	if( max_radius < 32) c = 3;
	if( max_radius < 16) c = 2;
	if( max_radius < 8) c = 1;
	return c;
}

void init_mpeg_parms(mpeg2parm *param,Mpeg2Settings *opt)
{
	int i;
    const char *msg;

	inputtype = 0;  /* doesnt matter */
	istrm_nframes = 999999999; /* determined by EOF of stdin */

	ctl->N_min = param->min_GOP_size;      /* I frame distance */
	ctl->N_max = param->max_GOP_size;
    ctl->closed_GOPs = param->closed_GOPs;
	mjpeg_info( "GOP SIZE RANGE %d TO %d %s", 
                ctl->N_min, ctl->N_max,
                ctl->closed_GOPs ? "(all GOPs closed)" : "" 
                );
	ctl->M = param->Bgrp_size;             /* I or P frame distance */
	ctl->M_min = param->preserve_B ? ctl->M : 1;
	if( ctl->M >= ctl->N_min )
		ctl->M = ctl->N_min-1;
	opt->mpeg1           = (param->mpeg == 1);
	opt->fieldpic        = (param->fieldenc == 2);

    // SVCD and probably DVD? mandate progressive_sequence = 0 
    switch( param->format )
    {
    case MPEG_FORMAT_SVCD :
    case MPEG_FORMAT_SVCD_NSR :
    case MPEG_FORMAT_SVCD_STILL :
    case MPEG_FORMAT_DVD :
    case MPEG_FORMAT_DVD_NAV :
        opt->prog_seq = 0;
        break;
    default :
        opt->prog_seq        = (param->mpeg == 1 || param->fieldenc == 0);
        break;
    }
	opt->pulldown_32     = param->_32_pulldown;

	opt->aspectratio     = param->aspect_ratio;
	opt->frame_rate_code = param->frame_rate;
	opt->dctsatlim		= opt->mpeg1 ? 255 : 2047;

	/* If we're using a non standard (VCD?) profile bit-rate adjust	the vbv
		buffer accordingly... */

	if( param->bitrate == 0 )
	{
		mjpeg_error_exit1( "Generic format - must specify bit-rate!" );
	}

	opt->still_size = 0;
	if( MPEG_STILLS_FORMAT(param->format) )
	{
		opt->vbv_buffer_code = param->vbv_buffer_still_size / 2048;
		opt->vbv_buffer_still_size = param->pad_stills_to_vbv_buffer_size;
		opt->bit_rate = param->bitrate;
		opt->still_size = param->still_size;
	}
	else if( param->mpeg == 1 )
	{
		/* Scale VBV relative to VCD  */
		opt->bit_rate = MAX(10000, param->bitrate);
		opt->vbv_buffer_code = (20 * param->bitrate  / 1151929);
	}
	else
	{
		opt->bit_rate = MAX(10000, param->bitrate);
		opt->vbv_buffer_code = MIN(112,param->video_buffer_size / 2);
	}
	opt->vbv_buffer_size = opt->vbv_buffer_code*16384;

	if( param->quant )
	{
		ctl->quant_floor = inv_scale_quant( param->mpeg == 1 ? 0 : 1, 
                                           param->quant );
	}
	else
	{
		ctl->quant_floor = 0.0;		/* Larger than max quantisation */
	}

	ctl->video_buffer_size = param->video_buffer_size * 1024 * 8;
	
	opt->seq_hdr_every_gop = param->seq_hdr_every_gop;
	opt->seq_end_every_gop = param->seq_end_every_gop;
	opt->svcd_scan_data = param->svcd_scan_data;
	opt->ignore_constraints = param->ignore_constraints;
	ctl->seq_length_limit = param->seq_length_limit;
	ctl->nonvid_bit_rate = param->nonvid_bitrate * 1000;
	opt->low_delay       = 0;
	opt->constrparms     = (param->mpeg == 1 && 
						   !MPEG_STILLS_FORMAT(param->format));
	opt->profile         = 4; /* High or Main profile resp. */
	opt->level           = 8;                 /* Main Level      CCIR 601 rates */
	opt->chroma_format   = CHROMA420;
	switch(param->norm)
	{
	case 'p': opt->video_format = 1; break;
	case 'n': opt->video_format = 2; break;
	case 's': opt->video_format = 3; break;
	default:  opt->video_format = 5; break; /* unspec. */
	}
	switch(param->norm)
	{
	case 's':
	case 'p':  /* ITU BT.470  B,G */
		opt->color_primaries = 5;
		opt->transfer_characteristics = 5; /* Gamma = 2.8 (!!) */
		opt->matrix_coefficients = 5; 
        msg = "PAL B/G";
		break;
	case 'n': /* SMPTPE 170M "modern NTSC" */
		opt->color_primaries = 6;
		opt->matrix_coefficients = 6; 
		opt->transfer_characteristics = 6;
        msg = "NTSC";
		break; 
	default:   /* unspec. */
		opt->color_primaries = 2;
		opt->matrix_coefficients = 2; 
		opt->transfer_characteristics = 2;
        msg = "unspecified";
		break;
	}
    mjpeg_info( "Setting colour/gamma parameters to \"%s\"", msg);

	switch( param->format )
	{
	case MPEG_FORMAT_SVCD_STILL :
	case MPEG_FORMAT_SVCD_NSR :
	case MPEG_FORMAT_SVCD :
    case MPEG_FORMAT_DVD :
    case MPEG_FORMAT_DVD_NAV :
        /* It would seem DVD and perhaps SVCD demand a 540 pixel display size
           for 4:3 aspect video. However, many players expect 480 and go weird
           if this isn't set...
        */
        if( param->hack_svcd_hds_bug )
        {
            opt->display_horizontal_size  = opt->horizontal_size;
            opt->display_vertical_size    = opt->vertical_size;
        }
        else
        {
            opt->display_horizontal_size  = opt->aspectratio == 2 ? 540 : 720;
            opt->display_vertical_size    = opt->vertical_size;
        }
		break;
	default:
		opt->display_horizontal_size  = opt->horizontal_size;
		opt->display_vertical_size    = opt->vertical_size;
		break;
	}

	opt->dc_prec         = param->mpeg2_dc_prec;  /* 9 bits */
    opt->topfirst = 0;
	if( ! opt->prog_seq )
	{
		int fieldorder;
		if( param->force_interlacing != Y4M_UNKNOWN ) 
		{
			mjpeg_info( "Forcing playback video to be: %s",
						mpeg_interlace_code_definition(	param->force_interlacing ) );	
			fieldorder = param->force_interlacing;
		}
		else
			fieldorder = param->input_interlacing;

		opt->topfirst = (fieldorder == Y4M_ILACE_TOP_FIRST || 
						fieldorder ==Y4M_ILACE_NONE );
	}
	else
		opt->topfirst = 0;

    // Restrict to frame motion estimation and DCT modes only when MPEG1
    // or when progressive content is specified for MPEG2.
    // Note that for some profiles although we have progressive sequence 
    // header bit = 0 we still only encode with frame modes (for speed).
	opt->frame_pred_dct_tab[0] 
		= opt->frame_pred_dct_tab[1] 
		= opt->frame_pred_dct_tab[2] 
        = (param->mpeg == 1 || param->fieldenc == 0) ? 1 : 0;

    mjpeg_info( "Progressive format frames = %d", 	opt->frame_pred_dct_tab[0] );
	opt->qscale_tab[0] 
		= opt->qscale_tab[1] 
		= opt->qscale_tab[2] 
		= param->mpeg == 1 ? 0 : 1;

	opt->intravlc_tab[0] 
		= opt->intravlc_tab[1]
		= opt->intravlc_tab[2] 
		= param->mpeg == 1 ? 0 : 1;

	opt->altscan_tab[2]  
		= opt->altscan_tab[1]  
		= opt->altscan_tab[0]  
		= (param->mpeg == 1 || param->hack_altscan_bug) ? 0 : 1;
	

	/*  A.Stevens 2000: The search radius *has* to be a multiple of 8
		for the new fast motion compensation search to work correctly.
		We simply round it up if needs be.  */

	if(param->searchrad*ctl->M>127)
	{
		param->searchrad = 127/ctl->M;
		mjpeg_warn("Search radius reduced to %d",param->searchrad);
	}
	
	{ 
		int radius_x = param->searchrad;
		int radius_y = param->searchrad*opt->vertical_size/opt->horizontal_size;

		/* TODO: These f-codes should really be adjusted for each
		   picture type... */

		//opt->motion_data = (struct motion_data *)malloc(ctl->M*sizeof(struct motion_data));
		opt->motion_data =new motion_data[ctl->M];
		if (!opt->motion_data)
			mjpeg_error_exit1("malloc failed");

		for (i=0; i<ctl->M; i++)
		{
			if(i==0)
			{
				opt->motion_data[i].sxf = round_search_radius(radius_x*ctl->M);
				opt->motion_data[i].forw_hor_f_code  = f_code(opt->motion_data[i].sxf);
				opt->motion_data[i].syf = round_search_radius(radius_y*ctl->M);
				opt->motion_data[i].forw_vert_f_code  = f_code(opt->motion_data[i].syf);
			}
			else
			{
				opt->motion_data[i].sxf = round_search_radius(radius_x*i);
				opt->motion_data[i].forw_hor_f_code  = f_code(opt->motion_data[i].sxf);
				opt->motion_data[i].syf = round_search_radius(radius_y*i);
				opt->motion_data[i].forw_vert_f_code  = f_code(opt->motion_data[i].syf);
				opt->motion_data[i].sxb = round_search_radius(radius_x*(ctl->M-i));
				opt->motion_data[i].back_hor_f_code  = f_code(opt->motion_data[i].sxb);
				opt->motion_data[i].syb = round_search_radius(radius_y*(ctl->M-i));
				opt->motion_data[i].back_vert_f_code  = f_code(opt->motion_data[i].syb);
			}

			/* MPEG-1 demands f-codes for vertical and horizontal axes are
			   identical!!!!
			*/
			if( opt->mpeg1 )
			{
				opt->motion_data[i].syf = opt->motion_data[i].sxf;
				opt->motion_data[i].syb  = opt->motion_data[i].sxb;
				opt->motion_data[i].forw_vert_f_code  = 
					opt->motion_data[i].forw_hor_f_code;
				opt->motion_data[i].back_vert_f_code  = 
					opt->motion_data[i].back_hor_f_code;
				
			}
		}
		
	}
	


	/* make sure MPEG specific parameters are valid */
	range_checks();

	/* Set the frame decode rate and frame display rates.
	   For 3:2 movie pulldown decode rate is != display rate due to
	   the repeated field that appears every other frame.
	*/
	opt->frame_rate = Y4M_RATIO_DBL(mpeg_framerate(opt->frame_rate_code));
	if( param->_32_pulldown )
	{
		ctl->decode_frame_rate = opt->frame_rate * (2.0 + 2.0) / (3.0 + 2.0);
		mjpeg_info( "3:2 Pulldown selected frame decode rate = %3.3f fps", 
					ctl->decode_frame_rate);
	}
	else
		ctl->decode_frame_rate = opt->frame_rate;

	if ( !opt->mpeg1)
	{
		profile_and_level_checks();
	}
	else
	{
		/* MPEG-1 */
		if (opt->constrparms)
		{
			if (opt->horizontal_size>768
				|| opt->vertical_size>576
				|| ((opt->horizontal_size+15)/16)*((opt->vertical_size+15)/16)>396
				|| ((opt->horizontal_size+15)/16)*((opt->vertical_size+15)/16)*opt->frame_rate>396*25.0
				|| opt->frame_rate>30.0)
			{
				mjpeg_info( "size - setting constrained_parameters_flag = 0");
				opt->constrparms = 0;
			}
		}

		if (opt->constrparms)
		{
			for (i=0; i<ctl->M; i++)
			{
				if (opt->motion_data[i].forw_hor_f_code>4)
				{
					mjpeg_info("Hor. motion search forces constrained_parameters_flag = 0");
					opt->constrparms = 0;
					break;
				}

				if (opt->motion_data[i].forw_vert_f_code>4)
				{
					mjpeg_info("Ver. motion search forces constrained_parameters_flag = 0");
					opt->constrparms = 0;
					break;
				}

				if (i!=0)
				{
					if (opt->motion_data[i].back_hor_f_code>4)
					{
						mjpeg_info("Hor. motion search setting constrained_parameters_flag = 0");
						opt->constrparms = 0;
						break;
					}

					if (opt->motion_data[i].back_vert_f_code>4)
					{
						mjpeg_info("Ver. motion search setting constrained_parameters_flag = 0");
						opt->constrparms = 0;
						break;
					}
				}
			}
		}
	}

	/* relational checks */
	if ( opt->mpeg1 )
	{
		if (!opt->prog_seq)
		{
			mjpeg_warn("opt->mpeg1 specified - setting progressive_sequence = 1");
			opt->prog_seq = 1;
		}

		if (opt->chroma_format!=CHROMA420)
		{
			mjpeg_info("mpeg1 - forcing chroma_format = 1 (4:2:0) - others not supported");
			opt->chroma_format = CHROMA420;
		}

		if (opt->dc_prec!=0)
		{
			mjpeg_info("mpeg1 - setting intra_dc_precision = 0");
			opt->dc_prec = 0;
		}

		for (i=0; i<3; i++)
			if (opt->qscale_tab[i])
			{
				mjpeg_info("mpeg1 - setting qscale_tab[%d] = 0",i);
				opt->qscale_tab[i] = 0;
			}

		for (i=0; i<3; i++)
			if (opt->intravlc_tab[i])
			{
				mjpeg_info("mpeg1 - setting intravlc_tab[%d] = 0",i);
				opt->intravlc_tab[i] = 0;
			}

		for (i=0; i<3; i++)
			if (opt->altscan_tab[i])
			{
				mjpeg_info("mpeg1 - setting altscan_tab[%d] = 0",i);
				opt->altscan_tab[i] = 0;
			}
	}

	if ( !opt->mpeg1 && opt->constrparms)
	{
		mjpeg_info("not mpeg1 - setting constrained_parameters_flag = 0");
		opt->constrparms = 0;
	}


	if( (!opt->prog_seq || opt->fieldpic != 0 ) &&
		( (opt->vertical_size+15) / 16)%2 != 0 )
	{
		mjpeg_warn( "Frame height won't split into two equal field pictures...");
		mjpeg_warn( "forcing encoding as progressive video\n");
		mjpeg_warn( "prog_seq : %d, fieldpic : %d\n",opt->prog_seq,opt->fieldpic);
		opt->prog_seq = 1;
		opt->fieldpic = 0;
	}


	if (opt->prog_seq && opt->fieldpic != 0)
	{
		mjpeg_info("prog sequence - forcing progressive frame encoding");
		opt->fieldpic = 0;
	}


	if (opt->prog_seq && opt->topfirst)
	{
		mjpeg_info("prog sequence setting top_field_first = 0");
		opt->topfirst = 0;
	}

	/* search windows */
	for (i=0; i<ctl->M; i++)
	{
		if (opt->motion_data[i].sxf > (4U<<opt->motion_data[i].forw_hor_f_code)-1)
		{
			mjpeg_info(
				"reducing forward horizontal search width to %d",
						(4<<opt->motion_data[i].forw_hor_f_code)-1);
			opt->motion_data[i].sxf = (4U<<opt->motion_data[i].forw_hor_f_code)-1;
		}

		if (opt->motion_data[i].syf > (4U<<opt->motion_data[i].forw_vert_f_code)-1)
		{
			mjpeg_info(
				"reducing forward vertical search width to %d",
				(4<<opt->motion_data[i].forw_vert_f_code)-1);
			opt->motion_data[i].syf = (4U<<opt->motion_data[i].forw_vert_f_code)-1;
		}

		if (i!=0)
		{
			if (opt->motion_data[i].sxb > (4U<<opt->motion_data[i].back_hor_f_code)-1)
			{
				mjpeg_info(
					"reducing backward horizontal search width to %d",
					(4<<opt->motion_data[i].back_hor_f_code)-1);
				opt->motion_data[i].sxb = (4U<<opt->motion_data[i].back_hor_f_code)-1;
			}

			if (opt->motion_data[i].syb > (4U<<opt->motion_data[i].back_vert_f_code)-1)
			{
				mjpeg_info(
					"reducing backward vertical search width to %d",
					(4<<opt->motion_data[i].back_vert_f_code)-1);
				opt->motion_data[i].syb = (4U<<opt->motion_data[i].back_vert_f_code)-1;
			}
		}
	}

}

/*
  If the use has selected suppression of hf noise via quantisation
  then we boost quantisation of hf components EXPERIMENTAL: currently
  a linear ramp from 0 at 4pel to param->hf_q_boost increased
  quantisation...

*/

int quant_hfnoise_filt(int orgquant, int qmat_pos,mpeg2parm *param )
    {
    int orgdist = intmax(qmat_pos % 8, qmat_pos/8);
    double qboost;

    /* Maximum param->hf_q_boost quantisation boost for HF components.. */
    qboost = 1.0 + ((param->hf_q_boost * orgdist) / 8);
    return static_cast<int>(orgquant * qboost);
    }

void init_quantmat(mpeg2parm *param,Mpeg2Settings *opt)
    {
    int i, v, q;
    const char *msg = NULL;
    const uint16_t *qmat, *niqmat;
    opt->load_iquant = 0;
    opt->load_niquant = 0;

    /* bufalloc to ensure alignment */
    opt->intra_q = (uint16_t*)bufalloc(64*sizeof(uint16_t));
    opt->inter_q = (uint16_t*)bufalloc(64*sizeof(uint16_t));
    i_intra_q = (uint16_t*)bufalloc(64*sizeof(uint16_t));
    i_inter_q = (uint16_t*)bufalloc(64*sizeof(uint16_t));

    switch  (param->hf_quant)
        {
        case  0:    /* No -N, -H or -K used.  Default matrices */
            msg = "Using default unmodified quantization matrices";
            qmat = default_intra_quantizer_matrix;
            niqmat = default_nonintra_quantizer_matrix;
            break;
        case  1:    /* "-N value" used but not -K or -H */
            msg = "Using -N modified default quantization matrices";
            qmat = default_intra_quantizer_matrix;
            niqmat = default_nonintra_quantizer_matrix;
            opt->load_iquant = 1;
            opt->load_niquant = 1;
            break;
        case  2:    /* -H used OR -H followed by "-N value" */
            msg = "Setting hi-res intra Quantisation matrix";
            qmat = hires_intra_quantizer_matrix;
            niqmat = hires_nonintra_quantizer_matrix;
            opt->load_iquant = 1;
            if  (param->hf_q_boost)
                opt->load_niquant = 1;   /* Custom matrix if -N used */
            break;
        case  3:
            msg = "KVCD Notch Quantization Matrix";
            qmat = kvcd_intra_quantizer_matrix;
            niqmat = kvcd_nonintra_quantizer_matrix;
            opt->load_iquant = 1;
            opt->load_niquant = 1;
            break;
        case  4:
            msg = "TMPGEnc Quantization matrix";
            qmat = tmpgenc_intra_quantizer_matrix;
            niqmat = tmpgenc_nonintra_quantizer_matrix;
            opt->load_iquant = 1;
            opt->load_niquant = 1;
            break;
        case  5:            /* -K file=qmatrixfilename */
            msg = "Loading custom matrices from user specified file";
            opt->load_iquant = 1;
            opt->load_niquant = 1;
            qmat = custom_intra_quantizer_matrix;
            niqmat = custom_nonintra_quantizer_matrix;
            break;
        default:
            mjpeg_error_exit1("Help!  Unknown param->hf_quant value %d",
		param->hf_quant);
            /* NOTREACHED */
        }

  
        printf("%s\n",msg);
    
    for (i = 0; i < 64; i++)
        {
        v = quant_hfnoise_filt(qmat[i], i,param);
        if  (v < 1 || v > 255)
            mjpeg_error_exit1("bad intra value after -N adjust");
        opt->intra_q[i] = v;

        v = quant_hfnoise_filt(niqmat[i], i,param);
        if  (v < 1 || v > 255)
            mjpeg_error_exit1("bad nonintra value after -N adjust");
        opt->inter_q[i] = v;
        }

    /* TODO: Inv Quant matrix initialisation should check if the
     * fraction fits in 16 bits! */
  
    for (i = 0; i < 64; i++)
        {
        i_intra_q[i] = (int)(((double)IQUANT_SCALE) / ((double)opt->intra_q[i]));
        i_inter_q[i] = (int)(((double)IQUANT_SCALE) / ((double)opt->inter_q[i]));
        }
    
    for (q = 1; q <= 112; ++q)
        {
        for (i = 0; i < 64; i++)
            {
            intra_q_tbl[q][i] = opt->intra_q[i] * q;
            inter_q_tbl[q][i] = opt->inter_q[i] * q;
            intra_q_tblf[q][i] = (float)intra_q_tbl[q][i];
            inter_q_tblf[q][i] = (float)inter_q_tbl[q][i];
            i_intra_q_tblf[q][i] = 1.0f / ( intra_q_tblf[q][i] * 0.98);
            i_intra_q_tbl[q][i] = (IQUANT_SCALE/intra_q_tbl[q][i]);
            i_inter_q_tblf[q][i] =  1.0f / (inter_q_tblf[q][i] * 0.98);
            i_inter_q_tbl[q][i] = (IQUANT_SCALE/inter_q_tbl[q][i]);
            }
        }
    }
/**********************************/
int infer_mpeg1_aspect_code(  mpeg2parm *param )
{
	switch( param->aspect_ratio )
	{
	case 1 :					/* 1:1 */
		return 1;
	case 2 :					/* 4:3 */
		if( param->norm == 'p' || param->norm == 's' )
			return 8;
	    else if( param->norm == 'n' )
			return 12;
		else 
			return 0;
	case 3 :					/* 16:9 */
		if( param->norm == 'p' || param->norm == 's' )
			return 3;
	    else if( param->norm == 'n' )
			return 6;
		else
			return 0;
	default :
		return 0;				/* Unknown */
	}
}
/**********************************/

void DisplayFrameRates(void)
{
 	unsigned int i;
	printf("Frame-rate codes:\n");
	for( i = 0; i < mpeg_num_framerates; ++i )
	{
		printf( "%2d - %s\n", i, mpeg_framerate_code_definition(i));
	}
	exit(0);
}
/**********************************/

void DisplayAspectRatios(void)
{
 	unsigned int i;
	printf("\nDisplay aspect ratio codes:\n");
	for( i = 1; i <= mpeg_num_aspect_ratios[1]; ++i )
	{
		printf( "%2d - %s\n", i, mpeg_aspect_code_definition(2,i));
	}
	exit(0);
}

/**********************************/

void Usage(void)
{
	fprintf(stderr,
"--verbose|-v num\n" 
"    Level of verbosity. 0 = quiet, 1 = normal 2 = verbose/debug\n"
"--format|-f fmt\n"
"    Set pre-defined mux format fmt.\n"
"    [0 = Generic MPEG1, 1 = standard VCD, 2 = VCD,\n"
"     3 = Generic MPEG2, 4 = standard SVCD, 5 = user SVCD,\n"
"     6 = VCD Stills sequences, 7 = SVCD Stills sequences, 8 = DVD]\n"
"--aspect|-a num\n"
"    Set displayed image aspect ratio image (default: 2 = 4:3)\n"
"    [1 = 1:1, 2 = 4:3, 3 = 16:9, 4 = 2.21:1]\n"
"--frame-rate|-F num\n"
"    Set playback frame rate of encoded video\n"
"    (default: frame rate of input stream)\n"
"    0 = Display frame rate code table\n"
"--video-bitrate|-b num\n"
"    Set Bitrate of compressed video in KBit/sec\n"
"    (default: 1152 for VCD, 2500 for SVCD, 7500 for DVD)\n"
"--nonvideo-birate|-B num\n"
"    Non-video data bitrate to assume for sequence splitting\n"
"    calculations (see also --sequence-length).\n"
"--quantisation|-q num\n"
"    Image data quantisation factor [1..31] (1 is best quality, no default)\n"
"    When quantisation is set variable bit-rate encoding is activated and\n"
"    the --bitrate value sets an *upper-bound* video data-rate\n"
"--output|-o pathname\n"
"    pathname of output file or fifo (REQUIRED!!!)\n"
"--vcd-still-size|-T size\n"
"    Size in KB of VCD stills\n"
"--interlace-mode|-I num\n"
"    Sets MPEG 2 motion estimation and encoding modes:\n"
"    0 = Progressive (non-interlaced)(Movies)\n"
"    1 = Interlaced source material (video)\n"
"--motion-search-radius|-r num\n"
"    Motion compensation search radius [0..32] (default 16)\n"
"--reduction-4x4|-4 num\n"
"    Reduction factor for 4x4 subsampled candidate motion estimates\n"
"    [1..4] [1 = max quality, 4 = max. speed] (default: 2)\n"
"--reduction-2x2|-2 num\n"
"    Reduction factor for 2x2 subsampled candidate motion estimates\n"
"    [1..4] [1 = max quality, 4 = max. speed] (default: 3)\n"
"--min-gop-size|-g num\n"
"    Minimum size Group-of-Pictures (default depends on selected format)\n"
"--max-gop-size|-G num\n"
"    Maximum size Group-of-Pictures (default depends on selected format)\n"
"    If min-gop is less than max-gop, mpeg2enc attempts to place GOP\n"
"    boundaries to coincide with scene changes\n"
"--closed-gop|-c\n"
"    All Group-of-Pictures are closed.  Useful for authoring multi-angle DVD\n"
"--force-b-b-p|-P\n"
"    Preserve two B frames between I/P frames when placing GOP boundaries\n"
"--quantisation-reduction|-Q num\n"
"    Max. quantisation reduction for highly active blocks\n"
"    [0.0 .. 5] (default: 0.0)\n"
"--video-buffer|-V num\n"
"    Target decoders video buffer size in KB (default 46)\n"
"--video-norm|-n n|p|s\n"
"    Tag output to suit playback in specified video norm\n"
"    (n = NTSC, p = PAL, s = SECAM) (default: PAL)\n"
"--sequence-length|-S num\n"
"    Place a sequence boundary in the video stream so they occur every\n"
"    num Mbytes once the video is multiplexed with audio etc.\n"
"    N.b. --non-video-bitrate is used to the bitrate of the other\n"
"    data that will be multiplexed with this video stream\n"
"--3-2-pulldown|-p\n"
"    Generate header flags for 3-2 pull down of 24fps movie material\n"
"--intra_dc_prec|-D [8..10]\n"
"    Set number of bits precision for DC (base colour) of blocks in MPEG-2\n"
"--reduce-hf|-N num\n"
"    [0.0..2.0] Reduce hf resolution (increase quantization) by num (default: 0.0)\n"
"--keep-hf|-H\n"
"    Maximise high-frequency resolution - useful for high quality sources\n"
"    and/or high bit-rates)\n"
"--sequence-header-every-gop|-s\n"
"    Include a sequence header every GOP if the selected format doesn't\n"
"    do so by default.\n"
"--no-dummy-svcd-SOF|-d\n"
"    Do not generate dummy SVCD scan-data for the ISO CD image\n"
"    generator \"vcdimager\" to fill in.\n"
"--playback-field-order|-z b|t\n"
"    Force setting of playback field order to bottom or top first\n"
"--multi-thread|-M num\n"
"    Activate multi-threading to optimise throughput on a system with num CPU's\n"
"    [0..32], 0=no multithreading, (default: 1)\n"
"--correct-svcd-hds|-C\n"
"    Force SVCD horizontal_display_size to be 480 - standards say 540 or 720\n"
"    But many DVD/SVCD players screw up with these values.\n"
"--no-altscan-mpeg2\n"
"    Force MPEG2 *not* to use alternate block scanning.  This may allow some\n"
"    buggy players to play SVCD streams\n"
"--no-constraints\n"
"    Deactivate the constraints for maximum video resolution and sample rate.\n"
"    Could expose bugs in the software at very high resolutions!\n"
"--custom-quant-matrices|-K kvcd|tmpgenc|default|hi-res|file=inputfile|help\n"
"    Request custom or userspecified (from a file) quantization matrices\n"
"--help|-?\n"
"    Print this lot out!\n"
	);
	exit(0);
}

/*---------------------------------*/
void init_encoder(mpeg2parm *param,Mpeg2Settings *opt)
{
	int i;
    unsigned int n;
	static int block_count_tab[3] = {6,8,12};
    int enc_chrom_width, enc_chrom_height;
	initbits(); 

	/* Tune threading and motion compensation for specified number of CPU's 
	   and specified speed parameters.
	 */

	ctl->act_boost = param->act_boost >= 0.0 
        ? (param->act_boost+1.0)
        : (param->act_boost-1.0);
    ctl->boost_var_ceil = param->boost_var_ceil;
	switch( param->num_cpus )
	{

	case 0 : /* Special case for debugging... turns of multi-threading */
		ctl->max_encoding_frames = 1;
		ctl->refine_from_rec = true;
		ctl->parallel_read = false;
		break;

	case 1 :
		ctl->max_encoding_frames = 1;
		ctl->refine_from_rec = true;
		ctl->parallel_read = true;
		break;
	case 2:
		ctl->max_encoding_frames = 2;
		ctl->refine_from_rec = true;
		ctl->parallel_read = true;
		break;
	default :
		ctl->max_encoding_frames = param->num_cpus > MAX_WORKER_THREADS-1 ?
			                  MAX_WORKER_THREADS-1 :
			                  param->num_cpus;
		ctl->refine_from_rec = false;
		ctl->parallel_read = true;
		break;
	}


	ctl->_44_red		= param->_44_red;
	ctl->_22_red		= param->_22_red;
	
	/* round picture dimensions to nearest multiple of 16 or 32 */
	mb_width = (opt->horizontal_size+15)/16;
	mb_height = opt->prog_seq ? (opt->vertical_size+15)/16 : 2*((opt->vertical_size+31)/32);
	mb_height2 = opt->fieldpic ? mb_height>>1 : mb_height; /* for field pictures */
	opt->enc_width = 16*mb_width;
	opt->enc_height = 16*mb_height;

#ifdef HAVE_ALTIVEC
	/* Pad opt->phy_width to 64 so that the rowstride of 4*4
	 * sub-sampled data will be a multiple of 16 (ideal for AltiVec)
	 * and the rowstride of 2*2 sub-sampled data will be a multiple
	 * of 32. Height does not affect rowstride, no padding needed.
	 */
	opt->phy_width = (opt->enc_width + 63) & (~63);
#else
	opt->phy_width = opt->enc_width;
#endif
	opt->phy_height = opt->enc_height;

	/* Calculate the sizes and offsets in to luminance and chrominance
	   buffers.  A.Stevens 2000 for luminance data we allow space for
	   fast motion estimation data.  This is actually 2*2 pixel
	   sub-sampled uint8_t followed by 4*4 sub-sampled.  We add an
	   extra row to act as a margin to allow us to neglect / postpone
	   edge condition checking in time-critical loops...  */

	opt->phy_chrom_width = (opt->chroma_format==CHROMA444) 
        ? opt->phy_width 
        : opt->phy_width>>1;
	opt->phy_chrom_height = (opt->chroma_format!=CHROMA420) 
        ? opt->phy_height 
        : opt->phy_height>>1;
	enc_chrom_width = (opt->chroma_format==CHROMA444) 
        ? opt->enc_width 
        : opt->enc_width>>1;
	enc_chrom_height = (opt->chroma_format!=CHROMA420) 
        ? opt->enc_height 
        : opt->enc_height>>1;

	opt->phy_height2 = opt->fieldpic ? opt->phy_height>>1 : opt->phy_height;
	opt->enc_height2 = opt->fieldpic ? opt->enc_height>>1 : opt->enc_height;
	opt->phy_width2 = opt->fieldpic ? opt->phy_width<<1 : opt->phy_width;
	opt->phy_chrom_width2 = opt->fieldpic 
        ? opt->phy_chrom_width<<1 
        : opt->phy_chrom_width;
 
	block_count = block_count_tab[opt->chroma_format-1];
	lum_buffer_size = (opt->phy_width*opt->phy_height) +
					 sizeof(uint8_t) *(opt->phy_width/2)*(opt->phy_height/2) +
					 sizeof(uint8_t) *(opt->phy_width/4)*(opt->phy_height/4+1);
	chrom_buffer_size = opt->phy_chrom_width*opt->phy_chrom_height;


	fsubsample_offset = (opt->phy_width)*(opt->phy_height) * sizeof(uint8_t);
	qsubsample_offset =  fsubsample_offset 
        + (opt->phy_width/2)*(opt->phy_height/2)*sizeof(uint8_t);

	mb_per_pict = mb_width*mb_height2;


	/* Allocate the frame data buffers */
    frame_buffer_size = 2*param->max_GOP_size+param->Bgrp_size+READ_CHUNK_SIZE+1;
    mjpeg_info( "Buffering %d frames", frame_buffer_size );
	frame_buffers = (uint8_t ***) bufalloc(frame_buffer_size*sizeof(uint8_t**));

	for(n=0;n<frame_buffer_size;n++)
	{
         frame_buffers[n] = (uint8_t **) bufalloc(3*sizeof(uint8_t*));
		 for (i=0; i<3; i++)
		 {
			 frame_buffers[n][i] =
                 static_cast<uint8_t *>( bufalloc( (i==0)
                                                   ? lum_buffer_size
                                                   : chrom_buffer_size )
                     );
		 }

         border_mark(frame_buffers[n][0],
                     opt->enc_width,opt->enc_height,
                     opt->phy_width,opt->phy_height);
         border_mark(frame_buffers[n][1],
                     enc_chrom_width, enc_chrom_height,
                     opt->phy_chrom_width,opt->phy_chrom_height);
         border_mark(frame_buffers[n][2],
                     enc_chrom_width, enc_chrom_height,
                     opt->phy_chrom_width,opt->phy_chrom_height);

	}
#ifdef OUTPUT_STAT
	/* open statistics output file */
	if (!(statfile = fopen(statname,"w")))
	{
		mjpeg_error_exit1( "Couldn't create statistics output file %s",
						   statname);
	}
#endif
}
/*---------------------------------*/
int infer_default_params(mpeg2parm *param,Mpeg2Settings *opt)
{
	int nerr = 0;


	/* Infer norm, aspect ratios and frame_rate if not specified */
#if 0	
	if( param->frame_rate == 0 )
	{
		if(strm_frame_rate_code<1 || strm_frame_rate_code>8)
		{
			mjpeg_error("Input stream with unknown frame-rate and no frame-rate specified with -a!");
			++nerr;
		}
		else
			param->frame_rate = strm_frame_rate_code;
	}
	if( param->norm == 0 && (strm_frame_rate_code==3 || strm_frame_rate_code == 2) )
	{
		mjpeg_info("Assuming norm PAL");
		param->norm = 'p';
	}
	if( param->norm == 0 && (strm_frame_rate_code==4 || strm_frame_rate_code == 1) )
	{
		mjpeg_info("Assuming norm NTSC");
		param->norm = 'n';
	}





	if( param->frame_rate != 0 )
	{
		if( strm_frame_rate_code != param->frame_rate && 
			strm_frame_rate_code > 0 && 
			strm_frame_rate_code < mpeg_num_framerates )
		{
			mjpeg_warn( "Specified display frame-rate %3.2f will over-ride", 
						Y4M_RATIO_DBL(mpeg_framerate(param->frame_rate)));
			mjpeg_warn( "(different!) frame-rate %3.2f of the input stream",
						Y4M_RATIO_DBL(mpeg_framerate(strm_frame_rate_code)));
		}
		opt->frame_rate_code = param->frame_rate;
	}
	if( param->aspect_ratio == 0 )
	{
		param->aspect_ratio = strm_aspect_ratio;
	}
#endif

	if( param->aspect_ratio == 0 )
	{
		mjpeg_warn( "No aspect ratio specifed and no guess possible: assuming 4:3 display aspect!");
		param->aspect_ratio = 2;
	}

	/* Convert to MPEG1 coding if we're generating MPEG1 */
	if( param->mpeg == 1 )
	{
		param->aspect_ratio = infer_mpeg1_aspect_code( param );
	}

	return nerr;
}
/*---------------------------------*/

int parse_custom_matrixfile(char *fname, int dbug)
    {
    FILE  *fp;
    uint16_t  q[128];
    int  i, j, row;
    char line[80];

    fp = fopen(fname, "r");
    if  (!fp)
        {
        mjpeg_error("can not open custom matrix file '%s'", fname);
        return(-1);
        }

    row = i = 0;
    while   (fgets(line, sizeof(line), fp))
            {
            row++;
            /* Empty lines (\n only) and comments are ignored */
            if  ((strlen(line) == 1) || line[0] == '#')
                continue;
            j = sscanf(line, "%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu\n",
                    &q[i+0], &q[i+1], &q[i+2], &q[i+3],
		    &q[i+4], &q[i+5], &q[i+6], &q[i+7]);
            if  (j != 8)
                {
                mjpeg_error("line %d ('%s') of '%s' malformed", row, line, fname);
                break;
                }
            for (j = 0; j < 8; j++)
                {
                if  (q[i + j] < 1 || q[i + j] > 255)
                    {
                    mjpeg_error("entry %d (%u) in line %d from '%s' invalid",
                        j, q[i + j], row, fname);
                    i = -1;
                    break;
                    }
                }
            i += 8;
            }

        fclose(fp);

        if  (i != 128)
            {
            mjpeg_error("file '%s' did NOT have 128 values - ignoring custom matrix file", fname);
            return(-1);
            }

        for (j = 0; j < 64; j++)
            {
            custom_intra_quantizer_matrix[j] = q[j];
            custom_nonintra_quantizer_matrix[j] = q[j + 64];
            }

        if  (dbug)
            {
            mjpeg_info("INTRA and NONINTRA tables from '%s'",fname);
            for (j = 0; j < 128; j += 8)
                {
                mjpeg_info("%u %u %u %u %u %u %u %u", 
                    q[j + 0], q[j + 1], q[j + 2], q[j + 3], 
                    q[j + 4], q[j + 5], q[j + 6], q[j + 7]);
                }
            }
        return(0);
        }

void parse_custom_option(char *arg,mpeg2parm *param)
    {

    if	(strcasecmp(arg, "kvcd") == 0)
    	param->hf_quant = 3;
    else if (strcasecmp(arg, "hi-res") == 0)
    	param->hf_quant = 2;
    else if (strcasecmp(arg, "default") == 0)
    	{
    	param->hf_quant = 0;
    	param->hf_q_boost = 0;
    	}
    else if (strcasecmp(arg, "tmpgenc") == 0)
    	param->hf_quant = 4;
    else if (strncasecmp(arg, "file=", 5) == 0)
    	{
    	if  (parse_custom_matrixfile(arg + 5, arg[0] == 'F' ? 1 : 0) == 0)
    	    param->hf_quant = 5;
    	}
    else if (strcasecmp(arg, "help") == 0)
    	{
    	fprintf(stderr, "Quantization matrix names:\n\n");
    	fprintf(stderr, "\thelp - this message\n");
    	fprintf(stderr, "\tkvcd - matrices from http://www.kvcd.net\n");
    	fprintf(stderr, "\thi-res - high resolution tables (same as -H)\n");
    	fprintf(stderr, "\tdefault - turn off -N or -H (use standard tables)\n");
    	fprintf(stderr, "\ttmpgenc - TMPGEnc tables (http://www.tmpgenc.com)\n");
    	fprintf(stderr, "\tfile=filename - filename contains custom matrices\n");
    	fprintf(stderr, "\t\t8 comma separated values per line.  8 lines per matrix, INTRA matrix first, then NONINTRA\n");
    	exit(0);
    	}
    else
    	mjpeg_error_exit1("Unknown type '%s' used with -K/--custom-quant", arg);
    return;
    }


