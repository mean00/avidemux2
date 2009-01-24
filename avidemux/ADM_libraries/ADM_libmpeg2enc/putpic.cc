/* putpic.c, block and motion vector encoding routines                      */

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
#include <math.h>
#include "global.h"
#include "simd.h"
#include "ratectl.hh"

/* output motion vectors (6.2.5.2, 6.3.16.2)
 *
 * this routine also updates the predictions for motion vectors (PMV)
 */
void Picture::PutMVs( MotionEst &me, bool back )

{
	int hor_f_code;
	int vert_f_code;

	if( back )
	{
		hor_f_code = back_hor_f_code;
		vert_f_code = back_vert_f_code;
	}
	else
	{
		hor_f_code = forw_hor_f_code;
		vert_f_code = forw_vert_f_code;
	}

	if (pict_struct==FRAME_PICTURE)
	{
		if (me.motion_type==MC_FRAME)
		{
			/* frame prediction */
			putmv(me.MV[0][back][0]-PMV[0][back][0],hor_f_code);
			putmv(me.MV[0][back][1]-PMV[0][back][1],vert_f_code);
			PMV[0][back][0]=PMV[1][back][0]=me.MV[0][back][0];
			PMV[0][back][1]=PMV[1][back][1]=me.MV[0][back][1];
		}
		else if (me.motion_type==MC_FIELD)
		{
			/* field prediction */

			putbits(me.field_sel[0][back],1);
			putmv(me.MV[0][back][0]-PMV[0][back][0],hor_f_code);
			putmv((me.MV[0][back][1]>>1)-(PMV[0][back][1]>>1),vert_f_code);
			putbits(me.field_sel[1][back],1);
			putmv(me.MV[1][back][0]-PMV[1][back][0],hor_f_code);
			putmv((me.MV[1][back][1]>>1)-(PMV[1][back][1]>>1),vert_f_code);
			PMV[0][back][0]=me.MV[0][back][0];
			PMV[0][back][1]=me.MV[0][back][1];
			PMV[1][back][0]=me.MV[1][back][0];
			PMV[1][back][1]=me.MV[1][back][1];

		}
		else
		{
			/* dual prime prediction */
			putmv(me.MV[0][back][0]-PMV[0][back][0],hor_f_code);
			putdmv(me.dualprimeMV[0]);
			putmv((me.MV[0][back][1]>>1)-(PMV[0][back][1]>>1),vert_f_code);
			putdmv(me.dualprimeMV[1]);
			PMV[0][back][0]=PMV[1][back][0]=me.MV[0][back][0];
			PMV[0][back][1]=PMV[1][back][1]=me.MV[0][back][1];
		}
	}
	else
	{
		/* field picture */
		if (me.motion_type==MC_FIELD)
		{
			/* field prediction */
			putbits(me.field_sel[0][back],1);
			putmv(me.MV[0][back][0]-PMV[0][back][0],hor_f_code);
			putmv(me.MV[0][back][1]-PMV[0][back][1],vert_f_code);
			PMV[0][back][0]=PMV[1][back][0]=me.MV[0][back][0];
			PMV[0][back][1]=PMV[1][back][1]=me.MV[0][back][1];
		}
		else if (me.motion_type==MC_16X8)
		{
			/* 16x8 prediction */
			putbits(me.field_sel[0][back],1);
			putmv(me.MV[0][back][0]-PMV[0][back][0],hor_f_code);
			putmv(me.MV[0][back][1]-PMV[0][back][1],vert_f_code);
			putbits(me.field_sel[1][back],1);
			putmv(me.MV[1][back][0]-PMV[1][back][0],hor_f_code);
			putmv(me.MV[1][back][1]-PMV[1][back][1],vert_f_code);
			PMV[0][back][0]=me.MV[0][back][0];
			PMV[0][back][1]=me.MV[0][back][1];
			PMV[1][back][0]=me.MV[1][back][0];
			PMV[1][back][1]=me.MV[1][back][1];
		}
		else
		{
			/* dual prime prediction */
			putmv(me.MV[0][back][0]-PMV[0][back][0],hor_f_code);
			putdmv(me.dualprimeMV[0]);
			putmv(me.MV[0][back][1]-PMV[0][back][1],vert_f_code);
			putdmv(me.dualprimeMV[1]);
			PMV[0][back][0]=PMV[1][back][0]=me.MV[0][back][0];
			PMV[0][back][1]=PMV[1][back][1]=me.MV[0][back][1];
		}
	}
}

void MacroBlock::PutBlocks( )
{
    int comp;
    int cc;
    for (comp=0; comp<block_count; comp++)
    {
        /* block loop */
        if( cbp & (1<<(block_count-1-comp)))
        {
            if (final_me.mb_type & MB_INTRA)
            {
                // TODO: 420 Only?
                cc = (comp<4) ? 0 : (comp&1)+1;
                putintrablk(picture, qdctblocks[comp],cc);
            }
            else
            {
                putnonintrablk(picture,qdctblocks[comp]);
            }
        }
    }
}

void MacroBlock::SkippedCoding( bool slice_begin_end )
{
    skipped = false;
    if( slice_begin_end || cbp )
    {
        /* there's no VLC for 'No MC, Not Coded':
         * we have to transmit (0,0) motion vectors
         */
        if (picture->pict_type==P_TYPE && !cbp)
            final_me.mb_type|= MB_FORWARD;
        return;
    }

    MacroBlock *prev_mb = picture->prev_mb;
    /* P picture, no motion vectors -> skip */
    if (picture->pict_type==P_TYPE && !(final_me.mb_type&MB_FORWARD))
    {
        /* reset predictors */
        picture->Reset_DC_DCT_Pred();
        picture->Reset_MV_Pred();
        skipped = true;
        return;
    }
    
    if(picture->pict_type==B_TYPE )
    {
        /* B frame picture with same prediction type
         * (forward/backward/interp.)  and same active vectors
         * as in previous macroblock -> skip
         */

        if (  picture->pict_struct==FRAME_PICTURE
              && final_me.motion_type==MC_FRAME
              && ((prev_mb->final_me.mb_type ^ final_me.mb_type) &(MB_FORWARD|MB_BACKWARD))==0
              && (!(final_me.mb_type&MB_FORWARD) ||
                  (picture->PMV[0][0][0]==final_me.MV[0][0][0] &&
                   picture->PMV[0][0][1]==final_me.MV[0][0][1]))
              && (!(final_me.mb_type&MB_BACKWARD) ||
                  (picture->PMV[0][1][0]==final_me.MV[0][1][0] &&
                   picture->PMV[0][1][1]==final_me.MV[0][1][1])))
        {
            skipped = true;
            return;
        }

        /* B field picture macroblock with same prediction
         * type (forward/backward/interp.) and active
         * vectors as previous macroblock and same
         * vertical field selects as current field -> skio
         */

        if (picture->pict_struct!=FRAME_PICTURE
            && final_me.motion_type==MC_FIELD
            && ((prev_mb->final_me.mb_type^final_me.mb_type)&(MB_FORWARD|MB_BACKWARD))==0
            && (!(final_me.mb_type&MB_FORWARD) ||
                (picture->PMV[0][0][0]==final_me.MV[0][0][0] &&
                 picture->PMV[0][0][1]==final_me.MV[0][0][1] &&
                 final_me.field_sel[0][0]==(picture->pict_struct==BOTTOM_FIELD)))
            && (!(final_me.mb_type&MB_BACKWARD) ||
                (picture->PMV[0][1][0]==final_me.MV[0][1][0] &&
                 picture->PMV[0][1][1]==final_me.MV[0][1][1] &&
                 final_me.field_sel[0][1]==(picture->pict_struct==BOTTOM_FIELD))))
        {
            skipped = true;
            return;
        }
    }

}


/* generate picture header (6.2.3, 6.3.10) */
void Picture::PutHeader()
{
	alignbits();
	putbits(PICTURE_START_CODE,32); /* picture_start_code */
	putbits(temp_ref,10); /* temporal_reference */
	putbits(pict_type,3); /* picture_coding_type */
	putbits(vbv_delay,16); /* vbv_delay */

	if (pict_type==P_TYPE || pict_type==B_TYPE)
	{
		putbits(0,1); /* full_pel_forward_vector */
		if (opt->mpeg1)
			putbits(forw_hor_f_code,3);
		else
			putbits(7,3); /* forward_f_code */
	}

	if (pict_type==B_TYPE)
	{
		putbits(0,1); /* full_pel_backward_vector */
		if (opt->mpeg1)
			putbits(back_hor_f_code,3);
		else
			putbits(7,3); /* backward_f_code */
	}


	putbits(0,1); /* extra_bit_picture */
	if ( !opt->mpeg1 )
	{
		PutCodingExt();
	}

}

/* generate picture coding extension (6.2.3.1, 6.3.11)
 *
 * composite display information (v_axis etc.) not implemented
 */
void Picture::PutCodingExt()
{
	alignbits();
	putbits(EXT_START_CODE,32); /* extension_start_code */
	putbits(CODING_ID,4); /* extension_start_code_identifier */
	putbits(forw_hor_f_code,4); /* forward_horizontal_f_code */
	putbits(forw_vert_f_code,4); /* forward_vertical_f_code */
	putbits(back_hor_f_code,4); /* backward_horizontal_f_code */
	putbits(back_vert_f_code,4); /* backward_vertical_f_code */
	putbits(dc_prec,2); /* intra_dc_precision */
	putbits(pict_struct,2); /* picture_structure */
	putbits((pict_struct==FRAME_PICTURE)?topfirst : 0, 1); /* top_field_first */
	putbits(frame_pred_dct,1); /* frame_pred_frame_dct */
	putbits(0,1); /* concealment_motion_vectors  -- currently not implemented */
	putbits(q_scale_type,1); /* q_scale_type */
	putbits(intravlc,1); /* intra_vlc_format */
	putbits(altscan,1); /* alternate_scan */
	putbits(repeatfirst,1); /* repeat_first_field */

	putbits(prog_frame,1); /* chroma_420_type */
	putbits(prog_frame,1); /* progressive_frame */
	putbits(0,1); /* composite_display_flag */
}


void Picture::PutSliceHdr( int slice_mb_y )
{
    /* slice header (6.2.4) */
    alignbits();
    
    if (opt->mpeg1 || opt->vertical_size<=2800)
        putbits(SLICE_MIN_START+slice_mb_y,32); /* slice_start_code */
    else
    {
        putbits(SLICE_MIN_START+(slice_mb_y&127),32); /* slice_start_code */
        putbits(slice_mb_y>>7,3); /* slice_vertical_position_extension */
    }
    
    /* quantiser_scale_code */
    putbits(q_scale_type 
            ? map_non_linear_mquant[mquant_pred] 
            : mquant_pred >> 1, 5);
    
    putbits(0,1); /* extra_bit_slice */
    
} 



/* *****************
 *
 * putpict - Quantise and encode picture with Sequence and GOP headers
 * as required.
 *
 *
 * TODO: Really we should seperate the Sequence start / GOP start logic
 * out.
 *
 ******************/

void Picture::PutHeadersAndEncoding( RateCtl &ratecontrol )
{

	/* Handle splitting of output stream into sequences of desired size */
	if( new_seq )
	{
		putseqend();
		ratecontrol.InitSeq(true);
	}
	/* Handle start of GOP stuff... */
	if( gop_start )
	{
		ratecontrol.InitGOP( np, nb);
	}

	ratecontrol.CalcVbvDelay(*this);
    ratecontrol.InitPict(*this); /* set up rate control */

	/* Sequence header if new sequence or we're generating for a
       format like (S)VCD that mandates sequence headers every GOP to
       do fast forward, rewind etc.
	*/

    if( new_seq || decode == 0 ||
        (gop_start && opt->seq_hdr_every_gop) )
    {
		putseqhdr();
    }
	if( gop_start )
	{
		/* set closed_GOP in first GOP only No need for per-GOP seqhdr
		   in first GOP as one has already been created.
		*/
        
		putgophdr( decode,
				   closed_gop );
	}
    
    QuantiseAndPutEncoding(ratecontrol);
}

/* ************************************************
 *
 * QuantiseAndEncode - Quantise and Encode a picture.
 *
 * NOTE: It may seem perverse to quantise in the sample as
 * coding. However, actually makes (limited) sense: feedback from the
 * *actual* bit-allocation may be used to adjust quantisation "on the
 * fly".  We, of course, need the quantised DCT blocks to construct
 * the reference picture for future motion compensation etc.
 *
 * *********************************************** */

void Picture::QuantiseAndPutEncoding(RateCtl &ratectl)
{
	int i, j, k;
	int MBAinc;
	MacroBlock *cur_mb = 0;
	// MEANX
	int totalQuant=0;
    
	/* picture header and picture coding extension */
    PutHeader();

    /* TODO: This should really be a member of the picture object */
	if( opt->svcd_scan_data && pict_type == I_TYPE )
	{
		putuserdata( dummy_svcd_scan_data, sizeof(dummy_svcd_scan_data) );
	}

	mquant_pred = ratectl.InitialMacroBlockQuant(*this);

	k = 0;
    
    /* TODO: We're currently hard-wiring each macroblock row as a
       slice.  For MPEG-2 we could do this better and reduce slice
       start code coverhead... */

	for (j=0; j<mb_height2; j++)
	{

        PutSliceHdr(j);
        Reset_DC_DCT_Pred();
        Reset_MV_Pred();

        MBAinc = 1; /* first MBAinc denotes absolute position */

        /* Slice macroblocks... */
		for (i=0; i<mb_width; i++)
		{
            prev_mb = cur_mb;
			cur_mb = &mbinfo[k];

			/* determine mquant (rate control) */
            cur_mb->mquant = ratectl.MacroBlockQuant( *cur_mb );

			/* quantize macroblock : N.b. the MB_PATTERN bit may be
               set as a side-effect of this call. */
            cur_mb->Quantize();
	    totalQuant+=cur_mb->mquant;

			/* output mquant if it has changed */
			if (cur_mb->cbp && mquant_pred!=cur_mb->mquant)
				cur_mb->final_me.mb_type|= MB_QUANT;

            /* Check to see if Macroblock is skippable, this may set
               the MB_FORWARD bit... */
            bool slice_begin_or_end = (i==0 || i==mb_width-1);
            cur_mb->SkippedCoding(slice_begin_or_end);
            if( cur_mb->skipped )
            {
                ++MBAinc;
            }
            else
            {
                putaddrinc(MBAinc); /* macroblock_address_increment */
                MBAinc = 1;
                
                putmbtype(pict_type,cur_mb->final_me.mb_type); /* macroblock type */

                if ( (cur_mb->final_me.mb_type & (MB_FORWARD|MB_BACKWARD)) && !frame_pred_dct)
                    putbits(cur_mb->final_me.motion_type,2);

                if (pict_struct==FRAME_PICTURE 	&& cur_mb->cbp && !frame_pred_dct)
                    putbits(cur_mb->field_dct,1);

                if (cur_mb->final_me.mb_type & MB_QUANT)
                {
                    putbits(q_scale_type 
                            ? map_non_linear_mquant[cur_mb->mquant]
                            : cur_mb->mquant>>1,5);
                    mquant_pred = cur_mb->mquant;
                }


                if (cur_mb->final_me.mb_type & MB_FORWARD)
                {
                    /* forward motion vectors, update predictors */
                    PutMVs( cur_mb->final_me, false );
                }

                if (cur_mb->final_me.mb_type & MB_BACKWARD)
                {
                    /* backward motion vectors, update predictors */
                    PutMVs( cur_mb->final_me,  true );
                }

                if (cur_mb->final_me.mb_type & MB_PATTERN)
                {
                    putcbp((cur_mb->cbp >> (block_count-6)) & 63);
                    if (opt->chroma_format!=CHROMA420)
                        putbits(cur_mb->cbp,block_count-6);
                }
            
                /* Output VLC DCT Blocks for Macroblock */

                cur_mb->PutBlocks( );
                /* reset predictors */
                if (!(cur_mb->final_me.mb_type & MB_INTRA))
                    Reset_DC_DCT_Pred();

                if (cur_mb->final_me.mb_type & MB_INTRA || 
                    (pict_type==P_TYPE && !(cur_mb->final_me.mb_type & MB_FORWARD)))
                {
                    Reset_MV_Pred();
                }
            }
            ++k;
        } /* Slice MB loop */
    } /* Slice loop */

	ratectl.UpdatePict(*this);
	
	double db;
	db=mb_width*mb_height;
	db=totalQuant/db+0.49;
	this->averageQuant= (int)floor(db);
}



/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
