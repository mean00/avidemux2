/* puthdr.c, generation of headers                                          */

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


#include <ADM_default.h>
#include <stdio.h>
#include <math.h>
#include "global.h"


/* private prototypes */
static int frametotc (int frame);

/* generate sequence header (6.2.2.1, 6.3.3)
 *
 * matrix download not implemented
 */
void putseqhdr(void)
{
	int i;

	alignbits();
	putbits(SEQ_START_CODE,32); /* sequence_header_code */
	putbits(opt->horizontal_size,12); /* horizontal_size_value */
	putbits(opt->vertical_size,12); /* vertical_size_value */
	putbits(opt->aspectratio,4); /* aspect_ratio_information */
	putbits(opt->frame_rate_code,4); /* frame_rate_code */

	/* MPEG-1 VBR is FFFF rate code. 
	   MPEG-2 VBR is a matter of mux-ing.  The ceiling bit_rate is always
	   sent 
	*/
	if(opt->mpeg1 && (ctl->quant_floor != 0 || opt->still_size > 0) ) {
		putbits(0xfffff,18);
	} else {
		putbits((int)ceil(opt->bit_rate/400.0),18); /* bit_rate_value */
	}
	putbits(1,1); /* marker_bit */
	putbits(opt->vbv_buffer_code,10); /* vbv_buffer_size_value */
	putbits(opt->constrparms,1); /* constrained_parameters_flag */

	putbits(opt->load_iquant,1); /* load_intra_quantizer_matrix */
	if (opt->load_iquant)
		for (i=0; i<64; i++)  /* matrices are always downloaded in zig-zag order */
			putbits(opt->intra_q[zig_zag_scan[i]],8); /* intra_quantizer_matrix */

	putbits(opt->load_niquant,1); /* load_non_intra_quantizer_matrix */
	if (opt->load_niquant)
		for (i=0; i<64; i++)
			putbits(opt->inter_q[zig_zag_scan[i]],8); /* non_intra_quantizer_matrix */
	if (!opt->mpeg1)
	{
		putseqext();
		putseqdispext();
	}

}

/* generate sequence extension (6.2.2.3, 6.3.5) header (MPEG-2 only) */
void putseqext(void)
{
	alignbits();
	putbits(EXT_START_CODE,32); /* extension_start_code */
	putbits(SEQ_ID,4); /* extension_start_code_identifier */
	putbits((opt->profile<<4)|opt->level,8); /* profile_and_level_indication */
	putbits(opt->prog_seq,1); /* progressive sequence */
	putbits(opt->chroma_format,2); /* chroma_format */
	putbits(opt->horizontal_size>>12,2); /* horizontal_size_extension */
	putbits(opt->vertical_size>>12,2); /* vertical_size_extension */
	putbits(((int)ceil(opt->bit_rate/400.0))>>18,12); /* bit_rate_extension */
	putbits(1,1); /* marker_bit */
	putbits(opt->vbv_buffer_code>>10,8); /* vbv_buffer_size_extension */
	putbits(0,1); /* low_delay  -- currently not implemented */
	putbits(0,2); /* frame_rate_extension_n */
	putbits(0,5); /* frame_rate_extension_d */
}

/* generate sequence display extension (6.2.2.4, 6.3.6)
 *
 */

void putseqdispext(void)
{
	alignbits();
	putbits(EXT_START_CODE,32); /* extension_start_code */
	putbits(DISP_ID,4); /* extension_start_code_identifier */
	putbits(opt->video_format,3); /* video_format */
	putbits(1,1); /* colour_description */
	putbits(opt->color_primaries,8); /* colour_primaries */
	putbits(opt->transfer_characteristics,8); /* transfer_characteristics */
	putbits(opt->matrix_coefficients,8); /* matrix_coefficients */
	putbits(opt->display_horizontal_size,14); /* display_horizontal_size */
	putbits(1,1); /* marker_bit */
	putbits(opt->display_vertical_size,14); /* display_vertical_size */
}

/* output a zero terminated string as user data (6.2.2.2.2, 6.3.4.1)
 *
 * string must not emulate start codes
 */
void putuserdata(const uint8_t *userdata, int len)
{
	int i;
	alignbits();
	putbits(USER_START_CODE,32); /* user_data_start_code */
	for( i =0; i < len; ++i )
		putbits(userdata[i],8);
}

/* generate group of pictures header (6.2.2.6, 6.3.9)
 *
 * uses tc0 (timecode of first frame) and frame0 (number of first frame)
 */
void putgophdr(int frame,int closed_gop )
{
	int tc;

	alignbits();
	putbits(GOP_START_CODE,32); /* group_start_code */
	tc = frametotc(frame);
	putbits(tc,25); /* time_code */
	putbits(closed_gop,1); /* closed_gop */
	putbits(0,1); /* broken_link */
}

/* convert frame number to time_code
 *
 * drop_frame not implemented
 */
static int frametotc(int gop_timecode0_frame)
{
	int frame = gop_timecode0_frame;
	int fps, pict, sec, minute, hour, tc;

	/* Note: no drop_frame_flag support here, so we're simply rounding
	   the frame rate as per 6.3.8 13818-2
	*/
	fps = (int)(opt->frame_rate+0.5);
	pict = frame%fps;
	frame = (frame-pict)/fps;
	sec = frame%60;
	frame = (frame-sec)/60;
	minute = frame%60;
	frame = (frame-minute)/60;
	hour = frame%24;
	tc = (hour<<19) | (minute<<13) | (1<<12) | (sec<<6) | pict;
	return tc;
}

/* generate sequence_end_code (6.2.2) */
void putseqend(void)
{
	alignbits();
	putbits(SEQ_END_CODE,32);
}


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
