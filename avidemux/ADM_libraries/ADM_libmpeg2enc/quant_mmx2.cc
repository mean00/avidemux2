/*
 *   Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
 * 
 * 
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * 
 * 
 *   quantize_ni_mmx.s:  MMX optimized coefficient quantization sub-routine
 */

#include <ADM_default.h>
#ifdef HAVE_X86CPU
#include "mjpeg_types.h"
#include "mmx.h"

		
void iquant_non_intra_m1_mmx(int16_t *src, int16_t *dst, uint16_t *quant_mat)
{
	int i;

		/*
 		 *   MMX Register usage
		 *   mm7 = [1|0..3]W
		 *   mm6 = [MAX_UINT16-2047|0..3]W
		 *   mm5 = 0
		 */

	/* Load 1 into all 4 words of mm7 */
	pxor_r2r(mm7, mm7);
	pcmpeqw_r2r(mm7, mm7);
	psrlw_i2r(15, mm7);
	
	pxor_r2r(mm6, mm6);
	
	for(i=0; i<64; i+=4) /* 64 coeffs in a DCT block */
	{
		movq_m2r(src[0], mm0); 	/* mm0 = *psrc */
		src += 4; 
		
		movq_r2r(mm0, mm2);	/* mm2 = TRUE where *psrc==0 */
		pcmpeqw_r2r(mm6, mm2);
		
		movq_r2r(mm0, mm3);	/* mm3 = TRUE where *psrc<0 */
		psraw_i2r(15, mm3);
		
		/* Work with absolute value for convenience... */
		pxor_r2r (mm3, mm0); 	/* mm0 = abs(*psrc) */
		psubw_r2r(mm3, mm0);
		
		paddw_r2r(mm0, mm0);	/* mm0 = 2*abs(*psrc)     */
		paddw_r2r(mm7, mm0);	/* mm0 = 2*abs(*psrc) + 1 */
		
		movq_m2r(quant_mat[0], mm4); /* multiply by *quant_mat */
		movq_r2r(mm0, mm1);
		pmullw_r2r(mm4, mm0);
		pmulhw_r2r(mm4, mm1);
		quant_mat += 4;
		
		pcmpgtw_r2r(mm6, mm1); 	/* if there was overflow, saturate low bits with all 1's */
		por_r2r(mm1, mm0);
		
		psrlw_i2r(5, mm0);	/* divide by 32 (largest possible value = 65535/32 == 2047) */
		
		/* zero case */
		pandn_r2r(mm0, mm2);	/* set to 0 where *psrc==0 */
		
		/* mismatch control */
		movq_r2r(mm2, mm1);
		psubw_r2r(mm7, mm2);
		pcmpeqw_r2r(mm6, mm1);	/* mm0 = v==0 */
		por_r2r(mm7, mm2);
		pandn_r2r(mm2, mm1);
		
		/* Handle zero case and restoring sign */
		pxor_r2r(mm3, mm1); 	/* retain original sign of *psrc */
		psubw_r2r(mm3, mm1);
		
		movq_r2m(mm1, dst[0]);
		dst += 4;
	}
	
	emms();
}


/* extmmx Inverse mpeg-2 quantisation routine. */
void iquant_non_intra_m2_mmx(int16_t *src, int16_t *dst, uint16_t *quant_mat)
{
	int sum;
	int i;
	int16_t *dst2;
	
		/*
 		 *  mm0 *psrc, scratch
		 *  mm1 *pdst
		 *  mm2 TRUE if *psrc is 0, then scratch
		 *  mm3 TRUE if *psrc is negative
		 *  mm4 Partial sums 
		 *  mm5 
		 *  mm6 <0,0,0,0>
		 *  mm7 <1,1,1,1>
		 */

	/* Load 1 into all 4 words of mm7 */
	pxor_r2r(mm7, mm7);
	pcmpeqw_r2r(mm7, mm7);
	psrlw_i2r(15, mm7);
	
	pxor_r2r(mm6, mm6);
	pxor_r2r(mm4, mm4);

	dst2 = dst;

	for(i=0; i<64; i+=4) /* 64 coeffs in a DCT block */
	{
		movq_m2r(src[0], mm0); 	/* mm0 = *psrc */
		src += 4; 
		
		movq_r2r(mm0, mm2);	/* mm2 = TRUE where *psrc==0 */
		pcmpeqw_r2r(mm6, mm2);
		
		movq_r2r(mm0, mm3);	/* mm3 = TRUE where *psrc<0 */
		psraw_i2r(15, mm3);
		
		/* Work with absolute value for convenience... */
		pxor_r2r (mm3, mm0); 	/* mm0 = abs(*psrc) */
		psubw_r2r(mm3, mm0);
		
		paddw_r2r(mm0, mm0);	/* mm0 = 2*abs(*psrc)      */
		paddw_r2r(mm7, mm0);	/* mm0 = 2*abs(*psrc) + 1  */
		pandn_r2r(mm0, mm2);	/* set to 0 where *psrc==0 */
		
		movq_m2r(quant_mat[0], mm1); /* multiply by *quant_mat */
		movq_r2r(mm2, mm0);
		pmulhw_r2r(mm1, mm2);
		pmullw_r2r(mm1, mm0);
		quant_mat += 4;
		
		pcmpgtw_r2r(mm6, mm2); 	/* if there was overflow, saturate low bits with all 1's */
		por_r2r(mm2, mm0);
		
		psrlw_i2r(5, mm0); 	/* divide by 32 (largest possible value = 65535/32 == 2047) */
		
		/* Accumulate sum... */
		paddw_r2r(mm0, mm4);
		
		/* Handle zero case and restoring sign */
		pxor_r2r(mm3, mm0);	/* retain original sign of *psrc */
		psubw_r2r(mm3, mm0);
		
		movq_r2m(mm0, dst[0]);
		dst += 4;
	}
	
	/* Mismatch control compute lower bits of sum... */
	movq_r2r(mm4, mm5);
	psrlq_i2r(32, mm5);
	paddw_r2r(mm5, mm4);
	movq_r2r(mm4, mm5);
	psrlq_i2r(16, mm5);
	paddw_r2r(mm5, mm4);
	movd_r2g(mm4, sum);
	
	sum &= 1;
	sum ^= 1;
	dst2[63] ^= sum;

	emms();
}


/* 
 *  Simply add up the sum of coefficients weighted  
 *  by their quantisation coefficients
 */
int32_t quant_weight_coeff_sum_mmx(int16_t *src, int16_t *i_quant_mat)
{
	int32_t sum, sum1, sum2;
	int i;
		/*
		 *   MMX Register usage
		 *   mm7 = [1|0..3]W
		 *   mm6 = [2047|0..3]W
		 *   mm5 = 0
		 */

	pxor_r2r(mm6, mm6); /*  Accumulator */
	
	for(i=0; i<16; i+=2) /* 16 coefficient / quantiser quads to process... */
	{
		movq_m2r(src[0], mm0);
		movq_m2r(src[4], mm2);
		pxor_r2r(mm1, mm1);
		pxor_r2r(mm3, mm3);
	
		/* 
		 *  Compute absolute value of coefficients...
		 */
		pcmpgtw_r2r(mm0, mm1); 	/* (mm0 < 0 ) */
		pcmpgtw_r2r(mm2, mm3); 	/* (mm0 < 0 ) */
		pxor_r2r(mm1, mm0);
		pxor_r2r(mm3, mm2);
		psubw_r2r(mm1, mm0);
		psubw_r2r(mm3, mm2);
		
		/* 
		 *  Compute the low and high words of the result....
		 */
		pmaddwd_m2r(i_quant_mat[0], mm0);
		pmaddwd_m2r(i_quant_mat[4], mm2);
		src += 8;
		i_quant_mat += 8;
		paddd_r2r(mm0, mm6);
		paddd_r2r(mm2, mm6);	
	}

	movd_r2g(mm6, sum1);
	psrlq_i2r(32, mm6);
	movd_r2g(mm6, sum2);
	sum = sum1 + sum2;
	
	emms();
	
	return sum;
}

#endif
