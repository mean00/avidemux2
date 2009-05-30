/* motionsearch.c, block motion estimation for mpeg2enc  */


/* (C) 2000/2001 Andrew Stevens */

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
#include <stdio.h>
#include "ADM_default.h"
#include <limits.h>
#include <stdlib.h>
#if defined(ADM_BSD_FAMILY) || defined(ADM_SPARC)
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#include <math.h>
#include "cpu_accel.h"
#include "fastintfns.h"
#include "motionsearch.h"
#include "mjpeg_logging.h"


/* The AltiVec code needs access to symbols during benchmarking
 * and verification.
 */
#define STATIC static
#ifdef HAVE_ALTIVEC
//#include "altivec/altivec_motion.h"
#  if ALTIVEC_TEST_MOTION
#    undef STATIC
#    define STATIC /* static */
#  endif
#endif



#ifdef HAVE_X86CPU
#include "mmxsse_motion.h"
#endif

/*
 * Function pointers for selecting CPU specific implementations
 *
 */
/*
int (*pmblocks_sub44_mests)( uint8_t *blk,  uint8_t *ref,
							 int ilow, int jlow,
							 int ihigh, int jhigh,
							 int h, int rowstride,
							 int threshold,
							 me_result_s *resvec);
*/
void (*pfind_best_one_pel)( me_result_set *sub22set,
							uint8_t *org, uint8_t *blk,
							int i0, int j0,
							int ihigh, int jhigh,
							int rowstride, int h, 
							me_result_s *res
	);
int (*pbuild_sub22_mests)( me_result_set *sub44set,
							me_result_set *sub22set,
							int i0,  int j0, int ihigh, int jhigh, 
							int null_mc_sad,
							uint8_t *s22org,  uint8_t *s22blk, 
							int frowstride, int fh,
							int reduction
							);

int (*pbuild_sub44_mests)( me_result_set *sub44set,
							int ilow, int jlow, int ihigh, int jhigh, 
							int i0, int j0,
							int null_mc_sad,
							uint8_t *s44org, uint8_t *s44blk, 
							int qrowstride, int qh,
							int reduction );

int (*psumsq_sub22)( uint8_t *blk1, uint8_t *blk2,
						 int rowstride, int h);
int (*pbsumsq_sub22)( uint8_t *blk1f, uint8_t *blk1b, 
						  uint8_t *blk2,
						  int rowstride, int h);

void (*pvariance)(uint8_t *mb, int size, int rowstride, 
				 uint32_t *p_var, uint32_t *p_mean);


int (*psad_sub22) ( uint8_t *blk1, uint8_t *blk2,  int frowstride, int fh);
int (*psad_sub44) ( uint8_t *blk1, uint8_t *blk2,  int qrowstride, int qh);
int (*psad_00) ( uint8_t *blk1, uint8_t *blk2,  int rowstride, int h, int distlim);
int (*psad_01) (uint8_t *blk1, uint8_t *blk2, int rowstride, int h);
int (*psad_10) (uint8_t *blk1, uint8_t *blk2, int rowstride, int h);
int (*psad_11) (uint8_t *blk1, uint8_t *blk2, int rowstride, int h);

int (*psumsq) (uint8_t *blk1, uint8_t *blk2,
					  int rowstride, int hx, int hy, int h);
  
  
int (*pbsumsq) (uint8_t *pf, uint8_t *pb,
				uint8_t *p2, int rowstride, int hxf, int hyf, int hxb, int hyb, int h);

int (*pbsad) (uint8_t *pf, uint8_t *pb,
					   uint8_t *p2, int rowstride, int hxf, int hyf, int hxb, int hyb, int h);


void (*psubsample_image) (uint8_t *image, int rowstride, 
						uint8_t *sub22_image, 
						uint8_t *sub44_image);



/*
 * Round search radius to suit the search algorithm.
 * Currently radii must be multiples of 8.
 *
 */

int round_search_radius( int radius )
{
	return intmax(8,((radius+4) /8)*8);
}



/*
	Take a vector of motion estimations and repeatedly make passes
	discarding all elements whose sad "weight" is above the current mean weight.
*/

void sub_mean_reduction( me_result_set *matchset, 
						 int times,
						 int *minweight_res)
{
	me_result_s *matches = matchset->mests;
	int len = matchset->len;
	int i,j;
	int weight_sum;
	int mean_weight;
	int min_weight = 100000;
	if( len == 0 )
	{
		*minweight_res = 100000;
		matchset->len = 0;
		return;
	}

	for(;;)
	{
		weight_sum = 0;
		for( i = 0; i < len ; ++i )
			weight_sum += matches[i].weight;
		mean_weight = weight_sum / len;
		
		if( times <= 0)
			break;
			
		j = 0;
		for( i =0; i < len; ++i )
		{
			if( matches[i].weight <= mean_weight )
			{
				if( times == 1)
				{
					min_weight = matches[i].weight ;
				}
				matches[j] = matches[i];
				++j;
			}
		}
		len = j;
		--times;
	}
	matchset->len = len;
	*minweight_res = mean_weight;
}

/* 
 * Build a vector of the top 4*4 sub-sampled motion estimations in
 * the box (ilow,jlow) to (ihigh,jhigh).
 *
 *	The algorithm is as follows: 
 * 
 *  1. Matches on an 4*4 pel grid are collected. All those matches
 * whose that is over a (conservative) threshold (basically 50% more
 * than moving average of the mean sad of such matches) are discarded.
 *	
 *	2. Multiple passes are made discarding worse than-average matches.
 *	The number of passes is specified by the user.  The default it 2
 *	(leaving roughly 1/4 of the matches).
 *
 * The intial threshold and discard passes are controlled by reduction
 * [1..4].  The intial SAD threshold is calculated as 6 / reduction of
 * a reference SAD passed as a parameter.  For reduction == 1 one
 * discard pass is made otherwise two are made.
 *	
 *	The net result is very fast and find good matches if they're to be
 *	found.  I.e. the penalty over exhaustive search is pretty low.
 *	
 *	NOTE: The "discard below average" trick depends critically on
 *	having some variation in the matches.  The slight penalty imposed
 *	for distant matches (reasonable since the motion vectors have to
 *	be encoded) is *vital* as otherwise pathologically bad performance
 *	results on highly uniform images.
 *	
 *	TODO: We should probably allow the user to eliminate the initial
 *	thinning of 4*4 grid matches if ultimate quality is demanded
 *	(e.g. for low bit-rate applications).
 * 
 */

STATIC int build_sub44_mests( me_result_set *sub44set,
							   int ilow, int jlow, int ihigh, int jhigh, 
							   int i0, int j0,
							   int null_ctl_sad,
							   uint8_t *s44org, uint8_t *s44blk, 
							   int qrowstride, int qh,
							   int reduction
							   )
{
	uint8_t *s44orgblk;
	me_result_s *sub44_mests = sub44set->mests;
	int istrt = ilow-i0;
	int jstrt = jlow-j0;
	int iend = ihigh-i0;
	int jend = jhigh-j0;
	int mean_weight;
	int threshold;

	int i,j;
	int s1;
	uint8_t *old_s44orgblk;
	int sub44_num_mests;

	/* N.b. we may ignore the right hand block of the pair going over the
	   right edge as we have carefully allocated the buffer oversize to ensure
	   no memory faults.  The later motion estimation calculations
	   performed on the results of this pass will filter out
	   out-of-range blocks...
	*/
	
	threshold = 6*null_ctl_sad / (4*4*reduction);
	s44orgblk = s44org+(ilow>>2)+qrowstride*(jlow>>2);
	
	/* Exhaustive search on 4*4 sub-sampled data.  This is affordable because
		(a)	it is only 16th of the size of the real 1-pel data 
		(b) we ignore those matches with an sad above our threshold.	
	*/

	sub44_num_mests = 0;

	/* Invariant:  s44orgblk = s44org+(i>>2)+qrowstride*(j>>2) */
	s44orgblk = s44org+(ilow>>2)+qrowstride*(jlow>>2);
	for( j = jstrt; j <= jend; j += 4 )
	{
		old_s44orgblk = s44orgblk;
		for( i = istrt; i <= iend; i += 4 )
		{
			s1 = ((*psad_sub44)( s44orgblk,s44blk,qrowstride,qh) & 0xffff);
			if( s1 < threshold )
			{
				threshold = intmin(s1<<2,threshold);
				sub44_mests[sub44_num_mests].x = i;
				sub44_mests[sub44_num_mests].y = j;
				sub44_mests[sub44_num_mests].weight = s1 + 
					(intmax(intabs(i-i0),intabs(j-j0))<<1);
				++sub44_num_mests;
			}
			s44orgblk += 1;
		}
		s44orgblk = old_s44orgblk + qrowstride;
	}
	sub44set->len = sub44_num_mests;
			
	sub_mean_reduction( sub44set, 1+(reduction>1),  &mean_weight);


	return sub44set->len;
}



/*  Build a vector of the best 2*2 sub-sampled motion estimations for
 *  the 16*16 macroblock at i0,j0 using a set of 4*4 sub-sampled matches as
 *  starting points.  As with with the 4*4 matches We don't collect
 *  them densely as they're just search starting points for 1-pel
 *  search and ones that are 1 out * should still give better than
 *  average matches...
 *
 *	 The resulting candidate motion vectors are thinned by thresholding
 *   and discarding worse than-average matches.
 *
 * The intial threshold and number of discard passes are controlled by reduction
 * [1..4]:  the intial SAD threshold is calculated as 6 / reduction of
 * a reference SAD passed as a parameter and then reduction discard passes
 * are made.
 *
 */


STATIC int build_sub22_mests( me_result_set *sub44set,
							   me_result_set *sub22set,
							   int i0,  int j0, int ihigh, int jhigh, 
							   int null_ctl_sad,
							   uint8_t *s22org,  uint8_t *s22blk, 
							   int frowstride, int fh,
							   int reduction)
{
	int i,k,s;
	int threshold = 6*null_ctl_sad / (2 * 2*reduction);
	
	int min_weight;
	int ilim = ihigh-i0;
	int jlim = jhigh-j0;
	int x,y;
	uint8_t *s22orgblk;
	
	sub22set->len = 0;
	for( k = 0; k < sub44set->len; ++k )
	{

		x = sub44set->mests[k].x;
		y = sub44set->mests[k].y;

		s22orgblk =  s22org +((y+j0)>>1)*frowstride +((x+i0)>>1);
		for( i = 0; i < 4; ++i )
		{
			if( x <= ilim && y <= jlim )
			{	
				s = (*psad_sub22)( s22orgblk,s22blk,frowstride,fh)+
					(intmax(intabs(x),intabs(y))<<3);
				if( s < threshold )
				{
					me_result_s *mc = &sub22set->mests[sub22set->len];
					mc->x = (int8_t)x;
					mc->y = (int8_t)y;
					mc->weight = s;
					++(sub22set->len);
				}
			}

			if( i == 1 )
			{
				s22orgblk += frowstride-1;
				x -= 2;
				y += 2;
			}
			else
			{
				s22orgblk += 1;
				x += 2;
				
			}
		}

	}

	sub_mean_reduction( sub22set,  reduction, &min_weight );
	return sub22set->len;
}


/*
 * Search for the best 1-pel match within 1-pel of a good 2*2-pel
 * match.  
 * 
 * N.b. best_so_far must be initialised by the caller!
 */


STATIC void find_best_one_pel( me_result_set *sub22set,
							   uint8_t *org, uint8_t *blk,
							   int i0, int j0,
							   int ihigh, int jhigh,
							   int rowstride, int h, 
							   me_result_s *best_so_far
	)

{
	int i,k;
	int d;
	me_result_s minpos = *best_so_far;
	int dmin = INT_MAX;
	int ilim = ihigh-i0;
	int jlim = jhigh-j0;
	uint8_t *orgblk;
	int penalty;
	me_result_s matchrec;
 
	for( k = 0; k < sub22set->len; ++k )
	{

		matchrec = sub22set->mests[k];
		orgblk = org + (i0+matchrec.x)+rowstride*(j0+matchrec.y);
		penalty = intmax(intabs(matchrec.x),intabs(matchrec.y))<<5;

		for( i = 0; i < 4; ++i )
		{
			if( matchrec.x <= ilim && matchrec.y <= jlim )
			{
		
				d = penalty+(*psad_00)(orgblk,blk,rowstride,h, dmin);
				if (d<dmin)
				{
					dmin = d;
					minpos = matchrec;
				}
			}
			if( i == 1 )
			{
				orgblk += rowstride-1;
				matchrec.x -= 1;
				matchrec.y += 1;
			}
			else
			{
				orgblk += 1;
				matchrec.x += 1;
			}
		}
	}

	minpos.weight = (uint16_t)intmin(255*255, dmin);
	*best_so_far = minpos;
}

 

/* 
 * sum absolute difference between two (16*h) blocks Four variations
 * depending on the required half pel interpolation of blk1 (hx,hy)
 *
 * blk1,blk2: addresses of top left pels of both blocks
 * rowstride:        distance (in bytes) of vertically adjacent pels
 * hx,hy:     flags for horizontal and/or vertical interpolation
 * h:         height of block (usually 8 or 16)
 * distlim: bail out if sum exceeds this value 
 *
 **/


STATIC int sad_00(uint8_t *blk1,uint8_t *blk2,
					int rowstride, int h,int distlim)
{
	uint8_t *p1,*p2;
	int j;
	int s;
	register int v;

	s = 0;
	p1 = blk1;
	p2 = blk2;
	for (j=0; j<h; j++)
	{

#define pipestep(o) v = p1[o]-p2[o]; s+= abs(v);
		pipestep(0);  pipestep(1);  pipestep(2);  pipestep(3);
		pipestep(4);  pipestep(5);  pipestep(6);  pipestep(7);
		pipestep(8);  pipestep(9);  pipestep(10); pipestep(11);
		pipestep(12); pipestep(13); pipestep(14); pipestep(15);
#undef pipestep

		if (s >= distlim)
			break;
			
		p1+= rowstride;
		p2+= rowstride;
	}
	return s;
}

STATIC int sad_01(uint8_t *blk1,uint8_t *blk2,int rowstride, int h)
{
	uint8_t *p1,*p2;
	int i,j;
	int s;
	register int v;

	s = 0;
	p1 = blk1;
	p2 = blk2;
	for (j=0; j<h; j++)
	{
		for (i=0; i<16; i++)
		{

			v = ((unsigned int)(p1[i]+p1[i+1]+1)>>1) - p2[i];
			s+=intabs(v);
		}
		p1+= rowstride;
		p2+= rowstride;
	}
	return s;
}

STATIC int sad_10(uint8_t *blk1,uint8_t *blk2, int rowstride, int h)
{
	uint8_t *p1,*p1a,*p2;
	int i,j;
	int s;
	register int v;

	s = 0;
	p1 = blk1;
	p2 = blk2;
	p1a = p1 + rowstride;
	for (j=0; j<h; j++)
	{
		for (i=0; i<16; i++)
		{
			v = ((unsigned int)(p1[i]+p1a[i]+1)>>1) - p2[i];
			s+= intabs(v);
		}
		p1 = p1a;
		p1a+= rowstride;
		p2+= rowstride;
	}

	return s;
}

STATIC int sad_11(uint8_t *blk1,uint8_t *blk2, int rowstride, int h)
{
	uint8_t *p1,*p1a,*p2;
	int i,j;
	int s;
	register int v;

	s = 0;
	p1 = blk1;
	p2 = blk2;
	p1a = p1 + rowstride;
	  
	for (j=0; j<h; j++)
	{
		for (i=0; i<16; i++)
		{
			v = ((unsigned int)((p1[i]+p1[i+1])+(p1a[i]+p1a[i+1])+2)>>2) - p2[i];
			s+=intabs(v);
		}
		p1 = p1a;
		p1a+= rowstride;
		p2+= rowstride;
	}
	return s;
}


/* 
 *  Compute subsampled images for fast motion compensation search
 *  N.b. rowstride should be *two* line widths for interlace images
 */

void subsample_image( uint8_t *image, int rowstride, 
					  uint8_t *sub22_image, 
					  uint8_t *sub44_image)
{
	uint8_t *blk = image;
	uint8_t *b, *nb;
	uint8_t *pb;
	uint8_t *qb;
	uint8_t *start_s22blk, *start_s44blk;
	int i;
	int nextfieldline = rowstride;

	start_s22blk   = sub22_image;
	start_s44blk   = sub44_image;
	b = blk;
	nb = (blk+nextfieldline);

	pb = (uint8_t *) start_s22blk;

	while( nb < start_s22blk )
	{
		for( i = 0; i < nextfieldline/4; ++i ) /* We're doing 4 pels horizontally at once */
		{
			pb[0] = ((b[0]+b[1])+(nb[0]+nb[1])+2)>>2;
			pb[1] = ((b[2]+b[3])+(nb[2]+nb[3])+2)>>2;	
			pb += 2;
			b += 4;
			nb += 4;
		}
		b += nextfieldline;
		nb = b + nextfieldline;
	}


	/* Now create the 4*4 sub-sampled data from the 2*2 
	   N.b. the 2*2 sub-sampled motion data preserves the interlace structure of the
	   original.  Albeit half as many lines and pixels...
	*/

	nextfieldline = nextfieldline >> 1;

	qb = start_s44blk;
	b  = start_s22blk;
	nb = (start_s22blk+nextfieldline);

	while( nb < start_s44blk )
	{
		for( i = 0; i < nextfieldline/4; ++i )
		{
			qb[0] = ((b[0]+b[1])+(nb[0]+nb[1])+2)>>2;
			qb[1] = ((b[2]+b[3])+(nb[2]+nb[3])+2)>>2;
			qb += 2;
			b += 4;
			nb += 4;
		}
		b += nextfieldline;
		nb = b + nextfieldline;
	}

}

/*
 * Same as sad_00 except for 2*2 subsampled data so only 8 wide!
 *
 */
 

STATIC int sad_sub22( uint8_t *s22blk1, uint8_t *s22blk2,int frowstride,int fh)
{
	uint8_t *p1 = s22blk1;
	uint8_t *p2 = s22blk2;
	int s = 0;
	int j;

	for( j = 0; j < fh; ++j )
	{
		register int diff;
#define pipestep(o) diff = p1[o]-p2[o]; s += abs(diff)
		pipestep(0); pipestep(1);
		pipestep(2); pipestep(3);
		pipestep(4); pipestep(5);
		pipestep(6); pipestep(7);
		p1 += frowstride;
		p2 += frowstride;
#undef pipestep
	}

	return s;
}



/*
 * Same as sad_00 except for 4*4 sub-sampled data.  
 *
 * N.b.: currently assumes only 16*16 or 16*8 motion estimation will
 * be used...  I.e. 4*4 or 4*2 sub-sampled blocks will be compared.  
 *
 *
 */


STATIC int sad_sub44( uint8_t *s44blk1, uint8_t *s44blk2,int qrowstride,int qh)
{
	register uint8_t *p1 = s44blk1;
	register uint8_t *p2 = s44blk2;
	int s = 0;
	register int diff;

	/* #define pipestep(o) diff = p1[o]-p2[o]; s += abs(diff) */
#define pipestep(o) diff = p1[o]-p2[o]; s += diff < 0 ? -diff : diff;
	pipestep(0); pipestep(1);	 pipestep(2); pipestep(3);
	if( qh > 1 )
	{
		p1 += qrowstride; p2 += qrowstride;
		pipestep(0); pipestep(1);	 pipestep(2); pipestep(3);
		if( qh > 2 )
		{
			p1 += qrowstride; p2 += qrowstride;
			pipestep(0); pipestep(1);	 pipestep(2); pipestep(3);
			p1 += qrowstride; p2 += qrowstride;
			pipestep(0); pipestep(1);	 pipestep(2); pipestep(3);
		}
	}


	return s;
}

/*
 * total squared difference between two (8*h) blocks of 2*2 sub-sampled pels
 * blk1,blk2: addresses of top left pels of both blocks
 * rowstride:        distance (in bytes) of vertically adjacent pels
 * h:         height of block (usually 8 or 16)
 */
 
STATIC int sumsq_sub22(uint8_t *blk1, uint8_t *blk2, int rowstride, int h)
{
	uint8_t *p1 = blk1, *p2 = blk2;
	int i,j,v;
	int s = 0;
	for (j=0; j<h; j++)
	{
		for (i=0; i<8; i++)
		{
			v = p1[i] - p2[i];
			s+= v*v;
		}
		p1+= rowstride;
		p2+= rowstride;
	}
	return s;
}

/* total squared difference between bidirection prediction of (8*h)
 * blocks of 2*2 sub-sampled pels and reference 
 * blk1f, blk1b,blk2: addresses of top left
 * pels of blocks 
 * rowstride: distance (in bytes) of vertically adjacent
 * pels 
 * h: height of block (usually 4 or 8)
 */
 
STATIC int bsumsq_sub22(uint8_t *blk1f, uint8_t *blk1b, uint8_t *blk2, 
					 int rowstride, int h)
{
	uint8_t *p1f = blk1f,*p1b = blk1b,*p2 = blk2;
	int i,j,v;
	int s = 0;
	for (j=0; j<h; j++)
	{
		for (i=0; i<8; i++)
		{
			v = ((p1f[i]+p1b[i]+1)>>1) - p2[i];
			s+= v*v;
		}
		p1f+= rowstride;
		p1b+= rowstride;
		p2+= rowstride;
	}
	return s;
}

/*
 * total squared difference between two (16*h) blocks
 * including optional half pel interpolation of blk1 (hx,hy)
 * blk1,blk2: addresses of top left pels of both blocks
 * rowstride:        distance (in bytes) of vertically adjacent pels
 * hx,hy:     flags for horizontal and/or vertical interpolation
 * h:         height of block (usually 8 or 16)
 */
 

STATIC int sumsq(
// Meanx blk1,blk2,rowstride,hx,hy,h)
	uint8_t *blk1,uint8_t *blk2,
	int rowstride,int hx,int hy,int h)
{
	uint8_t *p1,*p1a,*p2;
	int i,j;
	int s,v;

	s = 0;
	p1 = blk1;
	p2 = blk2;
	if (!hx && !hy)
		for (j=0; j<h; j++)
		{
			for (i=0; i<16; i++)
			{
				v = p1[i] - p2[i];
				s+= v*v;
			}
			p1+= rowstride;
			p2+= rowstride;
		}
	else if (hx && !hy)
		for (j=0; j<h; j++)
		{
			for (i=0; i<16; i++)
			{
				v = ((unsigned int)(p1[i]+p1[i+1]+1)>>1) - p2[i];
				s+= v*v;
			}
			p1+= rowstride;
			p2+= rowstride;
		}
	else if (!hx && hy)
	{
		p1a = p1 + rowstride;
		for (j=0; j<h; j++)
		{
			for (i=0; i<16; i++)
			{
				v = ((unsigned int)(p1[i]+p1a[i]+1)>>1) - p2[i];
				s+= v*v;
			}
			p1 = p1a;
			p1a+= rowstride;
			p2+= rowstride;
		}
	}
	else /* if (hx && hy) */
	{
		p1a = p1 + rowstride;
		for (j=0; j<h; j++)
		{
			for (i=0; i<16; i++)
			{
				v = ((unsigned int)(p1[i]+p1[i+1]+p1a[i]+p1a[i+1]+2)>>2) - p2[i];
				s+= v*v;
			}
			p1 = p1a;
			p1a+= rowstride;
			p2+= rowstride;
		}
	}
 
	return s;
}


/*
 * absolute difference error between a (16*h) block and a bidirectional
 * prediction
 *
 * p2: address of top left pel of block
 * pf,hxf,hyf: address and half pel flags of forward ref. block
 * pb,hxb,hyb: address and half pel flags of backward ref. block
 * h: height of block
 * rowstride: distance (in bytes) of vertically adjacent pels in p2,pf,pb
 */
 

STATIC int bsad(
//meanx pf,pb,p2,rowstride,hxf,hyf,hxb,hyb,h)
	uint8_t *pf,uint8_t *pb,uint8_t *p2,
	int rowstride,int hxf,int hyf,int hxb,int hyb,int h)
{
	uint8_t *pfa,*pfb,*pfc,*pba,*pbb,*pbc;
	int i,j;
	int s,v;

	pfa = pf + hxf;
	pfb = pf + rowstride*hyf;
	pfc = pfb + hxf;

	pba = pb + hxb;
	pbb = pb + rowstride*hyb;
	pbc = pbb + hxb;

	s = 0;

	for (j=0; j<h; j++)
	{
		for (i=0; i<16; i++)
		{
			v = ((((unsigned int)(*pf++ + *pfa++ + *pfb++ + *pfc++ + 2)>>2) +
				  ((unsigned int)(*pb++ + *pba++ + *pbb++ + *pbc++ + 2)>>2) + 1)>>1)
				- *p2++;
			s += abs(v);
		}
		p2+= rowstride-16;
		pf+= rowstride-16;
		pfa+= rowstride-16;
		pfb+= rowstride-16;
		pfc+= rowstride-16;
		pb+= rowstride-16;
		pba+= rowstride-16;
		pbb+= rowstride-16;
		pbc+= rowstride-16;
	}

	return s;
}

/*
 * squared error between a (16*h) block and a bidirectional
 * prediction
 *
 * p2: address of top left pel of block
 * pf,hxf,hyf: address and half pel flags of forward ref. block
 * pb,hxb,hyb: address and half pel flags of backward ref. block
 * h: height of block
 * rowstride: distance (in bytes) of vertically adjacent pels in p2,pf,pb
 */
 

STATIC int bsumsq(
//meanx pf,pb,p2,rowstride,hxf,hyf,hxb,hyb,h)
	uint8_t *pf,uint8_t *pb,uint8_t *p2,
	int rowstride,int hxf,int hyf,int hxb,int hyb,int h)
{
	uint8_t *pfa,*pfb,*pfc,*pba,*pbb,*pbc;
	int i,j;
	int s,v;
	
	pfa = pf + hxf;
	pfb = pf + rowstride*hyf;
	pfc = pfb + hxf;

	pba = pb + hxb;
	pbb = pb + rowstride*hyb;
	pbc = pbb + hxb;

	s = 0;

	for (j=0; j<h; j++)
	{
		for (i=0; i<16; i++)
		{
#define ui(x) ((unsigned int)x)
			v = ((((ui(*pf++) + ui(*pfa++) + ui(*pfb++) + ui(*pfc++) + 2)>>2) +
				  ((ui(*pb++) + ui(*pba++) + ui(*pbb++) + ui(*pbc++) + 2)>>2)
				  + 1
				)>>1) - ui(*p2++);
#undef ui
			s+=v*v;
		}
		p2+= rowstride-16;
		pf+= rowstride-16;
		pfa+= rowstride-16;
		pfb+= rowstride-16;
		pfc+= rowstride-16;
		pb+= rowstride-16;
		pba+= rowstride-16;
		pbb+= rowstride-16;
		pbc+= rowstride-16;
	}

	return s;
}


/*
 * variance of a (size*size) block, multiplied by 256
 * p:  address of top left pel of block
 * rowstride: distance (in bytes) of vertically adjacent pels
 * SIZE is a multiple of 8.
 */
void variance(uint8_t *p, int size,	int rowstride,
			 uint32_t *p_var,uint32_t  *p_mean)
{
	int i,j;
	unsigned int v,s,s2;
	s = s2 = 0;

	for (j=0; j<size; j++)
	{
		for (i=0; i<size; i++)
		{
			v = *p++;
			s+= v;
			s2+= v*v;
		}
		p+= rowstride-size;
	}
	*p_mean = s/(size*size);
	*p_var = s2 - (s*s)/(size*size);
}




/*
 *  Initialise motion estimation - currently only selection of which
 * versions of the various low level computation routines to use
 * 
 */

void init_motion_search(void)
{
	/* Initialize function pointers. This allows partial acceleration
	 * implementations to update only the function pointers they support.
	 */
	psad_sub22 = sad_sub22;
	psad_sub44 = sad_sub44;
	psad_00 = sad_00;
	psad_01 = sad_01;
	psad_10 = sad_10;
	psad_11 = sad_11;
	pbsad = bsad;
	pvariance = variance;
	psumsq = sumsq;
	pbsumsq = bsumsq;
	psumsq_sub22 = sumsq_sub22;
	pbsumsq_sub22 = bsumsq_sub22;
	pfind_best_one_pel = find_best_one_pel;
	pbuild_sub22_mests = build_sub22_mests;
	pbuild_sub44_mests = build_sub44_mests;
	psubsample_image = subsample_image;

#ifdef HAVE_X86CPU
	printf("[Mpeg2enc]Enabling mmx motion search\n");
	enable_mmxsse_motion(0);
#endif

}
