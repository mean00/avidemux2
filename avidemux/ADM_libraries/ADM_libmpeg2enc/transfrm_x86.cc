/* transfrm.c,  forward / inverse transformation  
   In compiler (gcc) embdeed assembly language...              */

/* Modifications and enhancements (C) 2000-2003 Andrew Stevens */

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
#ifdef HAVE_X86CPU
#include <math.h>
#include "mjpeg_types.h"
#include "syntaxparams.h"
#include "attributes.h"
#include "mmx.h"
#include "simd.h"

static __inline__ void
mmx_sum_4_word_accs( mmx_t *accs, int32_t *res )
{
	movq_m2r( *accs, mm1 );
	movq_r2r( mm1, mm3 );
	movq_r2r( mm1, mm2 );
	/* Generate sign extensions for mm1 words! */
	psraw_i2r( 15, mm3 );
	punpcklwd_r2r( mm3, mm1 );
	punpckhwd_r2r( mm3, mm2 );
	paddd_r2r( mm1, mm2 );
	movq_r2r( mm2, mm3);
	psrlq_i2r( 32, mm2);
	paddd_r2r( mm2, mm3);
	movd_r2m( mm3, *res );
}


static __inline__ void
sum_sumsq_8bytes( uint8_t *cur_lum_mb, 
				  uint8_t *pred_lum_mb,
				  mmx_t *sumtop_accs,
				  mmx_t *sumbot_accs,
				  mmx_t *sumsqtop_accs,
				  mmx_t *sumsqbot_accs,
				  mmx_t *sumxprod_accs
	)
{
	pxor_r2r(mm0,mm0);

	/* Load pixels from top field into mm1.w,mm2.w
	 */
	movq_m2r( *((mmx_t*)cur_lum_mb), mm1 );
	movq_m2r( *((mmx_t*)pred_lum_mb), mm2 );
	
	/* mm3 := mm1 mm4 := mm2
	   mm1.w[0..3] := mm1.b[0..3]-mm2.b[0..3]
	*/ 
	
	movq_r2r( mm1, mm3 );
	punpcklbw_r2r( mm0, mm1 );
	movq_r2r( mm2, mm4 );
	punpcklbw_r2r( mm0, mm2 );
	psubw_r2r( mm2, mm1 );
	
	/* mm3.w[0..3] := mm3.b[4..7]-mm4.b[4..7]
	 */
	punpckhbw_r2r( mm0, mm3 );
	punpckhbw_r2r( mm0, mm4 );
	psubw_r2r( mm4, mm3 );

	/* sumtop_accs->w[0..3] += mm1.w[0..3];
	   sumtop_accs->w[0..3] += mm3.w[0..3];
	   mm6 = mm1; mm7 = mm3;
	*/
	movq_m2r( *sumtop_accs, mm5 );
	paddw_r2r( mm1, mm5 );
	paddw_r2r( mm3, mm5 );
	movq_r2r( mm1, mm6 );
	movq_r2r( mm3, mm7 );
	movq_r2m( mm5, *sumtop_accs );

	/* 
	   *sumsq_top_acc += mm1.w[0..3] * mm1.w[0..3];
	   *sumsq_top_acc += mm3.w[0..3] * mm3.w[0..3];
	*/
	pmaddwd_r2r( mm1, mm1 );
	movq_m2r( *sumsqtop_accs, mm5 );
	pmaddwd_r2r( mm3, mm3 );
	paddd_r2r( mm1, mm5 );
	paddd_r2r( mm3, mm5 );
	movq_r2m( mm5, *sumsqtop_accs );
	

	/* Load pixels from bot field into mm1.w,mm2.w
	 */
	movq_m2r( *((mmx_t*)(cur_lum_mb+opt->phy_width)), mm1 );
	movq_m2r( *((mmx_t*)(pred_lum_mb+opt->phy_width)), mm2 );
	
	/* mm2 := mm1 mm4 := mm2
	   mm1.w[0..3] := mm1.b[0..3]-mm2.b[0..3]
	*/ 
	
	movq_r2r( mm1, mm3 );
	punpcklbw_r2r( mm0, mm1 );
	movq_r2r( mm2, mm4 );
	punpcklbw_r2r( mm0, mm2 );
	psubw_r2r( mm2, mm1 );
	
	/* mm3.w[0..3] := mm3.b[4..7]-mm4.b[4..7]
	 */
	punpckhbw_r2r( mm0, mm3 );
	punpckhbw_r2r( mm0, mm4 );
	psubw_r2r( mm4, mm3 );

	/* 
	   sumbot_accs->w[0..3] += mm1.w[0..3];
	   sumbot_accs->w[0..3] += mm3.w[0..3];
	   mm2 := mm1; mm4 := mm3;
	*/
	movq_m2r( *sumbot_accs, mm5 );
	paddw_r2r( mm1, mm5 );
	movq_r2r( mm1, mm2 );
	paddw_r2r( mm3, mm5 );
	movq_r2r( mm3, mm4 );
	movq_r2m( mm5, *sumbot_accs );

	/* 
	   *sumsqbot_acc += mm1.w[0..3] * mm1.w[0..3];
	   *sumsqbot_acc += mm3.w[0..3] * mm3.w[0..3];
	*/
	pmaddwd_r2r( mm1, mm1 );
	movq_m2r( *sumsqbot_accs, mm5 );
	pmaddwd_r2r( mm3, mm3 );
	paddd_r2r( mm1, mm5 );
	paddd_r2r( mm3, mm5 );
	movq_r2m( mm5, *sumsqbot_accs );
	
	
	/* Accumulate cross-product 
	 *sum_xprod_acc += mm1.w[0..3] * mm6[0..3];
	 *sum_xprod_acc += mm3.w[0..3] * mm7[0..3];
	 */

	movq_m2r( *sumxprod_accs, mm5 );
	pmaddwd_r2r( mm6, mm2);
	pmaddwd_r2r( mm7, mm4);
	paddd_r2r( mm2, mm5 );
	paddd_r2r( mm4, mm5 );
	movq_r2m( mm5, *sumxprod_accs );
	emms();
}

int field_dct_best_mmx( uint8_t *cur_lum_mb, uint8_t *pred_lum_mb)
{
	/*
	 * calculate prediction error (cur-pred) for top (blk0)
	 * and bottom field (blk1)
	 */
	double r,d;
	int rowoffs = 0;
	int32_t sumtop, sumbot, sumsqtop, sumsqbot, sumbottop;
	int j;
	int dct_type;
	int topvar, botvar;
	mmx_t sumtop_accs, sumbot_accs;
	mmx_t sumsqtop_accs, sumsqbot_accs, sumxprod_accs;
	int32_t sumtop_acc, sumbot_acc;
	int32_t sumsqtop_acc, sumsqbot_acc, sumxprod_acc;

	pxor_r2r(mm0,mm0);
	movq_r2m( mm0, *(&sumtop_accs) );
	movq_r2m( mm0, *(&sumbot_accs) );
	movq_r2m( mm0, *(&sumsqtop_accs) );
	movq_r2m( mm0, *(&sumsqbot_accs) );
	movq_r2m( mm0, *(&sumxprod_accs) );
	
	sumtop = sumsqtop = sumbot = sumsqbot = sumbottop = 0;
	sumtop_acc = sumbot_acc = sumsqtop_acc = sumsqbot_acc = sumxprod_acc = 0; 
	for (j=0; j<8; j++)
	{
#ifdef ORIGINAL_CODE
		for (i=0; i<16; i++)
		{
			register int toppix = 
				cur_lum_mb[rowoffs+i] - pred_lum_mb[rowoffs+i];
			register int botpix = 
				cur_lum_mb[rowoffs+width+i] - pred_lum_mb[rowoffs+width+i];
			sumtop += toppix;
			sumsqtop += toppix*toppix;
			sumbot += botpix;
			sumsqbot += botpix*botpix;
			sumbottop += toppix*botpix;
		}
#endif
		sum_sumsq_8bytes( &cur_lum_mb[rowoffs], &pred_lum_mb[rowoffs],
						  &sumtop_accs, &sumbot_accs,
						  &sumsqtop_accs, &sumsqbot_accs, &sumxprod_accs
						  );
		sum_sumsq_8bytes( &cur_lum_mb[rowoffs+8], &pred_lum_mb[rowoffs+8],
						  &sumtop_accs, &sumbot_accs,
						  &sumsqtop_accs, &sumsqbot_accs, &sumxprod_accs );
		rowoffs += (opt->phy_width<<1);
	}

	mmx_sum_4_word_accs( &sumtop_accs, &sumtop );
	mmx_sum_4_word_accs( &sumbot_accs, &sumbot );
	emms();
	sumsqtop = sumsqtop_accs.d[0] + sumsqtop_accs.d[1];
	sumsqbot = sumsqbot_accs.d[0] + sumsqbot_accs.d[1];
	sumbottop = sumxprod_accs.d[0] + sumxprod_accs.d[1];

	/* Calculate Variances top and bottom.  If they're of similar
	 sign estimate correlation if its good use frame DCT otherwise
	 use field.
	*/
	r = 0.0;
	topvar = sumsqtop-sumtop*sumtop/128;
	botvar = sumsqbot-sumbot*sumbot/128;
	if ( !((topvar <= 0) ^ (botvar <= 0)) )
	{
		d = ((double) topvar) * ((double)botvar);
		r = (sumbottop-(sumtop*sumbot)/128);
		if (r>0.5*sqrt(d))
			return 0; /* frame DCT */
		else
			return 1; /* field DCT */
	}
	else
		return 1; /* field DCT */

	return dct_type;
}
#endif
