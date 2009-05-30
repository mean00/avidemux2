/* predict.cc, motion compensated prediction                                 */

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


#include "ADM_default.h"
#include "mjpeg_logging.h"
#include "global.h"
#include "predict_ref.h"
void pred_comp_mmxe(
	uint8_t *src,
	uint8_t *dst,
	int lx,
	int w, int h,
	int x, int y,
	int dx, int dy,
	int addflag);
void pred_comp_mmx(
	uint8_t *src,
	uint8_t *dst,
	int lx,
	int w, int h,
	int x, int y,
	int dx, int dy,
	int addflag);
/* form prediction for a complete picture (frontend for predict_mb)
 *
 * mbi:  macroblock info
 *
 */

void predict(Picture *picture)
{
	vector<MacroBlock>::iterator mbi;

	/* loop through all macroblocks of the picture */
	for( mbi = picture->mbinfo.begin(); mbi < picture->mbinfo.end(); ++mbi)
		mbi->Predict();
}


/* predict a rectangular block (all three components)
 *
 * src:     source frame (Y,U,V)
 * sfield:  source field select (0: frame or top field, 1: bottom field)
 * dst:     destination frame (Y,U,V)
 * dfield:  destination field select (0: frame or top field, 1: bottom field)
 *
 * the following values are in luminance picture (frame or field) dimensions
 * lx:      distance of vertically adjacent pels (selects frame or field pred.)
 * w,h:     width and height of block (only 16x16 or 16x8 are used)
 * x,y:     coordinates of destination block
 * dx,dy:   half pel motion vector
 * addflag: store or add (= average) prediction
 */

void pred (	uint8_t *src[], int sfield,
			uint8_t *dst[], int dfield,
			int lx, int w, int h, int x, int y, 
			int dx, int dy, bool addflag
	)
{
	int cc;

	for (cc=0; cc<3; cc++)
	{
		if (cc==1)
		{
			/* scale for color components */
			if (opt->chroma_format==CHROMA420)
			{
				/* vertical */
				h >>= 1; y >>= 1; dy /= 2;
			}
			if (opt->chroma_format!=CHROMA444)
			{
				/* horizontal */
				w >>= 1; x >>= 1; dx /= 2;
				lx >>= 1;
			}
		}
		ppred_comp(	src[cc]+(sfield?lx>>1:0),dst[cc]+(dfield?lx>>1:0),
					lx,w,h,x,y,dx,dy, (int)addflag);
	}
}


/* form prediction for one macroblock
 *
 * lx:     frame width (identical to global var `width')
 *
 * Notes:
 * - when predicting a P type picture which is the second field of
 *   a frame, the same parity reference field is in oldref, while the
 *   opposite parity reference field is assumed to be in newref!
 * - intra macroblocks are modelled to have a constant prediction of 128
 *   for all pels; this results in a DC DCT coefficient symmetric to 0
 * - vectors for field prediction in frame pictures are in half pel frame
 *   coordinates (vertical component is twice the field value and always
 *   even)
 *
 * already covers dual prime (not yet used)
 */

void MacroBlock::Predict()
{
	const Picture &picture = ParentPicture();
	int bx = TopleftX();
	int by = TopleftY();
	uint8_t **oldref = picture.oldref;	// Forward prediction
	uint8_t **newref = picture.newref;	// Backward prediction
	uint8_t **cur = picture.pred;      // Frame to predict
	int lx = opt->phy_width;

	bool addflag;
	int currentfield;
	uint8_t **predframe;
	int DMV[2][2];

	if (final_me.mb_type&MB_INTRA)
	{
		clearblock(picture.pict_struct,cur,bx,by);
		return;
	}

	addflag = false; /* first prediction is stored, second is added and averaged */

	if ((final_me.mb_type & MB_FORWARD) || (picture.pict_type==P_TYPE))
	{
		/* forward prediction, including zero MV in P pictures */

		if (picture.pict_struct==FRAME_PICTURE)
		{
			/* frame picture */

			if ( (final_me.motion_type==MC_FRAME) 
				 || !(final_me.mb_type & MB_FORWARD))
			{
				/* frame-based prediction in frame picture */
				pred( oldref,0,cur,0,
					 lx,16,16,bx,by,final_me.MV[0][0][0],final_me.MV[0][0][1],false);
			}
			else if (final_me.motion_type==MC_FIELD)
			{
				/* field-based prediction in frame picture
				 *
				 * note scaling of the vertical coordinates (by, MV[][0][1])
				 * from frame to field!
				 */

				/* top field prediction */
				pred(oldref,final_me.field_sel[0][0],cur,0,
					 lx<<1,16,8,bx,by>>1,
					 final_me.MV[0][0][0],final_me.MV[0][0][1]>>1,false);

				/* bottom field prediction */
				pred(oldref,final_me.field_sel[1][0],cur,1,
					 lx<<1,16,8,bx,by>>1,
					 final_me.MV[1][0][0],final_me.MV[1][0][1]>>1,false);
			}
			else if (final_me.motion_type==MC_DMV)
			{
				/* dual prime prediction */

				/* calculate derived motion vectors */
				calc_DMV(picture.pict_struct, picture.topfirst,
						 DMV,final_me.dualprimeMV,
						 final_me.MV[0][0][0],final_me.MV[0][0][1]>>1);

				/* predict top field from top field */
				pred(oldref,0,cur,0,
					 lx<<1,16,8,bx,by>>1,
					 final_me.MV[0][0][0],final_me.MV[0][0][1]>>1,false);

				/* predict bottom field from bottom field */
				pred(oldref,1,cur,1,
					 lx<<1,16,8,bx,by>>1,
					 final_me.MV[0][0][0],final_me.MV[0][0][1]>>1,false);

				/* predict and add to top field from bottom field */
				pred(oldref,1,cur,0,
					 lx<<1,16,8,bx,by>>1,
					 DMV[0][0],DMV[0][1],true);

				/* predict and add to bottom field from top field */
				pred(oldref,0,cur,1,
					 lx<<1,16,8,bx,by>>1,
					 DMV[1][0],DMV[1][1],true);
			}
			else
			{
				/* invalid motion_type in frame picture */
				mjpeg_error_exit1("Internal: invalid motion_type");
			}
		}
		else /* TOP_FIELD or BOTTOM_FIELD */
		{
			/* field picture */

			currentfield = (picture.pict_struct==BOTTOM_FIELD);

			/* determine which frame to use for prediction */
			if ((picture.pict_type==P_TYPE) && picture.secondfield
				&& (currentfield!=final_me.field_sel[0][0]))
				predframe = newref; /* same frame */
			else
				predframe = oldref; /* previous frame */

			if ( final_me.motion_type==MC_FIELD
				 || !(final_me.mb_type & MB_FORWARD))
			{
				/* field-based prediction in field picture */
				pred(predframe,final_me.field_sel[0][0],cur,currentfield,
					 lx<<1,16,16,bx,by,
					 final_me.MV[0][0][0],final_me.MV[0][0][1],false);
			}
			else if (final_me.motion_type==MC_16X8)
			{
				/* 16 x 8 motion compensation in field picture */

				/* upper half */
				pred(predframe,final_me.field_sel[0][0],cur,currentfield,
					 lx<<1,16,8,bx,by,
					 final_me.MV[0][0][0],final_me.MV[0][0][1],false);

				/* determine which frame to use for lower half prediction */
				if ((picture.pict_type==P_TYPE) && picture.secondfield
					&& (currentfield!=final_me.field_sel[1][0]))
					predframe = newref; /* same frame */
				else
					predframe = oldref; /* previous frame */

				/* lower half */
				pred(predframe,final_me.field_sel[1][0],cur,currentfield,
					 lx<<1,16,8,bx,by+8,
					 final_me.MV[1][0][0],final_me.MV[1][0][1],false);
			}
			else if (final_me.motion_type==MC_DMV)
			{
				/* dual prime prediction */

				/* determine which frame to use for prediction */
				if (picture.secondfield)
					predframe = newref; /* same frame */
				else
					predframe = oldref; /* previous frame */

				/* calculate derived motion vectors */
				calc_DMV(picture.pict_struct,
						 picture.topfirst,
						 DMV,final_me.dualprimeMV,
						 final_me.MV[0][0][0],final_me.MV[0][0][1]);

				/* predict from field of same parity */
				pred(oldref,currentfield,cur,currentfield,
					 lx<<1,16,16,bx,by,
					 final_me.MV[0][0][0],final_me.MV[0][0][1],false);

				/* predict from field of opposite parity */
				pred(predframe,!currentfield,cur,currentfield,
					 lx<<1,16,16,bx,by,
					 DMV[0][0],DMV[0][1],true);
			}
			else
			{
				/* invalid motion_type in field picture */
				mjpeg_error_exit1("Internal: invalid motion_type");
			}
		}
		addflag = true; /* next prediction (if any) will be averaged with this one */
	}

	if (final_me.mb_type & MB_BACKWARD)
	{
		/* backward prediction */

		if (picture.pict_struct==FRAME_PICTURE)
		{
			/* frame picture */

			if (final_me.motion_type==MC_FRAME)
			{
				/* frame-based prediction in frame picture */
				pred(newref,0,cur,0,
					 lx,16,16,bx,by,
					 final_me.MV[0][1][0],final_me.MV[0][1][1],addflag);
			}
			else
			{
				/* field-based prediction in frame picture
				 *
				 * note scaling of the vertical coordinates (by, MV[][1][1])
				 * from frame to field!
				 */

				/* top field prediction */
				pred(newref,final_me.field_sel[0][1],cur,0,
					 lx<<1,16,8,bx,by>>1,
					 final_me.MV[0][1][0],final_me.MV[0][1][1]>>1,addflag);

				/* bottom field prediction */
				pred(newref,final_me.field_sel[1][1],cur,1,
					 lx<<1,16,8,bx,by>>1,
					 final_me.MV[1][1][0],final_me.MV[1][1][1]>>1,addflag);
			}
		}
		else /* TOP_FIELD or BOTTOM_FIELD */
		{
			/* field picture */

			currentfield = (picture.pict_struct==BOTTOM_FIELD);

			if (final_me.motion_type==MC_FIELD)
			{
				/* field-based prediction in field picture */
				pred(newref,final_me.field_sel[0][1],cur,currentfield,
					 lx<<1,16,16,bx,by,
					 final_me.MV[0][1][0],final_me.MV[0][1][1],addflag);
			}
			else if (final_me.motion_type==MC_16X8)
			{
				/* 16 x 8 motion compensation in field picture */

				/* upper half */
				pred(newref,final_me.field_sel[0][1],cur,currentfield,
					 lx<<1,16,8,bx,by,
					 final_me.MV[0][1][0],final_me.MV[0][1][1],addflag);

				/* lower half */
				pred(newref,final_me.field_sel[1][1],cur,currentfield,
					 lx<<1,16,8,bx,by+8,
					 final_me.MV[1][1][0],final_me.MV[1][1][1],addflag);
			}
			else
			{
				/* invalid motion_type in field picture */
				mjpeg_error_exit1("Internal: invalid motion_type");
			}
		}
	}
}
