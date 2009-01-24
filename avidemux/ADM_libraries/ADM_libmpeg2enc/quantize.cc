/* quantize.c, quantization / inverse quantization                          */

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


#include "config.h"
#include "global.h"
#include "quantize_ref.h"


void MacroBlock::Quantize()
{
    if (final_me.mb_type & MB_INTRA)
    {
        mp2_quant_intra( dctblocks[0],
                     qdctblocks[0],
                     picture->q_scale_type,
                     picture->dc_prec,
                     &mquant );
		
        cbp = (1<<block_count) - 1;
    }
    else
    {
        cbp = (*pquant_non_intra)( dctblocks[0],
                                   qdctblocks[0],
                                   picture->q_scale_type,
                                   &mquant );
        if (cbp)
            final_me.mb_type|= MB_PATTERN;
    }

}

void MacroBlock::IQuantize()
{
    int j;
    if (final_me.mb_type & MB_INTRA)
    {
        for (j=0; j<block_count; j++)
            iquant_intra(qdctblocks[j], qdctblocks[j], 
                         picture->dc_prec, mquant);
    }
    else
    {
        for (j=0;j<block_count;j++)
            iquant_non_intra(qdctblocks[j], qdctblocks[j], mquant);
    }
}



void iquantize( Picture *picture )
{
#ifdef ORIGINAL_JUNK
	int j,k;
	int16_t (*qblocks)[64] = picture->qblocks;
#else
    int k;
#endif
	for (k=0; k<mb_per_pict; k++)
	{
#ifdef ORIGINAL_JUNK
		if (picture->mbinfo[k].mb_type & MB_INTRA)
			for (j=0; j<block_count; j++)
            {
				iquant_intra(qblocks[k*block_count+j],
							 qblocks[k*block_count+j],
							 picture->dc_prec,
							 picture->mbinfo[k].mquant);
            }
		else
			for (j=0;j<block_count;j++)
				iquant_non_intra(qblocks[k*block_count+j],
								 qblocks[k*block_count+j],
								 picture->mbinfo[k].mquant);
#else
        picture->mbinfo[k].IQuantize();
#endif
	}
}

/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
