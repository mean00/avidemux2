/* Optimised lowlevel motion estimation routines for mpeg2enc */

/* (C) 2000/2001 Andrew Stevens */

/* This is free software; you can redistribute it
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

#include "motionsearch.h"
#include "mblock_sub44_sads_x86.h"



void sub_mean_reduction( me_result_set *matchset,
			 int times,
			 int *minweight_res);

void enable_mmxsse_motion(int a);
void mblock_sub22_nearest4_sads_mmxe(uint8_t *blk1,uint8_t *blk2,
				 int frowstride,int fh, int* resvec);

//int mblock_nearest4_sads_mmxe(uint8_t *blk1, uint8_t *blk2,int rowstride, int h, int *resvec);
int mblock_nearest4_sads_mmxe(uint8_t *blk1,uint8_t *blk2,int lx,int h,int32_t *weightvec,int peakerror);


int sad_00_mmxe(uint8_t *blk1, uint8_t *blk2, int rowstride, int h, int distlim);
int sad_01_mmxe(uint8_t *blk1, uint8_t *blk2, int rowstride, int h);
int sad_10_mmxe(uint8_t *blk1, uint8_t *blk2, int rowstride, int h);
int sad_11_mmxe(uint8_t *blk1, uint8_t *blk2, int rowstride, int h);


int sad_sub22_mmxe ( uint8_t *blk1, uint8_t *blk2,  int frowstride, int fh) ;
int sad_sub44_mmxe ( uint8_t *blk1, uint8_t *blk2,  int qrowstride, int qh);
int sumsq_mmx( uint8_t *blk1, uint8_t *blk2, int rowstride, int hx, int hy, int h) ;
int sumsq_sub22_mmx( uint8_t *blk1, uint8_t *blk2, int rowstride, int h) ;
int bsumsq_sub22_mmx( uint8_t *blk1f, uint8_t *blk1b, uint8_t *blk2, int rowstride, int h) ;
int bsumsq_mmx (uint8_t *pf, uint8_t *pb, uint8_t *p2, int rowstride,
				int hxf, int hyf, int hxb, int hyb, int h);
int bsad_mmx (uint8_t *pf, uint8_t *pb, uint8_t *p2, int rowstride,
	      int hxf, int hyf, int hxb, int hyb, int h);

void variance_mmx( uint8_t *p, int size,	int rowstride,
				  uint32_t *p_variance, uint32_t *p_mean);

int sad_00_mmx ( uint8_t *blk1, uint8_t *blk2,  int rowstride, int h, int distlim);
int sad_01_mmx(uint8_t *blk1, uint8_t *blk2, int rowstride, int h);
int sad_10_mmx(uint8_t *blk1, uint8_t *blk2, int rowstride, int h);
int sad_11_mmx(uint8_t *blk1, uint8_t *blk2, int rowstride, int h);
int sad_sub22_mmx ( uint8_t *blk1, uint8_t *blk2,  int frowstride, int fh);
int sad_sub44_mmx (uint8_t *blk1, uint8_t *blk2,  int qrowstride, int qh);


/* Assembler core routine used in x86 MMX/SSE implementation */ 

extern int (*pmblocks_sub44_mests)( uint8_t *blk,  uint8_t *ref,
					int ilow, int jlow,
					int ihigh, int jhigh, 
					int h, int rowstride, 
					int threshold,
					me_result_s *resvec);

void find_best_one_pel_mmxe( me_result_set *sub22set,
					 uint8_t *org, uint8_t *blk,
					 int i0, int j0,
					 int ihigh, int jhigh,
					 int rowstride, int h,
					 me_result_s *best_so_far);

int build_sub44_mests_mmx( me_result_set *sub44set,
				   int ilow, int jlow, int ihigh, int jhigh, 
				   int i0, int j0,
				   int null_ctl_sad,
				   uint8_t *s44org, uint8_t *s44blk,
				   int qrowstride, int qh,
				   int reduction);

int build_sub22_mests_mmxe( me_result_set *sub44set,
				me_result_set *sub22set,
				int i0,  int j0, int ihigh, int jhigh, 
				int null_ctl_sad,
				uint8_t *s22org,  uint8_t *s22blk, 
				int frowstride, int fh,
				int reduction);

