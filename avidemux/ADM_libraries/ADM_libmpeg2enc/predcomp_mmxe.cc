/*
 *   predcomp_00_mmxe.s:
 * 
 *  		  Extended MMX prediction composition
 *   routines handling the four different interpolation cases...
 *  
 *   Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
 * 
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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *   02111-1307, USA.
 */
 
#include <config.h>
#ifdef HAVE_X86CPU
#include "mjpeg_types.h"
#include "mmx.h"

/* 
 * void predcomp_<ix><iy>_mmxe(char *src,char *dst,int lx, int w, int h, int mask);
 * 
 * ix - Interpolation in x iy - Interpolation in y
 * 		
 */


/* The no interpolation case... */

void predcomp_00_mmxe(char *src,char *dst,int lx, int w, int h, int mask)
{
	movd_g2r(mask, mm0);
	pxor_r2r(mm2, mm2);
	punpckldq_r2r(mm0, mm0);
	pcmpeqd_r2r(mm2, mm0);
	movq_r2r(mm0, mm1);	
	pcmpeqd_r2r(mm2, mm0);
	
	do {
		movq_m2r(*src, mm4); /* first 8 bytes of row */
		movq_m2r(*dst, mm2);
		pand_r2r(mm0, mm2);
		movq_r2r(mm4, mm3);
		pand_r2r(mm1, mm3);
		por_r2r(mm3, mm2);
		pavgb_r2r(mm2, mm4);
		movq_r2m(mm4, *dst);
		
		if (w != 8)
		{
			movq_m2r(*(src+8), mm4); /* first 8 bytes of row */
			movq_m2r(*(dst+8), mm2);
			pand_r2r(mm0, mm2);
			movq_r2r(mm4, mm3);
			pand_r2r(mm1, mm3);
			por_r2r(mm3, mm2);
			pavgb_r2r(mm2, mm4);
			movq_r2m(mm4, *(dst+8));		
		}

		dst += lx;
		src += lx;
		
		h--;	
	} while (h);
	
	emms();
}


/* The x-axis interpolation case... */

void predcomp_10_mmxe(char *src,char *dst,int lx, int w, int h, int mask)
{
	movd_g2r(mask, mm0);
	pxor_r2r(mm2, mm2);
	punpckldq_r2r(mm0, mm0);
	pcmpeqd_r2r(mm2, mm0);
	movq_r2r(mm0, mm1);	
	pcmpeqd_r2r(mm2, mm0);
		
	do {
		movq_m2r(*src, mm4); 	  /* first 8 bytes row:	 avg src in x */
		pavgb_m2r(*(src+1), mm4);
		movq_m2r(*dst, mm2);	
		pand_r2r(mm0, mm2);
		movq_r2r(mm4, mm3);
		pand_r2r(mm1, mm3);
		por_r2r(mm3, mm2);
		pavgb_r2r(mm2, mm4); /* combine */
		movq_r2m(mm4, *dst);
		
		if (w != 8)
		{
			movq_m2r(*(src+8), mm4); 	  /* first 8 bytes row:	 avg src in x */
			pavgb_m2r(*(src+9), mm4);
			movq_m2r(*(dst+8), mm2);	
			pand_r2r(mm0, mm2);
			movq_r2r(mm4, mm3);
			pand_r2r(mm1, mm3);
			por_r2r(mm3, mm2);
			pavgb_r2r(mm2, mm4); /* combine */
			movq_r2m(mm4, *(dst+8));		
		}		
		
		dst += lx;
		src += lx;		
		
		h--;
	} while (h);
	
	emms();
}



/* The x-axis and y-axis interpolation case... */
		
void predcomp_11_mmxe(char *src,char *dst,int lx, int w, int h, int mask)
{

	/* mm2 = [0,0,0,0]W */
	/* mm3 = [2,2,2,2]W */

	movd_g2r(0x00020002, mm3);
	punpckldq_r2r(mm3, mm3);
	
	movd_g2r(mask, mm0);	
	pxor_r2r(mm2, mm2);
	punpckldq_r2r(mm0, mm0);
	pcmpeqd_r2r(mm2, mm0);
	movq_r2r(mm0, mm1);
	pcmpeqd_r2r(mm2, mm0);

	do {
		movq_m2r(*src, mm4); /*  mm4 and mm6 accumulate partial sums for interp. */
		movq_r2r(mm4, mm6);
		punpcklbw_r2r(mm2, mm4);
		punpckhbw_r2r(mm2, mm6);
		
		movq_m2r(*(src+1), mm5);
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm2, mm5);
		paddw_r2r(mm5, mm4);
		punpckhbw_r2r(mm2, mm7);
		paddw_r2r(mm7, mm6);

		src += lx; 
		
		movq_m2r(*src, mm5); /* first 8 bytes 1st row:	 avg src in x */
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm2, mm5);
		paddw_r2r(mm5, mm4);
		punpckhbw_r2r(mm2, mm7);
		paddw_r2r(mm7, mm6);

		movq_m2r(*(src+1), mm5);
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm2, mm5);
		paddw_r2r(mm5, mm4);
		punpckhbw_r2r(mm2, mm7);
		paddw_r2r(mm7, mm6);
		
		/* Now round and repack... */
		paddw_r2r(mm3, mm4);
		paddw_r2r(mm3, mm6);
		psrlw_i2r(2, mm4);
		psrlw_i2r(2, mm6);
		packuswb_r2r(mm6, mm4);
		
		movq_m2r(*dst, mm7);
		pand_r2r(mm0, mm7);
		movq_r2r(mm4, mm6);
		pand_r2r(mm1, mm6);
		por_r2r(mm6, mm7);
		pavgb_r2r(mm7, mm4);
		movq_r2m(mm4, *dst);
		
		if (w != 8)
		{
			src -= lx; /* Back to 1st row */

			movq_m2r(*(src+8), mm4); /*  mm4 and mm6 accumulate partial sums for interp. */
			movq_r2r(mm4, mm6);
			punpcklbw_r2r(mm2, mm4);
			punpckhbw_r2r(mm2, mm6);
		
			movq_m2r(*(src+9), mm5);
			movq_r2r(mm5, mm7);
			punpcklbw_r2r(mm2, mm5);
			paddw_r2r(mm5, mm4);
			punpckhbw_r2r(mm2, mm7);
			paddw_r2r(mm7, mm6);

			src += lx; 
		
			movq_m2r(*(src+8), mm5); /* first 8 bytes 1st row:	 avg src in x */
			movq_r2r(mm5, mm7);
			punpcklbw_r2r(mm2, mm5);
			paddw_r2r(mm5, mm4);
			punpckhbw_r2r(mm2, mm7);
			paddw_r2r(mm7, mm6);

			movq_m2r(*(src+9), mm5);
			movq_r2r(mm5, mm7);
			punpcklbw_r2r(mm2, mm5);
			paddw_r2r(mm5, mm4);
			punpckhbw_r2r(mm2, mm7);
			paddw_r2r(mm7, mm6);
		
			/* Now round and repack... */
			paddw_r2r(mm3, mm4);
			paddw_r2r(mm3, mm6);
			psrlw_i2r(2, mm4);
			psrlw_i2r(2, mm6);
			packuswb_r2r(mm6, mm4);
		
			movq_m2r(*(dst+8), mm7);
			pand_r2r(mm0, mm7);
			movq_r2r(mm4, mm6);
			pand_r2r(mm1, mm6);
			por_r2r(mm6, mm7);
			pavgb_r2r(mm7, mm4);
			movq_r2m(mm4, *(dst+8));
		}
		
		dst += lx; 
					
		h--;
	} while (h);
	
	emms();	
}


/* The  y-axis interpolation case... */

void predcomp_01_mmxe(char *src,char *dst,int lx, int w, int h, int mask)
{
	movd_g2r(mask, mm0);	
	pxor_r2r(mm2, mm2);
	punpckldq_r2r(mm0, mm0);
	pcmpeqd_r2r(mm2, mm0);
	movq_r2r(mm0, mm1);
	pcmpeqd_r2r(mm2, mm0);

	do {
		movq_m2r(*src, mm4); 	/* first 8 bytes row */
		src+=lx; 		/*  update pointer to next row */
		pavgb_m2r(*src, mm4); 	/* Average in y */

		movq_m2r(*dst, mm2);
		pand_r2r(mm0, mm2);
		movq_r2r(mm4, mm3);
		pand_r2r(mm1, mm3);
		por_r2r(mm3, mm2);
		pavgb_r2r(mm2, mm4);
		movq_r2m(mm4, *dst);
		
		if (w != 8)
		{
			src-=lx;		  /* Back to prev row */
			movq_m2r(*(src+8), mm4);  /* first 8 bytes row */
			src+=lx; 		  /*  update pointer to next row */
			pavgb_m2r(*(src+8), mm4);  /* Average in y */

			movq_m2r(*(dst+8), mm2);
			pand_r2r(mm0, mm2);
			movq_r2r(mm4, mm3);
			pand_r2r(mm1, mm3);
			por_r2r(mm3, mm2);
			pavgb_r2r(mm2, mm4);
			movq_r2m(mm4, *(dst+8));			
		}

		dst+=lx;  /*  update pointer to next row */

		h--;
	} while (h);
	
	emms();
}
#endif
