/*
 *   bdist2_mmx.s:  MMX optimized bidirectional squared distance sum
 * 
 *   Original believed to be Copyright (C) 2000 Brent Byeler
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

#include <config.h>
#include "mjpeg_types.h"
#ifdef HAVE_X86CPU
#include "mmx.h"


/*
 * squared error between a (16*h) block and a bidirectional
 * prediction
 *
 * p2: address of top left pel of block
 * pf,hxf,hyf: address and half pel flags of forward ref. block
 * pb,hxb,hyb: address and half pel flags of backward ref. block
 * h: height of block
 * lx: distance (in bytes) of vertically adjacent pels in p2,pf,pb
 * mmX version
 */
 
int bsumsq_mmx(uint8_t *pf, uint8_t *pb, uint8_t *p2, int lx, int hxf, int hyf, int hxb, int hyb, int h)
{
	uint8_t *pfa,*pfb,*pfc,*pba,*pbb,*pbc;
	int s,s1,s2;
	
	pfa = pf + hxf;
	pfb = pf + lx * hyf;
	pfc = pfb + hxf;

	pba = pb + hxb;
	pbb = pb + lx * hyb; 
	pbc = pbb + hxb;
	
	s = 0; /* (accumulated sum) */
	
	if (h > 0)
	{
		pxor_r2r(mm7, mm7);
		pxor_r2r(mm6, mm6);
		pcmpeqw_r2r(mm5, mm5);
		psubw_r2r(mm5, mm6);
		psllw_i2r(1, mm6);
		
		do {
			movq_m2r(pf[0], mm0);
			movq_r2r(mm0, mm1);
			punpcklbw_r2r(mm7, mm0);
			punpckhbw_r2r(mm7, mm1);
			movq_m2r(pfa[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			movq_m2r(pfb[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			movq_m2r(pfc[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			paddw_r2r(mm6, mm0);
			paddw_r2r(mm6, mm1);									
			psrlw_i2r(2, mm0);
			psrlw_i2r(2, mm1);
			
			movq_m2r(pb[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			movq_m2r(pba[0], mm4);
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm7, mm4);
			punpckhbw_r2r(mm7, mm5);
			paddw_r2r(mm4, mm2);
			paddw_r2r(mm5, mm3);
			movq_m2r(pbb[0], mm4);
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm7, mm4);
			punpckhbw_r2r(mm7, mm5);
			paddw_r2r(mm4, mm2);
			paddw_r2r(mm5, mm3);
			movq_m2r(pbc[0], mm4);
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm7, mm4);
			punpckhbw_r2r(mm7, mm5);
			paddw_r2r(mm4, mm2);
			paddw_r2r(mm5, mm3);

			paddw_r2r(mm6, mm2);
			paddw_r2r(mm6, mm3);
			psrlw_i2r(2, mm2);			
			psrlw_i2r(2, mm3);
			
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			psrlw_i2r(1, mm6);
			paddw_r2r(mm6, mm0);
			paddw_r2r(mm6, mm1);
			psllw_i2r(1, mm6);
			psrlw_i2r(1, mm0);
			psrlw_i2r(1, mm1);
			
			movq_m2r(p2[0], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			
			psubw_r2r(mm2, mm0);
			psubw_r2r(mm3, mm1);
			pmaddwd_r2r(mm0, mm0);
			pmaddwd_r2r(mm1, mm1);
			paddd_r2r(mm1, mm0);
			
			movd_r2g(mm0, s1);
			psrlq_i2r(32, mm0);
			movd_r2g(mm0, s2);
			s += s1 + s2;						
			
			movq_m2r(pf[8], mm0);
			movq_r2r(mm0, mm1);
			punpcklbw_r2r(mm7, mm0);
			punpckhbw_r2r(mm7, mm1);
			movq_m2r(pfa[8], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			movq_m2r(pfb[8], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			movq_m2r(pfc[8], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			paddw_r2r(mm6, mm0);
			paddw_r2r(mm6, mm1);									
			psrlw_i2r(2, mm0);
			psrlw_i2r(2, mm1);
			
			movq_m2r(pb[8], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			movq_m2r(pba[8], mm4);
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm7, mm4);
			punpckhbw_r2r(mm7, mm5);
			paddw_r2r(mm4, mm2);
			paddw_r2r(mm5, mm3);
			movq_m2r(pbb[8], mm4);
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm7, mm4);
			punpckhbw_r2r(mm7, mm5);
			paddw_r2r(mm4, mm2);
			paddw_r2r(mm5, mm3);
			movq_m2r(pbc[8], mm4);
			movq_r2r(mm4, mm5);
			punpcklbw_r2r(mm7, mm4);
			punpckhbw_r2r(mm7, mm5);
			paddw_r2r(mm4, mm2);
			paddw_r2r(mm5, mm3);
			paddw_r2r(mm6, mm2);
			paddw_r2r(mm6, mm3);
			psrlw_i2r(2, mm2);			
			psrlw_i2r(2, mm3);
			
			paddw_r2r(mm2, mm0);
			paddw_r2r(mm3, mm1);
			psrlw_i2r(1, mm6);
			paddw_r2r(mm6, mm0);
			paddw_r2r(mm6, mm1);
			psllw_i2r(1, mm6);
			psrlw_i2r(1, mm0);
			psrlw_i2r(1, mm1);
			
			movq_m2r(p2[8], mm2);
			movq_r2r(mm2, mm3);
			punpcklbw_r2r(mm7, mm2);
			punpckhbw_r2r(mm7, mm3);
			
			psubw_r2r(mm2, mm0);
			psubw_r2r(mm3, mm1);
			pmaddwd_r2r(mm0, mm0);
			pmaddwd_r2r(mm1, mm1);
			paddd_r2r(mm1, mm0);
			
			movd_r2g(mm0, s1);
			psrlq_i2r(32, mm0);
			movd_r2g(mm0, s2);
			s += s1 + s2;
		
			p2  += lx;
			pf  += lx;
			pfa += lx;
			pfb += lx;
			pfc += lx;
			pb  += lx;
			pba += lx;
			pbb += lx;
			pbc += lx; 
		
			h--;
		} while(h > 0);
	}

	emms();
	
	return s;	
}
#endif
