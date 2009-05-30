/*
 *
 * mblock_sub44_sads.c
 * Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
 *
 * Fast block sum-absolute difference computation for a rectangular area 4*x
 * by y where y > h against a 4 by h block.
 *
 * Used for 4*4 sub-sampled motion compensation calculations.
 *
 * This is actually just a shell that uses templates from the included
 * file "mblock_sub44_sads_x86_h.c".  I didn't trust the compiler to do a good
 * job on nested inlining.  One day I'll experiment.
 * 
 *
 * This file is part of mpeg2enc, a free MPEG-2 video stream encoder
 * based on the original MSSG reference design
 *
 * mpeg2enc is free software; you can redistribute new parts
 * and/or modify under the terms of the GNU General Public License 
 * as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See the files for those sections (c) MSSG
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ADM_default.h>
#ifdef HAVE_X86CPU
#include "mjpeg_types.h"
#include "attributes.h"
#include "mmx.h"
#include "fastintfns.h"
#include "motionsearch.h"
#include "mblock_sub44_sads_x86.h"

/*
  Register usage:
  mm0-mm3  Hold the current row 
  mm4      Used for accumulating partial SAD
  mm7      Holds zero
 */

static inline void mmx_zero_reg (void)
{
	/*  load 0 into mm7	 */
	pxor_r2r (mm7, mm7);
}

/*
 * Load a 8*4 block of 4*4 sub-sampled pels (qpels) into the MMX
 * registers
 *
 */

static __inline__ void load_blk(uint8_t *blk,uint32_t rowstride,int h)
{
	movq_m2r( *blk, mm0);
	blk += rowstride;
	movq_m2r( *blk, mm1);
	if( h == 2 )
		return;
	blk += rowstride;
	movq_m2r( *blk, mm2);
	blk += rowstride;
	movq_m2r( *blk, mm3);
}

/*
 * Do a shift right on the 4*4 block in the MMX registers
 * We do this as a macro as otherwise we get warnings from the
 * pre-optimised asm generated. 
 */
#define shift_blk(shift)\
	psrlq_i2r( shift,mm0);\
	psrlq_i2r( shift,mm1);\
	psrlq_i2r( shift,mm2);\
	psrlq_i2r( shift,mm3);

/*
 * Compute the Sum absolute differences between the 4*h block in
 * the MMX registers (4 least sig. bytes in mm0..mm3)
 *
 * and the 4*h block pointed to by refblk
 *
 * h == 2 || h == 4
 *
 * TODO: Currently always loads and shifts 4*4 even if 4*2 is required.
 *
 */

static __inline__ int qblock_sad_mmxe(uint8_t *refblk, 
								  uint32_t h,
								  uint32_t rowstride)
{
	int res;
	pxor_r2r 	(mm4,mm4);
			
	movq_r2r	(mm0,mm5);		/* First row */
	movd_m2r	(*refblk, mm6);
	pxor_r2r    ( mm7, mm7);
	refblk += rowstride;
	punpcklbw_r2r	( mm7, mm5);
	punpcklbw_r2r	( mm7, mm6);
	psadbw_r2r      ( mm5, mm6);
	paddw_r2r     ( mm6, mm4 );
	


	movq_r2r	(mm1,mm5);		/* Second row */
	movd_m2r	(*refblk, mm6);
	refblk += rowstride;
	punpcklbw_r2r	( mm7, mm5);
	punpcklbw_r2r	( mm7, mm6);
	psadbw_r2r      ( mm5, mm6);
	paddw_r2r     ( mm6, mm4 );

	if( h == 4 )
	{

		movq_r2r	(mm2,mm5);		/* Third row */
		movd_m2r	(*refblk, mm6);
		refblk += rowstride;
		punpcklbw_r2r	( mm7, mm5);
		punpcklbw_r2r	( mm7, mm6);
		psadbw_r2r      ( mm5, mm6);
		paddw_r2r     ( mm6, mm4 );

		
		movq_r2r	(mm3,mm5);		/* Fourth row */
		movd_m2r	(*refblk, mm6);
		punpcklbw_r2r	( mm7, mm5);
		punpcklbw_r2r	( mm7, mm6);
		psadbw_r2r      ( mm5, mm6);
		paddw_r2r     ( mm6, mm4 );
		
	}
	movd_r2m      ( mm4, res );

	return res;
}



static __inline__ int qblock_sad_mmx(uint8_t *refblk, 
								  uint32_t h,
								  uint32_t rowstride)
{
	int res;
	pxor_r2r 	(mm4,mm4);
			
	movq_r2r	(mm0,mm5);		/* First row */
	movd_m2r	(*refblk, mm6);
	pxor_r2r    ( mm7, mm7);
	refblk += rowstride;
	punpcklbw_r2r	( mm7, mm5);

	punpcklbw_r2r	( mm7, mm6);

	movq_r2r		( mm5, mm7);
	psubusw_r2r	( mm6, mm5);

	psubusw_r2r   ( mm7, mm6);

	paddw_r2r     ( mm5, mm4);
	paddw_r2r     ( mm6, mm4 );
	


	movq_r2r	(mm1,mm5);		/* Second row */
	movd_m2r	(*refblk, mm6);
	pxor_r2r    ( mm7, mm7);
	refblk += rowstride;
	punpcklbw_r2r	( mm7, mm5);
	punpcklbw_r2r	( mm7, mm6);
	movq_r2r		( mm5, mm7);
	psubusw_r2r	( mm6, mm5);
	psubusw_r2r   ( mm7, mm6);
	paddw_r2r     ( mm5, mm4);
	paddw_r2r     ( mm6, mm4 );

	if( h == 4 )
	{

		movq_r2r	(mm2,mm5);		/* Third row */
		movd_m2r	(*refblk, mm6);
		pxor_r2r    ( mm7, mm7);
		refblk += rowstride;
		punpcklbw_r2r	( mm7, mm5);
		punpcklbw_r2r	( mm7, mm6);
		movq_r2r		( mm5, mm7);
		psubusw_r2r	( mm6, mm5);
		psubusw_r2r   ( mm7, mm6);
		paddw_r2r     ( mm5, mm4);
		paddw_r2r     ( mm6, mm4 );
		
		movq_r2r	(mm3,mm5);		/* Fourth row */
		movd_m2r	(*refblk, mm6);
		pxor_r2r    ( mm7, mm7);
		punpcklbw_r2r	( mm7, mm5);
		punpcklbw_r2r	( mm7, mm6);
		movq_r2r		( mm5, mm7);
		psubusw_r2r	( mm6, mm5);
		psubusw_r2r   ( mm7, mm6);
		paddw_r2r     ( mm5, mm4);
		paddw_r2r     ( mm6, mm4 );
	}


	movq_r2r      ( mm4, mm5 );
    psrlq_i2r     ( 32, mm5 );
    paddw_r2r     ( mm5, mm4 );
	movq_r2r      ( mm4, mm6 );
    psrlq_i2r     ( 16, mm6 );
    paddw_r2r     ( mm6, mm4 );
	movd_r2m      ( mm4, res );

	return res & 0xffff;
}


/*
 * Do the Extended MMX versions
 */
#define SIMD_SUFFIX(x) x##_mmxe
#include "mblock_sub44_sads_x86_h.cc"
#undef SIMD_SUFFIX
/*
 * Do the original MMX versions
 */
#define SIMD_SUFFIX(x) x##_mmx
#include "mblock_sub44_sads_x86_h.cc"
#undef SIMD_SUFFIX




#endif
