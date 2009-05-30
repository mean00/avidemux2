/* 
 *   mblock_sad_mmxe.s:  
 *
 *
 *     Enhanced MMX optimized Sum Absolute Differences routines for macroblocks
 *     (interpolated, 1-pel, 2*2 sub-sampled pel and 4*4 sub-sampled pel)
 * 
 *   sad_* Original Copyright (C) 2000 Chris Atenasio <chris@crud.net>
 *   Enhancements and rest Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
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
 */

#include <ADM_default.h>
#include "mjpeg_types.h"
#ifdef HAVE_X86CPU
#include "mmx.h"


int sad_00_mmx(uint8_t *blk1,uint8_t *blk2,int lx,int h, int distlim)
{
 	int rv;
/*
 *  N.b. distlim is *ignored* as testing for it is more expensive than the
 *  occasional saving by aborting the computionation early...
 * 
 *  mm0 = distance accumulators (4 words)
 *  mm1 = temp 
 *  mm2 = temp 
 *  mm3 = temp
 *  mm4 = temp
 *  mm5 = temp 
 *  mm6 = 0
 *  mm7 = temp
 */

	pxor_r2r(mm0, mm0);
	pxor_r2r(mm6, mm6);
	
	do {
		movq_m2r(blk1[0], mm4); /* load first 8 bytes of p1 row  */
		movq_m2r(blk2[0], mm5); /* load first 8 bytes of p2 row  */
		
		movq_r2r(mm4, mm7); 	/*  mm5 = abs(mm4-mm5) */
		psubusb_r2r(mm5, mm7);
		psubusb_r2r(mm4, mm5);
		paddb_r2r(mm7, mm5);
		
		/* Add the abs(mm4-mm5) bytes to the accumulators */
		movq_m2r(blk1[8], mm2); /* load second 8 bytes of p1 row (interleaved) */
		movq_r2r(mm5, mm7);	/* mm7 := [i :	B0..3, mm1]W */
		punpcklbw_r2r(mm6, mm7);
		movq_m2r(blk2[8], mm3);
		paddw_r2r(mm7, mm0);
		punpckhbw_r2r(mm6, mm5);
		paddw_r2r(mm5, mm0);
		
			/* This is logically where the mm2, mm3 loads would go... */
		
		movq_r2r(mm2, mm7); 	/* mm3 = abs(mm2-mm3) */
		psubusb_r2r(mm3, mm7);
		psubusb_r2r(mm2, mm3);
		paddb_r2r(mm7, mm3);
		
		/* Add the abs(mm4-mm5) bytes to the accumulators */
		movq_r2r(mm3, mm7);
		punpcklbw_r2r(mm6, mm7);
		punpckhbw_r2r(mm6, mm3);
		paddw_r2r(mm7, mm0);
		
		blk1 += lx; /* update pointers to next row */
		blk2 += lx;
		
		paddw_r2r(mm3, mm0);
		
		h--;
	} while(h);
	
	/* Sum the Accumulators */
	movq_r2r(mm0, mm5);	/*  mm5 := [W0+W2,W1+W3, mm0 */
	psrlq_i2r(32, mm5);
	movq_r2r(mm0, mm4);
	paddw_r2r(mm5, mm4);
	
	movq_r2r(mm4, mm7);	/* mm6 := [W0+W2+W1+W3, mm0] */
	psrlq_i2r(16, mm7);
	paddw_r2r(mm7, mm4);
	movd_r2g(mm4, rv);	/* store return value */
	rv &= 0xffff;

	emms();

	return rv;
}



/*
 *        sad_01_mmx.s:  mmx1 optimised 7bit*8 word absolute difference sum
 *        We're reduce to seven bits as otherwise we also have to mess
 *        horribly with carries and signed only comparisons make the code
 *        simply enormous (and probably barely faster than a simple loop).
 *        Since signals with a bona-fide 8bit res will be rare we simply
 *        take the precision hit...
 *        Actually we don't worry about carries from the low-order bits
 *        either so 1/4 of the time we'll be 1 too low...
 *  
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
 */

int sad_01_mmx(uint8_t *p1, uint8_t *p2, int lx, int h)
{
 	int rv;

/*
 *  mm0 = distance accumulators (4 words)
 *  mm1 = bytes p2
 *  mm2 = bytes p1
 *  mm3 = bytes p1+1
 *  mm4 = temp 4 bytes in words interpolating p1, p1+1
 *  mm5 = temp 4 bytes in words from p2
 *  mm6 = temp comparison bit mask p1,p2
 *  mm7 = temp comparison bit mask p2,p1
 */
 	pxor_r2r(mm0, mm0);
 

	do {
		/* First 8 bytes of row */
		
		/* First 4 bytes of 8 */
		
		movq_m2r(p1[0], mm4); 		/* mm4 := first 4 bytes p1 */
		pxor_r2r(mm7, mm7);
		movq_r2r(mm4, mm2);		/* mm2 records all 8 bytes */
		punpcklbw_r2r(mm7, mm4);	/* First 4 bytes p1 in Words... */
		
		movq_m2r(p1[1], mm6);		/* mm6 := first 4 bytes p1+1 */
		movq_r2r(mm6, mm3);		/* mm3 records all 8 bytes */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 bytes interpolated in words */
		psrlw_i2r(1, mm4);
		
		movq_m2r(p2[0], mm5);		/* mm5:=first 4 bytes of p2 in words */
		movq_r2r(mm5, mm1);
		punpcklbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/*  Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/*  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/*  Add to accumulator */
	
		/* Second 4 bytes of 8 */
		
		movq_r2r(mm2, mm4);		/* mm4 := Second 4 bytes p1 in words */
		pxor_r2r(mm7, mm7);
		punpckhbw_r2r(mm7, mm4);
		movq_r2r(mm3, mm6);		/*  mm6 := Second 4 bytes p1+1 in words */
		punpckhbw_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 Interpolated bytes in words */
		psrlw_i2r(1, mm4);
		
		movq_r2r(mm1, mm5);		/* mm5:= second 4 bytes of p2 in words */
		punpckhbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/*  Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/*  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
		

		/* Second 8 bytes of row */
		
		/* First 4 bytes of 8 */
		
		movq_m2r(p1[8], mm4); 		/* mm4 := first 4 bytes p1+8 */
		pxor_r2r(mm7, mm7);
		movq_r2r(mm4, mm2);		/* mm2 records all 8 bytes */
		punpcklbw_r2r(mm7, mm4);	/* First 4 bytes p1 in Words... */
		
		movq_m2r(p1[9], mm6);		/* mm6 := first 4 bytes p1+9 */
		movq_r2r(mm6, mm3);		/* mm3 records all 8 bytes */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 bytes interpolated in words */
		psrlw_i2r(1, mm4);
		
		movq_m2r(p2[8], mm5);		/* mm5:=first 4 bytes of p2+8 in words */
		movq_r2r(mm5, mm1);
		punpcklbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/*  Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/*  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/*  Add to accumulator */
	
		/* Second 4 bytes of 8 */
		
		movq_r2r(mm2, mm4);		/* mm4 := Second 4 bytes p1 in words */
		pxor_r2r(mm7, mm7);
		punpckhbw_r2r(mm7, mm4);
		movq_r2r(mm3, mm6);		/*  mm6 := Second 4 bytes p1+1 in words */
		punpckhbw_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 Interpolated bytes in words */
		psrlw_i2r(1, mm4);
		
		movq_r2r(mm1, mm5);		/* mm5:= second 4 bytes of p2 in words */
		punpckhbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/*  Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/*  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
		
		
		p1 += lx; /* update pointers to next row */
		p2 += lx;

		h--;
	} while (h);

 	/* Sum the Accumulators */
	movq_r2r(mm0, mm4);
	psrlq_i2r(32, mm4);
	paddw_r2r(mm4, mm0);
	movq_r2r(mm0, mm6);
	psrlq_i2r(16, mm6);
	paddw_r2r(mm6, mm0);
	movd_r2g(mm0, rv);	/* store return value */
	rv &= 0xffff;

	emms();

	return rv;
}


/*
 *     sad_01_mmx.s:  mmx1 optimised 7bit*8 word absolute difference sum
 *        We're reduce to seven bits as otherwise we also have to mess
 *        horribly with carries and signed only comparisons make the code
 *        simply enormous (and probably barely faster than a simple loop).
 *        Since signals with a bona-fide 8bit res will be rare we simply
 *        take the precision hit...
 *        Actually we don't worry about carries from the low-order bits
 *        either so 1/4 of the time we'll be 1 too low...
 *  
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
 */

int sad_10_mmx(uint8_t *p1, uint8_t *p2, int lx, int h)
{
	int rv;
	
/*
 *  mm0 = distance accumulators (4 words)
 *  mm1 = bytes p2
 *  mm2 = bytes p1
 *  mm3 = bytes p1+1
 *  mm4 = temp 4 bytes in words interpolating p1, p1+1
 *  mm5 = temp 4 bytes in words from p2
 *  mm6 = temp comparison bit mask p1,p2
 *  mm7 = temp comparison bit mask p2,p1
 */
	
	pxor_r2r(mm0, mm0);
	
	do {
		/* First 8 bytes of row */
		
		/* First 4 bytes of 8 */
		
		movq_m2r(p1[0], mm4); 		/* mm4 := first 4 bytes p1 */
		pxor_r2r(mm7, mm7);
		movq_r2r(mm4, mm2);		/* mm2 records all 8 bytes */
		punpcklbw_r2r(mm7, mm4);	/* First 4 bytes p1 in Words... */
		
		movq_m2r(p1[lx], mm6);		/* mm6 := first 4 bytes p1+lx */
		movq_r2r(mm6, mm3);		/* mm3 records all 8 bytes */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 bytes interpolated in words */
		psrlw_i2r(1, mm4);
		
		movq_m2r(p2[0], mm5); 		/* mm5:=first 4 bytes of p2 in words */
		movq_r2r(mm5, mm1);
		punpcklbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/* Add to accumulator */
			
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/* mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */

			
		/* Second 4 bytes of 8 */
	
		movq_r2r(mm2, mm4);		/* mm4 := Second 4 bytes p1 in words */
		pxor_r2r(mm7, mm7);
		punpckhbw_r2r(mm7, mm4);
		movq_r2r(mm3, mm6);		/* mm6 := Second 4 bytes p1+lx in words  */
		punpckhbw_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 Interpolated bytes in words */
		psrlw_i2r(1, mm4);
		
		movq_r2r(mm1, mm5);		/* mm5:= second 4 bytes of p2 in words */
		punpckhbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/* Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5); 		/* mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
		
		
		/* Second 8 bytes of row */

		/* First 4 bytes of 8 */
		
		movq_m2r(p1[8], mm4); 		/* mm4 := first 4 bytes p1+8 */
		pxor_r2r(mm7, mm7);
		movq_r2r(mm4, mm2);		/* mm2 records all 8 bytes */
		punpcklbw_r2r(mm7, mm4);	/* First 4 bytes p1 in Words... */
		
		movq_m2r(p1[lx+8], mm6);	/* mm6 := first 4 bytes p1+lx+8 */
		movq_r2r(mm6, mm3);		/* mm3 records all 8 bytes */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 bytes interpolated in words */
		psrlw_i2r(1, mm4);
		
		movq_m2r(p2[8], mm5); 		/* mm5:=first 4 bytes of p2+8 in words */
		movq_r2r(mm5, mm1);
		punpcklbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/* Add to accumulator */

		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/* mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
			
		/* Second 4 bytes of 8 */
	
		movq_r2r(mm2, mm4);		/* mm4 := Second 4 bytes p1 in words */
		pxor_r2r(mm7, mm7);
		punpckhbw_r2r(mm7, mm4);
		movq_r2r(mm3, mm6);		/* mm6 := Second 4 bytes p1+lx in words  */
		punpckhbw_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm4);		/* mm4 := First 4 Interpolated bytes in words */
		psrlw_i2r(1, mm4);
		
		movq_r2r(mm1, mm5);		/* mm5:= second 4 bytes of p2 in words */
		punpckhbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/* Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/* mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
		
		p1 += lx;
		p2 += lx;

		h--;
	} while (h);
	
 	/* Sum the Accumulators */
	movq_r2r(mm0, mm4);
	psrlq_i2r(32, mm4);
	paddw_r2r(mm4, mm0);
	movq_r2r(mm0, mm6);
	psrlq_i2r(16, mm6);
	paddw_r2r(mm6, mm0);
	movd_r2g(mm0, rv);	/* store return value */
	rv &= 0xffff;

	emms();

	return rv;
}



/*
 *     sad_01_mmx.s:  
 *  
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
 */

int sad_11_mmx(uint8_t *p1, uint8_t *p2, int lx, int h)
{
	int rv;

/*
 *  mm0 = distance accumulators (4 words)
 *  mm1 = bytes p2
 *  mm2 = bytes p1
 *  mm3 = bytes p1+lx
 *  I'd love to find someplace to stash p1+1 and p1+lx+1's bytes
 *  but I don't think thats going to happen in iA32-land...
 *  mm4 = temp 4 bytes in words interpolating p1, p1+1
 *  mm5 = temp 4 bytes in words from p2
 *  mm6 = temp comparison bit mask p1,p2
 *  mm7 = temp comparison bit mask p2,p1
 */

	pxor_r2r(mm0, mm0);
	
	do {
	
		/*  First 8 bytes of row */
		
		/*  First 4 bytes of 8 */
	
		movq_m2r(p1[0], mm4);		/* mm4 := first 4 bytes p1 */
		pxor_r2r(mm7, mm7);
		movq_r2r(mm4, mm2);		/* mm2 records all 8 bytes */
		punpcklbw_r2r(mm7, mm4);	/* First 4 bytes p1 in Words... */
		
		movq_m2r(p1[lx], mm6);		/* mm6 := first 4 bytes p1+lx */
		movq_r2r(mm6, mm3);		/*  mm3 records all 8 bytes */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		movq_m2r(p1[1], mm5);		/* mm5 := first 4 bytes p1+1 */
		punpcklbw_r2r(mm7, mm5);	/* First 4 bytes p1 in Words... */
		paddw_r2r(mm5, mm4);
		movq_m2r(p1[lx+1], mm6);	/* mm6 := first 4 bytes p1+lx+1 */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		psrlw_i2r(2, mm4);		/*  mm4 := First 4 bytes interpolated in words */
		
		movq_m2r(p2[0], mm5);		/* mm5:=first 4 bytes of p2 in words */
		movq_r2r(mm5, mm1);
		punpcklbw_r2r(mm7, mm5);

		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);

		paddw_r2r(mm6, mm0);		/* Add to accumulator */

		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5); 		/* mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
		
		/* Second 4 bytes of 8 */
		
		movq_r2r(mm2, mm4);		/* mm4 := Second 4 bytes p1 in words */
		pxor_r2r(mm7, mm7);
		punpckhbw_r2r(mm7, mm4);
		movq_r2r(mm3, mm6);		/*  mm6 := Second 4 bytes p1+1 in words */
		punpckhbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		movq_m2r(p1[1], mm5);		/*  mm5 := first 4 bytes p1+1 */
		punpckhbw_r2r(mm7, mm5);	/* First 4 bytes p1 in Words... */
		paddw_r2r(mm5, mm4);
		movq_m2r(p1[lx+1], mm6);	/* mm6 := first 4 bytes p1+lx+1 */
		punpckhbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		psrlw_i2r(2, mm4);		/* mm4 := First 4 bytes interpolated in words */
		
		movq_r2r(mm1, mm5);		/* mm5:= second 4 bytes of p2 in words */
		punpckhbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);		
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/* Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/*  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
		
		
		/* Second 8 bytes of row */

		/*  First 4 bytes of 8 */
	
		movq_m2r(p1[8], mm4);		/* mm4 := first 4 bytes p1+8 */
		pxor_r2r(mm7, mm7);
		movq_r2r(mm4, mm2);		/* mm2 records all 8 bytes */
		punpcklbw_r2r(mm7, mm4);	/* First 4 bytes p1 in Words... */
		
		movq_m2r(p1[lx+8], mm6);	/* mm6 := first 4 bytes p1+lx+8 */
		movq_r2r(mm6, mm3);		/*  mm3 records all 8 bytes */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		movq_m2r(p1[9], mm5);		/* mm5 := first 4 bytes p1+9 */
		punpcklbw_r2r(mm7, mm5);	/* First 4 bytes p1 in Words... */
		paddw_r2r(mm5, mm4);
		movq_m2r(p1[lx+9], mm6);	/* mm6 := first 4 bytes p1+lx+9 */
		punpcklbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		psrlw_i2r(2, mm4);		/*  mm4 := First 4 bytes interpolated in words */
		
		movq_m2r(p2[8], mm5);		/* mm5:=first 4 bytes of p2+8 in words */
		movq_r2r(mm5, mm1);
		punpcklbw_r2r(mm7, mm5);

		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);
		pand_r2r(mm7, mm6);

		paddw_r2r(mm6, mm0);		/* Add to accumulator */

		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5); 		/* mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */
		
		/* Second 4 bytes of 8 */
		
		movq_r2r(mm2, mm4);		/* mm4 := Second 4 bytes p1 in words */
		pxor_r2r(mm7, mm7);
		punpckhbw_r2r(mm7, mm4);
		movq_r2r(mm3, mm6);		/*  mm6 := Second 4 bytes p1+1 in words */
		punpckhbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		movq_m2r(p1[9], mm5);		/*  mm5 := first 4 bytes p1+9 */
		punpckhbw_r2r(mm7, mm5);	/* First 4 bytes p1 in Words... */
		paddw_r2r(mm5, mm4);
		movq_m2r(p1[lx+9], mm6);	/* mm6 := first 4 bytes p1+lx+9 */
		punpckhbw_r2r(mm7, mm6);
		paddw_r2r(mm6, mm4);
		
		psrlw_i2r(2, mm4);		/* mm4 := First 4 bytes interpolated in words */
		
		movq_r2r(mm1, mm5);		/* mm5:= second 4 bytes of p2 in words */
		punpckhbw_r2r(mm7, mm5);
		
		movq_r2r(mm4, mm7);
		pcmpgtw_r2r(mm5, mm7);		/* mm7 := [i : W0..3,mm4>mm5] */
		
		movq_r2r(mm4, mm6);		/* mm6 := [i : W0..3, (mm4-mm5)*(mm4-mm5 > 0)] */
		psubw_r2r(mm5, mm6);		
		pand_r2r(mm7, mm6);
		
		paddw_r2r(mm6, mm0);		/* Add to accumulator */
		
		movq_r2r(mm5, mm6);		/* mm6 := [i : W0..3,mm5>mm4] */
		pcmpgtw_r2r(mm4, mm6);
		psubw_r2r(mm4, mm5);		/*  mm5 := [i : B0..7, (mm5-mm4)*(mm5-mm4 > 0)] */
		pand_r2r(mm6, mm5);
		
		paddw_r2r(mm5, mm0);		/* Add to accumulator */			
		
		p1 += lx;	/* update pointers to next row */
		p2 += lx;
	
		h--;
	} while (h);

 	/* Sum the Accumulators */
	movq_r2r(mm0, mm4);
	psrlq_i2r(32, mm4);
	paddw_r2r(mm4, mm0);
	movq_r2r(mm0, mm6);
	psrlq_i2r(16, mm6);
	paddw_r2r(mm6, mm0);
	movd_r2g(mm0, rv);	/* store return value */
	rv &= 0xffff;

	return rv;
}



int sad_sub22_mmx(uint8_t *blk1,uint8_t *blk2,int lx,int h)
{
	int rv;

/*
 *  mm0 = distance accumulators (4 words)
 *  mm1 = temp 
 *  mm2 = temp 
 *  mm3 = temp
 *  mm4 = temp
 *  mm5 = temp 
 *  mm6 = 0
 *  mm7 = temp
 */

	pxor_r2r(mm0, mm0);
	pxor_r2r(mm6, mm6);

	do {
		movq_m2r(blk1[0], mm4);		/* load 8 bytes of p1 */
		movq_m2r(blk2[0], mm5);		/* load 8 bytes of p2 */
		
		movq_r2r(mm4, mm7);		/* mm5 = abs(*p1-*p2) */
		psubusb_r2r(mm5, mm7);
		psubusb_r2r(mm4, mm5);
		blk1 += lx;			/* update pointer to next row */
		paddb_r2r(mm7, mm5);
		
			/* Add the mm5 bytes to the accumulatores */
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm6, mm7);
		paddw_r2r(mm7, mm0);
		punpckhbw_r2r(mm6, mm5);
		blk2 += lx;			/* update pointer to next row */
		paddw_r2r(mm5, mm0);
		
		movq_m2r(blk1[0], mm4);		/* load 8 bytes of p1 (next row) */
		movq_m2r(blk2[0], mm5);		/* load 8 bytes of p2 (next row) */
		
		movq_r2r(mm4, mm7);		/* mm5 = abs(*p1-*p2) */
		psubusb_r2r(mm5, mm7);
		psubusb_r2r(mm4, mm5);
		blk1 += lx;			/* update pointer to next row */
		paddb_r2r(mm7, mm5);
		
			/* Add the mm5 bytes to the accumulatores */
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm6, mm7);
		blk2 += lx;			/* update pointer to next row */
		paddw_r2r(mm7, mm0);
		punpckhbw_r2r(mm6, mm5);
		h -= 2;
		paddw_r2r(mm5, mm0);
	} while (h);

 	/* Sum the Accumulators */
	movq_r2r(mm0, mm1);
	psrlq_i2r(16, mm1);
	movq_r2r(mm0, mm2);
	psrlq_i2r(32, mm2);
	movq_r2r(mm0, mm3);
	psrlq_i2r(48, mm3);
	paddw_r2r(mm1, mm0);
	paddw_r2r(mm3, mm2);
	paddw_r2r(mm2, mm0);
		
	movd_r2g(mm0, rv);	/* store return value */
	rv &= 0xffff;

	emms();

	return rv;
}


int sad_sub44_mmx(uint8_t *blk1, uint8_t *blk2, int qlx, int qh)
{
	int rv;

/*
 *  mm0 = distance accumulator left block p1
 *  mm1 = distance accumulator right block p1
 *  mm2 = 0
 *  mm3 = right block of p1
 *  mm4 = left block of p1
 *  mm5 = p2
 *  mm6 = temp
 *  mm7 = temp
 */

	pxor_r2r(mm0, mm0);
	pxor_r2r(mm1, mm1);
	pxor_r2r(mm2, mm2);
	
	do {

		/*
		 *  Beware loop obfuscated by interleaving to try to
		 *  hide latencies...
		 */

		movq_m2r(blk1[0], mm4);		/* mm4 =  first 4 bytes of p1 in words */
		movq_m2r(blk2[0], mm5);		/* mm5 = 4 bytes of p2 in words */
		movq_r2r(mm4, mm3);
		punpcklbw_r2r(mm2, mm4);
		punpcklbw_r2r(mm2, mm5);

		movq_r2r(mm4, mm7);
		movq_r2r(mm5, mm6);
		psubusw_r2r(mm5, mm7);
		psubusw_r2r(mm4, mm6);
		
		blk1 += qlx;	/* update a pointer to next row */
		
		paddw_r2r(mm6, mm7);
		paddw_r2r(mm7, mm0); 	/* Add absolute differences to left block accumulators */
			
		blk2 += qlx;
		qh--;
			
	} while (qh);
	
	
	/* Sum the accumulators */
	movq_r2r(mm0, mm4);
	psrlq_i2r(32, mm4);
	paddw_r2r(mm4, mm0);
	movq_r2r(mm0, mm6);
	psrlq_i2r(16, mm6);
	paddw_r2r(mm6, mm0);
	movd_r2g(mm0, rv);
	rv &= 0xffff;
	
	emms();

	return rv;
}
#endif

