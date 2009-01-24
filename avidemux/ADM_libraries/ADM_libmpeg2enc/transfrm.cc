/* transfrm.c,  forward / inverse transformation                            */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */


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
#include "transfrm_ref.h"



void MacroBlock::Transform()
{
	uint8_t **cur = picture->curorg;
	uint8_t **pred = picture->pred;
	// assert( dctblocks == &blocks[k*block_count]);
	int i = TopleftX();
	int j = TopleftY();
	int blocktopleft = j*opt->phy_width+i;
	field_dct =
		! picture->frame_pred_dct 
		&& picture->pict_struct == FRAME_PICTURE
		&& pfield_dct_best( &cur[0][blocktopleft], 
							&pred[0][blocktopleft]);
	int i1, j1, n, cc, offs, lx;

	for (n=0; n<block_count; n++)
	{
		cc = (n<4) ? 0 : (n&1)+1; /* color component index */
		if (cc==0)
		{
			/* A.Stevens Jul 2000 Record dct blocks associated
			 * with macroblock We'll use this for quantisation
			 * calculations  */
			/* luminance */
			if ((picture->pict_struct==FRAME_PICTURE) && field_dct)
			{
				/* field DCT */
				offs = i + ((n&1)<<3) + opt->phy_width*(j+((n&2)>>1));
				lx =  opt->phy_width<<1;
			}
			else
			{
				/* frame DCT */
				offs = i + ((n&1)<<3) +  opt->phy_width2*(j+((n&2)<<2));
				lx =  opt->phy_width2;
			}

			if (picture->pict_struct==BOTTOM_FIELD)
				offs +=  opt->phy_width;
		}
		else
		{
			/* chrominance */

			/* scale coordinates */
			i1 = (opt->chroma_format==CHROMA444) ? i : i>>1;
			j1 = (opt->chroma_format!=CHROMA420) ? j : j>>1;

			if ((picture->pict_struct==FRAME_PICTURE) && field_dct
				&& (opt->chroma_format!=CHROMA420))
			{
				/* field DCT */
				offs = i1 + (n&8) +  opt->phy_chrom_width*(j1+((n&2)>>1));
				lx =  opt->phy_chrom_width<<1;
			}
			else
			{
				/* frame DCT */
				offs = i1 + (n&8) +  opt->phy_chrom_width2*(j1+((n&2)<<2));
				lx =  opt->phy_chrom_width2;
			}

			if (picture->pict_struct==BOTTOM_FIELD)
				offs +=  opt->phy_chrom_width;
		}

		psub_pred(pred[cc]+offs,cur[cc]+offs,lx, dctblocks[n]);
		pfdct(dctblocks[n]);
	}
		
}
/* subtract prediction and transform prediction error */
void transform(	Picture *picture )
{
	vector<MacroBlock>::iterator mbi;

	for( mbi = picture->mbinfo.begin(); mbi < picture->mbinfo.end(); ++mbi)
	{
		mbi->Transform();
	}
}

void MacroBlock::ITransform()
{
	uint8_t **cur = picture->curref;
	uint8_t **pred = picture->pred;

	int i1, j1, n, cc, offs, lx;
	int i = TopleftX();
	int j = TopleftY();
			
	for (n=0; n<block_count; n++)
	{
		cc = (n<4) ? 0 : (n&1)+1; /* color component index */
			
		if (cc==0)
		{
			/* luminance */
			if ((picture->pict_struct==FRAME_PICTURE) && field_dct)
			{
				/* field DCT */
				offs = i + ((n&1)<<3) + opt->phy_width*(j+((n&2)>>1));
				lx = opt->phy_width<<1;
			}
			else
			{
				/* frame DCT */
				offs = i + ((n&1)<<3) + opt->phy_width2*(j+((n&2)<<2));
				lx = opt->phy_width2;
			}

			if (picture->pict_struct==BOTTOM_FIELD)
				offs +=  opt->phy_width;
		}
		else
		{
			/* chrominance */

			/* scale coordinates */
			i1 = (opt->chroma_format==CHROMA444) ? i : i>>1;
			j1 = (opt->chroma_format!=CHROMA420) ? j : j>>1;

			if ((picture->pict_struct==FRAME_PICTURE) && field_dct
				&& (opt->chroma_format!=CHROMA420))
			{
				/* field DCT */
				offs = i1 + (n&8) + opt->phy_chrom_width*(j1+((n&2)>>1));
				lx = opt->phy_chrom_width<<1;
			}
			else
			{
				/* frame DCT */
				offs = i1 + (n&8) + opt->phy_chrom_width2*(j1+((n&2)<<2));
				lx = opt->phy_chrom_width2;
			}

			if (picture->pict_struct==BOTTOM_FIELD)
				offs +=  opt->phy_chrom_width;
		}
		pidct(qdctblocks[n]);
		padd_pred(pred[cc]+offs,cur[cc]+offs,lx,qdctblocks[n]);
	}
}

/* inverse transform prediction error and add prediction */
void itransform(Picture *picture)
{
    vector<MacroBlock>::iterator mbi = picture->mbinfo.begin();
	for( mbi = picture->mbinfo.begin(); mbi < picture->mbinfo.end(); ++mbi)
	{
		mbi->ITransform();
	}
}


