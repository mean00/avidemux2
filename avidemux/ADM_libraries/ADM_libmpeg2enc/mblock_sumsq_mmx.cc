/*
 *   dist2_mmx.s:  mmX optimized squared distance sum
 * 
 *   Original believed to be Copyright (C) 2000 Brent Byeler
 * 
 *   This program is free software; you can reaxstribute it and/or
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
 */

#include "config.h"
#include "mjpeg_types.h"
#ifdef HAVE_X86CPU
#include "mmx.h"

/*
 *  total squared difference between two (16*h) blocks
 *  including optional half pel interpolation of [ebp+8] ; blk1 (hx,hy)
 *  blk1,blk2: addresses of top left pels of both blocks
 *  lx:        distance (in bytes) of vertically adjacent pels
 *  hx,hy:     flags for horizontal and/or vertical interpolation
 *  h:         height of block (usually 8 or 16)
 *  mmX version
 */
 
int sumsq_mmx(uint8_t  *blk1, uint8_t *blk2, int lx, int hx, int hy, int h)
{
	static const uint16_t twos[4]={2,2,2,2};
	int sum,sum1,sum2;

	pxor_r2r(mm5, mm5);

	if (h>0) 
	{
		pxor_r2r(mm7, mm7);
	
		if ((hx == 0) && (hy == 0)) 
		{
			do { /*  d2top00 */
				movq_m2r(blk1[0], mm0);
				movq_r2r(mm0, mm1);
				punpcklbw_r2r(mm7, mm0);
				punpckhbw_r2r(mm7, mm1);
			
				movq_m2r(blk2[0], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);
			
				psubw_r2r(mm2, mm0);
				psubw_r2r(mm3, mm1);
				pmaddwd_r2r(mm0, mm0);
				pmaddwd_r2r(mm1, mm1);
				paddd_r2r(mm1, mm0);
			
				movq_m2r(blk1[8], mm1);
				movq_r2r(mm1, mm2);
				punpcklbw_r2r(mm7, mm1);
				punpckhbw_r2r(mm7, mm2);
			
				movq_m2r(blk2[8], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
			
				psubw_r2r(mm3, mm1);
				psubw_r2r(mm4, mm2);
				pmaddwd_r2r(mm1, mm1);
				pmaddwd_r2r(mm2, mm2);
				paddd_r2r(mm2, mm1);
			
				paddd_r2r(mm1, mm0);
			
				paddd_r2r(mm0, mm5); /* Accumulate sum in mm5 */
			
				blk1 += lx;
				blk2 += lx;
				
				h--;
			} while (h);
		
		} else if ((hx != 0) && (hy == 0))  /* d2is10 */ 
		{
			pxor_r2r(mm6, mm6); /* mm6 = 0 and isn't changed anyplace in the loop.. */
			pcmpeqw_r2r(mm1, mm1);
			psubw_r2r(mm1, mm6);
			
			do { /* d2top10 */
				movq_m2r(blk1[0], mm0);
				movq_r2r(mm0, mm1);
				punpcklbw_r2r(mm7, mm0);
				punpckhbw_r2r(mm7, mm1);
				movq_m2r(blk1[1], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);
				paddw_r2r(mm2, mm0);
				paddw_r2r(mm3, mm1);
				paddw_r2r(mm6, mm0); /* here we add mm6 = 0.... weird... */
				paddw_r2r(mm6, mm1);
				psrlw_i2r(1, mm0);
				psrlw_i2r(1, mm1);
				
				movq_m2r(blk2[0], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);

				psubw_r2r(mm2, mm0);
				psubw_r2r(mm3, mm1);
				pmaddwd_r2r(mm0, mm0);
				pmaddwd_r2r(mm1, mm1);
				paddd_r2r(mm1, mm0);
				
				movq_m2r(blk1[8], mm1);
				movq_r2r(mm1, mm2);
				punpcklbw_r2r(mm7, mm1);
				punpckhbw_r2r(mm7, mm2);
				movq_m2r(blk1[9], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
				paddw_r2r(mm3, mm1);
				paddw_r2r(mm4, mm2);
				paddw_r2r(mm6, mm1);
				paddw_r2r(mm6, mm2);
				psrlw_i2r(1, mm1);				
				psrlw_i2r(1, mm2);
				
				movq_m2r(blk2[8], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
								
				psubw_r2r(mm3, mm1);
				psubw_r2r(mm4, mm2);
				pmaddwd_r2r(mm1, mm1);
				pmaddwd_r2r(mm2, mm2);
				paddd_r2r(mm2, mm1);
				
				paddd_r2r(mm1, mm0);
				
				paddd_r2r(mm0, mm5); /* Accumulate mm0 sum on mm5  */
				
				blk1 += lx;
				blk2 += lx;
			
				h--;
			} while (h);
			
		} else if ((hx == 0) && (hy != 0))  /* d2is01 */
		{
			pxor_r2r(mm6, mm6);
			pcmpeqw_r2r(mm1, mm1);
			psubw_r2r(mm1, mm6); /* mm6 = 1 */
		
			do { /* d2top01 */
				movq_m2r(blk1[0], mm0);
				movq_r2r(mm0, mm1);
				punpcklbw_r2r(mm7, mm0);
				punpckhbw_r2r(mm7, mm1);
				movq_m2r(blk1[lx], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);
				paddw_r2r(mm2, mm0);
				paddw_r2r(mm3, mm1);
				paddw_r2r(mm6, mm0);
				paddw_r2r(mm6, mm1);
				psrlw_i2r(1, mm0);
				psrlw_i2r(1, mm1);
				
				movq_m2r(blk2[0], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);
				psubw_r2r(mm2, mm0);
				psubw_r2r(mm3, mm1);
				
				pmaddwd_r2r(mm0, mm0);
				pmaddwd_r2r(mm1, mm1);
				paddd_r2r(mm1, mm0);
				
				movq_m2r(blk1[8], mm1);
				movq_r2r(mm1, mm2);
				punpcklbw_r2r(mm7, mm1);
				punpckhbw_r2r(mm7, mm2);
				
				movq_m2r(blk1[lx+8], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
				
				paddw_r2r(mm3, mm1);
				paddw_r2r(mm4, mm2);
				paddw_r2r(mm6, mm1);
				paddw_r2r(mm6, mm2);
				psrlw_i2r(1, mm1);
				psrlw_i2r(1, mm2);
				
				movq_m2r(blk2[8], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
				
				psubw_r2r(mm3, mm1);
				psubw_r2r(mm4, mm2);
				
				pmaddwd_r2r(mm1, mm1);
				pmaddwd_r2r(mm2, mm2);
				paddd_r2r(mm1, mm0);
				paddd_r2r(mm2, mm0);
				
				paddd_r2r(mm0, mm5);
				
				blk1 += lx;
				blk2 += lx;
		
				h--;
			} while (h);
		
		} else /* d2is11 */
		{
		
			do { /* d2top11 */
				movq_m2r(blk1[0], mm0);
				movq_r2r(mm0, mm1);
				punpcklbw_r2r(mm7, mm0);
				punpckhbw_r2r(mm7, mm1);
				movq_m2r(blk1[1], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);
				paddw_r2r(mm2, mm0);
				paddw_r2r(mm3, mm1);
				movq_m2r(blk1[lx], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);
				movq_m2r(blk1[lx+1], mm4);
				movq_r2r(mm4, mm6);
				punpcklbw_r2r(mm7, mm4);
				punpckhbw_r2r(mm7, mm6);
				paddw_r2r(mm4, mm2);
				paddw_r2r(mm6, mm3);
				paddw_r2r(mm2, mm0);
				paddw_r2r(mm3, mm1);
				movq_m2r(*twos, mm6);
				paddw_r2r(mm6, mm0); /* round mm0 */
				paddw_r2r(mm6, mm1); /* round mm1 */
				psrlw_i2r(2, mm0);
				psrlw_i2r(2, mm1);
				
				movq_m2r(blk2[0], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm7, mm2);
				punpckhbw_r2r(mm7, mm3);
				
				psubw_r2r(mm2, mm0);
				psubw_r2r(mm3, mm1);
				pmaddwd_r2r(mm0, mm0);
				pmaddwd_r2r(mm1, mm1);
				paddd_r2r(mm1, mm0);
				
				movq_m2r(blk1[8], mm1);
				movq_r2r(mm1, mm2);
				punpcklbw_r2r(mm7, mm1);
				punpckhbw_r2r(mm7, mm2);
				
				movq_m2r(blk1[9], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
				
				paddw_r2r(mm3, mm1);
				paddw_r2r(mm4, mm2);
				
				movq_m2r(blk1[lx+8], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
				paddw_r2r(mm3, mm1);
				paddw_r2r(mm4, mm2);
				
				movq_m2r(blk1[lx+9], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
				
				paddw_r2r(mm3, mm1);
				paddw_r2r(mm4, mm2);
				
				movq_m2r(*twos, mm6);
				paddw_r2r(mm6, mm1);
				paddw_r2r(mm6, mm2);
				
				psrlw_i2r(2, mm1);
				psrlw_i2r(2, mm2);
				
				movq_m2r(blk2[8], mm3);
				movq_r2r(mm3, mm4);
				punpcklbw_r2r(mm7, mm3);
				punpckhbw_r2r(mm7, mm4);
				
				psubw_r2r(mm3, mm1);
				psubw_r2r(mm4, mm2);
				pmaddwd_r2r(mm1, mm1);
				pmaddwd_r2r(mm2, mm2);
				paddd_r2r(mm2, mm1);
				
				paddd_r2r(mm1, mm0);
				
				paddd_r2r(mm0, mm5); 
				
				blk1 += lx;
				blk2 += lx;

				h--;
			} while (h);
		}		
	
	}
	
	movd_r2g(mm5, sum1);
	psrlq_i2r(32, mm5);
	movd_r2g(mm5, sum2);
	sum = sum1 + sum2;
	
	emms();
	
	return sum;
}


/*
 *  total squared difference between two (8*h) blocks
 *  blk1,blk2: addresses of top left pels of both blocks
 *  lx:        distance (in bytes) of vertically adjacent pels
 *  h:         height of block (usually 4, or 8)
 *  mmX version
 */

int sumsq_sub22_mmx(uint8_t *blk1, uint8_t *blk2, int lx, int h)
{
	int sum,sum1,sum2;

	pxor_r2r(mm5, mm5); /* sum */
	
	if (h > 0)
	{
		pxor_r2r(mm7, mm7);
		
		do { /* d2top22 */
			
			movq_m2r(blk1[0], mm0);
			movq_r2r(mm0, mm1);
			punpcklbw_r2r(mm7, mm0);
			punpckhbw_r2r(mm7, mm1);
			
			movq_m2r(blk2[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			
			psubw_r2r(mm2, mm0);
			psubw_r2r(mm3, mm1);
			pmaddwd_r2r(mm0, mm0);
			pmaddwd_r2r(mm1, mm1);
			paddd_r2r(mm0, mm5);
			paddd_r2r(mm1, mm5);
			
			blk1 += lx;
			blk2 += lx;
			
			h--;
		} while (h);
	}
		
	movd_r2g(mm5, sum1);
	psrlq_i2r(32, mm5);
	movd_r2g(mm5, sum2);
	sum = sum1 + sum2;

	emms();
	
	return sum;
}


/*
 *  total squared difference between interpolation of two (8*h) blocks and
 *  another 8*h block		
 *  blk1,blk2: addresses of top left pels of both blocks
 *  lx:        distance (in bytes) of vertically adjacent pels
 *  h:         height of block (usually 4, or 8)
 *  mmX version
 */

int bsumsq_sub22_mmx(uint8_t *blk1f, uint8_t *blk1b, uint8_t *blk2, int lx, int h)
{
	static const uint16_t ones[4]={1,1,1,1};
	int sum,sum1,sum2;

	pxor_r2r(mm5, mm5);
	
	if (h > 0)
	{
		pxor_r2r(mm7, mm7);
		
		do { /* bd2top22 */
			movq_m2r(blk1f[0], mm0);
			movq_r2r(mm0, mm1);
				movq_m2r(blk1b[0], mm4);
				movq_r2r(mm4, mm6);
			punpcklbw_r2r(mm7, mm0);
			punpckhbw_r2r(mm7, mm1);
				punpcklbw_r2r(mm7, mm4);
				punpckhbw_r2r(mm7, mm6);
			
			movq_m2r(blk2[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			
				paddw_r2r(mm4, mm0);
				paddw_m2r(*ones, mm0);
				psrlw_i2r(1, mm0);
			psubw_r2r(mm2, mm0);
			pmaddwd_r2r(mm0, mm0);
				paddw_r2r(mm6, mm1);
				paddw_m2r(*ones, mm1);
				psrlw_i2r(1, mm1);
			psubw_r2r(mm3, mm1);
			pmaddwd_r2r(mm1, mm1);
			paddd_r2r(mm0, mm5);
				paddd_r2r(mm1, mm5);
			
			blk1f += lx;
			blk1b += lx;
			blk2 += lx;
		
			h--;
		} while (h);
	}
	

	movd_r2g(mm5, sum1);
	psrlq_i2r(32, mm5);
	movd_r2g(mm5, sum2);
	sum = sum1 + sum2;

	emms();
	
	return sum;

}


/*
 *   variance of a (size*size) block, multiplied by 256
 *  p:  address of top left pel of block
 *  lx: seperation (in bytes) of vertically adjacent pels
 *  NOTE:		 size  is 8 or 16
 */

void variance_mmx(uint8_t *p, int size,	int lx, unsigned int *p_var, unsigned int *p_mean)
{
	uint8_t *p2;
	int col, row;
	unsigned int sum, sum_squared, squares_sum;
		
	 	/*
 		 *   Use of MMX registers
 		 *   mm0 = <0,0,0,0>
 		 *   mm6 = Sum pixel squared
 		 *   mm7 = Sum pixels
 		 */

	pxor_r2r(mm0, mm0);
	pxor_r2r(mm7, mm7); /* Zero sum accumulator (4 words)  */
	pxor_r2r(mm6, mm6); /* Zero squares accumulator (2 dwords) */

	for(col=0; col<size; col+=8)
	{	
		p2 = p;	
		for (row=0; row<size; row++) /* varrows */
		{ 
			movq_m2r(p2[col], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm0, mm2);
			punpckhbw_r2r(mm0, mm3);
			
			movq_r2r(mm2, mm4);
			movq_r2r(mm3, mm5);
			
			pmaddwd_r2r(mm2, mm4); /*  Squares 0:3                        */
			paddw_r2r(mm2, mm7);   /* Accumulate sum 0:3 (words)          */
			paddd_r2r(mm4, mm6);   /* Accumulate sum squares 0:3 (dwords) */

			pmaddwd_r2r(mm3, mm5); /*  Squares 4:7                        */
			paddw_r2r(mm3, mm7);   /* Accumulate sum 4:7 (words)          */
			paddd_r2r(mm5, mm6);   /* Accumulate sum squares 4:7 (dwords) */
	
			p2 += lx; /* Next row */
		} 
	} 

	movq_r2r(mm7, mm1);
	psrlq_i2r(32, mm1);
	paddw_r2r(mm1, mm7);
	
	movq_r2r(mm7, mm1);
	psrlq_i2r(16, mm1);
	paddw_r2r(mm1, mm7);
	movd_r2g(mm7, sum);
	sum &= 0xffff;
	sum_squared = sum*sum;
	
	movq_r2r(mm6, mm1);
	psrlq_i2r(32, mm1);
	paddd_r2r(mm1, mm6);
	movd_r2g(mm6, squares_sum);
	
		/* sum / (size*size) -> *p_mean                        */
		/*  Squares sum - sum squares / (size*size) -> *p_var  */
	
	sum >>= 6;    		/* Divide  sum and sum squared by 64 for 8*8 */
	sum_squared >>= 6;  		
	if (size != 8)		/* If 16 * 16 divide again by 4 (256) */
	{
		sum >>= 2;
		sum_squared >>= 2;  	
	}
		
	*p_mean = sum;
	*p_var = squares_sum-sum_squared;

	emms();
}

#endif

