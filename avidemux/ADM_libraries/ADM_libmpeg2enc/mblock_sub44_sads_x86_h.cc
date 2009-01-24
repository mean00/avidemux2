/*
 *
 * mblock_sub44_sads_x86_h.c
 * Copyright (C) 2000 Andrew Stevens <as@comlab.ox.ac.uk>
 *
 * Fast block sum-absolute difference computation for a rectangular area 4*x
 * by y where y > h against a 4 by h block.
 *
 * Used for 4*4 sub-sampled motion compensation calculations.
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
 * mpeg2enc is distributed in the hope that it will be useful,
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

/*
 *
 * Generates a vector sad's for 4*4 sub-sampled pel (qpel) data (with
 * co-ordinates and top-left qpel address) from specified rectangle
 * against a specified 16*h pel (4*4 qpel) reference block.  The
 * generated vector contains results only for those sad's that fall
 * below twice the running best sad and are aligned on 8-pel
 * boundaries
 *
 * Invariant: blk points to top-left sub-sampled pel for macroblock
 * at (ilow,ihigh)
 * i{low,high) j(low,high) must be multiples of 4.
 *
 * sad = Sum Absolute Differences
 *
 * NOTES: for best efficiency i{low,high) should be multiples of 16.
 *
 * */

int SIMD_SUFFIX(mblocks_sub44_mests)( uint8_t *blk,  uint8_t *ref,
									  int ilow,int jlow,
									  int ihigh, int jhigh, 
									  int h, int rowstride, 
									  int threshold,
									  me_result_s *resvec)
{
	int32_t x,y;
	uint8_t *currowblk = blk;
	uint8_t *curblk;
	me_result_s *cres = resvec;
	int      gridrowstride = (rowstride);
	int weight;

	for( y=jlow; y <= jhigh ; y+=4)
	{
		curblk = currowblk;
		for( x = ilow; x <= ihigh; x += 4)
		{
			if( (x & 15) == (ilow & 15) )
			{
				load_blk( curblk, rowstride, h );
			}
			weight = SIMD_SUFFIX(qblock_sad)(ref, h, rowstride);
			if( weight <= threshold )
			{
				threshold = intmin(weight<<2,threshold);
				/* Rough and-ready absolute distance penalty */
				/* NOTE: This penalty is *vital* to correct operation 
				   as otherwise the sub-mean filtering won't work on very
				   uniform images.
				 */
				cres->weight = (uint16_t)(weight+(intmax(intabs(x),intabs(y))<<2));
				cres->x = (uint8_t)x;
				cres->y = (uint8_t)y;
				++cres;
			}
			curblk += 1;
			shift_blk(8);
		}
		currowblk += gridrowstride;
	}
	emms();
	return cres - resvec;
}

#undef concat
