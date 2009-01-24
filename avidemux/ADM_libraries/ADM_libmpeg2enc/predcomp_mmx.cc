/*
 *   predcomp_00_mmx.s:
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
#include "mjpeg_types.h"
#ifdef HAVE_X86CPU
#include "mmx.h"


/* 
 * void predcomp_<ix><iy>_mmx(char *src,char *dst,int lx, int w, int h, int addflag);
 * 
 * ix - Interpolation in x iy - Interpolation in y
 * 		
 */


/* The no interpolation case... */

void predcomp_00_mmx(char *src,char *dst,int lx, int w, int h, int addflag)
{

/*
 *  mm1 = one's mask for src
 *  mm0 = zero mask for src...
 */

	movd_g2r(0x00010001, mm1);
	punpckldq_r2r(mm1, mm1);

	pxor_r2r(mm0, mm0);

	
	do {
		movq_m2r(src[0], mm4); /* first 8 bytes of row */
		if (addflag)
		{
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm0, mm4);	
			punpckhbw_r2r(mm0, mm5);
		
			movq_m2r(dst[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm0, mm2);	
			punpckhbw_r2r(mm0, mm3);
			paddw_r2r(mm2, mm4);
			paddw_r2r(mm3, mm5);
			paddw_r2r(mm1, mm4);
			paddw_r2r(mm1, mm5);
			psrlw_i2r(1, mm4);
			psrlw_i2r(1, mm5);
			packuswb_r2r(mm5, mm4);
		}
		
		movq_r2m(mm4, dst[0]);
		
		if (w != 8)
		{
			movq_m2r(src[8], mm4); /* first 8 bytes of row */
			if (addflag)
			{
				movq_r2r(mm4, mm5);
				punpcklbw_r2r(mm0, mm4);	
				punpckhbw_r2r(mm0, mm5);
				
				movq_m2r(dst[8], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm0, mm2);	
				punpckhbw_r2r(mm0, mm3);
				paddw_r2r(mm2, mm4);
				paddw_r2r(mm3, mm5);
				paddw_r2r(mm1, mm4);
				paddw_r2r(mm1, mm5);
				psrlw_i2r(1, mm4);
				psrlw_i2r(1, mm5);
				packuswb_r2r(mm5, mm4);
			}
			movq_r2m(mm4, dst[8]);		
		}
		
		dst += lx; /* update pointer to next row */
		src += lx;
		
		h--;	
	} while (h > 0);
	
	emms();
} 

 
/* The x-axis interpolation case... */

void predcomp_10_mmx(char *src,char *dst,int lx, int w, int h, int addflag)
{
	movd_g2r(0x00010001, mm1);
	punpckldq_r2r(mm1, mm1);

	pxor_r2r(mm0, mm0);

	do {
		movq_m2r(src[0], mm4); /* first 8 bytes of row */
		movq_r2r(mm4, mm5);
		punpcklbw_r2r(mm0, mm4);
		punpckhbw_r2r(mm0, mm5);
		movq_m2r(src[1], mm2);
		movq_r2r(mm2, mm3);
		punpcklbw_r2r(mm0, mm2);
		punpckhbw_r2r(mm0, mm3);
		
		paddw_r2r(mm2, mm4); /* Average mm4/mm5 and mm2/mm3 */
		paddw_r2r(mm3, mm5);
		paddw_r2r(mm1, mm4);
		paddw_r2r(mm1, mm5);
		psrlw_i2r(1, mm4);
		psrlw_i2r(1, mm5);
		
		if (addflag)
		{
			movq_m2r(dst[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm0, mm2);
			punpckhbw_r2r(mm0, mm3);
			paddw_r2r(mm2, mm4); 	 /* Average mm4/mm5 and mm2/mm3 */
			paddw_r2r(mm3, mm5);
			paddw_r2r(mm1, mm4);
			paddw_r2r(mm1, mm5);
			psrlw_i2r(1, mm4);
			psrlw_i2r(1, mm5);
		}
		
		packuswb_r2r(mm5, mm4);
		movq_r2m(mm4, dst[0]);
		
		if (w != 8)
		{
			movq_m2r(src[8], mm4); /* first 8 bytes of row */
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm0, mm4);
			punpckhbw_r2r(mm0, mm5);
			movq_m2r(src[9], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm0, mm2);
			punpckhbw_r2r(mm0, mm3);
		
			paddw_r2r(mm2, mm4); /* Average mm4/mm5 and mm2/mm3 */
			paddw_r2r(mm3, mm5);
			paddw_r2r(mm1, mm4);
			paddw_r2r(mm1, mm5);
			psrlw_i2r(1, mm4);
			psrlw_i2r(1, mm5);
		
			if (addflag)
			{
				movq_m2r(dst[8], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm0, mm2);
				punpckhbw_r2r(mm0, mm3);
				paddw_r2r(mm2, mm4); 	 /* Average mm4/mm5 and mm2/mm3 */
				paddw_r2r(mm3, mm5);
				paddw_r2r(mm1, mm4);
				paddw_r2r(mm1, mm5);
				psrlw_i2r(1, mm4);
				psrlw_i2r(1, mm5);
			}
		
			packuswb_r2r(mm5, mm4);
			movq_r2m(mm4, dst[8]);
		}
		
		src += lx;
		dst += lx;
	
		h--;
	} while (h > 0);

	emms();
}


/* The y-axis interpolation case... */

void predcomp_01_mmx(char *src,char *dst,int lx, int w, int h, int addflag)
{
	movd_g2r(0x00010001, mm1);
	punpckldq_r2r(mm1, mm1);

	pxor_r2r(mm0, mm0);

	do {
		movq_m2r(src[0], mm4);	/* first 8 bytes of row */
		movq_r2r(mm4, mm5);
		src += lx;		/* Next row */
		punpcklbw_r2r(mm0, mm4);
		punpckhbw_r2r(mm0, mm5);
	
		movq_m2r(src[0], mm2);	
		movq_r2r(mm2, mm3);
		punpcklbw_r2r(mm0, mm2);
		punpckhbw_r2r(mm0, mm3);

		paddw_r2r(mm2, mm4); /* Average mm4/mm5 and mm2/mm3 */
		paddw_r2r(mm3, mm5);
		paddw_r2r(mm1, mm4);
		paddw_r2r(mm1, mm5);
		psrlw_i2r(1, mm4);
		psrlw_i2r(1, mm5);
			
		if (addflag)
		{
			movq_m2r(dst[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm0, mm2);
			punpckhbw_r2r(mm0, mm3);
			paddw_r2r(mm2, mm4); 	 /* Average mm4/mm5 and mm2/mm3 */
			paddw_r2r(mm3, mm5);
			paddw_r2r(mm1, mm4);
			paddw_r2r(mm1, mm5);
			psrlw_i2r(1, mm4);
			psrlw_i2r(1, mm5);
		}
				
		packuswb_r2r(mm5, mm4);
		movq_r2m(mm4, dst[0]);

		if (w != 8)
		{
			src -= lx; /* Back to first row... */
			movq_m2r(src[8], mm4);	/* first 8 bytes of row */
			movq_r2r(mm4, mm5);
			src += lx;		/* Next row */
			punpcklbw_r2r(mm0, mm4);
			punpckhbw_r2r(mm0, mm5);
	
			movq_m2r(src[8], mm2);	
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm0, mm2);
			punpckhbw_r2r(mm0, mm3);

			paddw_r2r(mm2, mm4); /* Average mm4/mm5 and mm2/mm3 */
			paddw_r2r(mm3, mm5);
			paddw_r2r(mm1, mm4);
			paddw_r2r(mm1, mm5);
			psrlw_i2r(1, mm4);
			psrlw_i2r(1, mm5);
			
			if (addflag)
			{
				movq_m2r(dst[8], mm2);
				movq_r2r(mm2, mm3);
				punpcklbw_r2r(mm0, mm2);
				punpckhbw_r2r(mm0, mm3);
				paddw_r2r(mm2, mm4); 	 /* Average mm4/mm5 and mm2/mm3 */
				paddw_r2r(mm3, mm5);
				paddw_r2r(mm1, mm4);
				paddw_r2r(mm1, mm5);
				psrlw_i2r(1, mm4);
				psrlw_i2r(1, mm5);
			}
				
			packuswb_r2r(mm5, mm4);
			movq_r2m(mm4, dst[8]);
		}	
	
		dst += lx;
	
		h--;
	} while (h > 0);

	emms();
}


/* The x-axis and y-axis interpolation case... */

void predcomp_11_mmx(char *src,char *dst,int lx, int w, int h, int addflag)
{

/*
 *  mm0 = [0,0,0,0]W
 *  mm1 = [1,1,1,1]W		
 *  mm2 = [2,2,2,2]W
 */
	movd_g2r(0x00020002, mm2);
	punpckldq_r2r(mm2, mm2);
	movd_g2r(0x00010001, mm1);
	punpckldq_r2r(mm1, mm1);
	pxor_r2r(mm0, mm0);

	do {
		movq_m2r(src[0], mm4);	/* mm4 and mm6 accumulate partial sums for interp. */
		movq_r2r(mm4, mm6);
		punpcklbw_r2r(mm0, mm4);
		punpckhbw_r2r(mm0, mm6);

		movq_m2r(src[1], mm5);
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm0, mm5);
		paddw_r2r(mm5, mm4);
		punpckhbw_r2r(mm0, mm7);
		paddw_r2r(mm7, mm6);
		
		src += lx;		/* update pointer to next row */
		
		movq_m2r(src[0], mm5); 	/* first 8 bytes 1st row:	 avg src in x */
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm0, mm5); /*  Accumulate partial interpolation */
		paddw_r2r(mm5, mm4);
		punpckhbw_r2r(mm0, mm7);
		paddw_r2r(mm7, mm6);

		movq_m2r(src[1], mm5);
		movq_r2r(mm5, mm7);
		punpcklbw_r2r(mm0, mm5);
		paddw_r2r(mm5, mm4);
		punpckhbw_r2r(mm0, mm7);
		paddw_r2r(mm7, mm6);
		
		/* Now round */
		paddw_r2r(mm2, mm4);
		paddw_r2r(mm2, mm6);
		psrlw_i2r(2, mm4);
		psrlw_i2r(2, mm6);

		if (addflag)
		{
			movq_m2r(dst[0], mm5);
			movq_r2r(mm5, mm7);
			punpcklbw_r2r(mm0, mm5);
			punpckhbw_r2r(mm0, mm7);
			paddw_r2r(mm5, mm4); 	 /* Average mm4/mm6 and mm5/mm7 */
			paddw_r2r(mm7, mm6);
			paddw_r2r(mm1, mm4);
			paddw_r2r(mm1, mm6);
			psrlw_i2r(1, mm4);
			psrlw_i2r(1, mm6);
		}

		packuswb_r2r(mm6, mm4);
		movq_r2m(mm4, dst[0]);

		if (w != 8)
		{
			src -= lx; /* Back to first row... */
		
			movq_m2r(src[8], mm4);	/* mm4 and mm6 accumulate partial sums for interp. */
			movq_r2r(mm4, mm6);
			punpcklbw_r2r(mm0, mm4);
			punpckhbw_r2r(mm0, mm6);

			movq_m2r(src[9], mm5);
			movq_r2r(mm5, mm7);
			punpcklbw_r2r(mm0, mm5);
			paddw_r2r(mm5, mm4);
			punpckhbw_r2r(mm0, mm7);
			paddw_r2r(mm7, mm6);
		
			src += lx;	/* update pointer to next row */
		
			movq_m2r(src[8], mm5); 	/* first 8 bytes 1st row:	 avg src in x */
			movq_r2r(mm5, mm7);
			punpcklbw_r2r(mm0, mm5); /*  Accumulate partial interpolation */
			paddw_r2r(mm5, mm4);
			punpckhbw_r2r(mm0, mm7);
			paddw_r2r(mm7, mm6);

			movq_m2r(src[9], mm5);
			movq_r2r(mm5, mm7);
			punpcklbw_r2r(mm0, mm5);
			paddw_r2r(mm5, mm4);
			punpckhbw_r2r(mm0, mm7);
			paddw_r2r(mm7, mm6);
		
			/* Now round */
			paddw_r2r(mm2, mm4);
			paddw_r2r(mm2, mm6);
			psrlw_i2r(2, mm4);
			psrlw_i2r(2, mm6);

			if (addflag)
			{
				movq_m2r(dst[8], mm5);
				movq_r2r(mm5, mm7);
				punpcklbw_r2r(mm0, mm5);
				punpckhbw_r2r(mm0, mm7);
				paddw_r2r(mm5, mm4); 	 /* Average mm4/mm6 and mm5/mm7 */
				paddw_r2r(mm7, mm6);
				paddw_r2r(mm1, mm4);
				paddw_r2r(mm1, mm6);
				psrlw_i2r(1, mm4);
				psrlw_i2r(1, mm6);
			}

			packuswb_r2r(mm6, mm4);
			movq_r2m(mm4, dst[8]);
		}
	
		dst += lx;
	
		h--;
	} while (h > 0);
	
	emms();
}
#endif

