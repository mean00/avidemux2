#include "config.h"
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include "mmxsse_motion.h"
#include "fastintfns.h"

void find_best_one_pel_mmxe( me_result_set *sub22set,
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
	int ilim = ihigh-i0;
	int jlim = jhigh-j0;
	int dmin = INT_MAX;
	uint8_t *orgblk;
	int penalty;
	me_result_s matchrec;
	int32_t resvec[4];

	for( k = 0; k < sub22set->len; ++k )
	{
                int x;
		matchrec = sub22set->mests[k];
		orgblk = org + (i0+matchrec.x)+rowstride*(j0+matchrec.y);
		penalty = (abs(matchrec.x) + abs(matchrec.y))<<3;
		
		/* Get SAD for macroblocks: 	orgblk,orgblk(+1,0),
		   orgblk(0,+1), and orgblk(+1,+1)
		   Done all in one go to reduce memory bandwidth demand
		*/
                if( penalty>=dmin )
                    continue;
		x=mblock_nearest4_sads_mmxe(orgblk,blk,rowstride,h,resvec,dmin-penalty);
                if( x+penalty>=dmin )
                    continue;
		for( i = 0; i < 4; ++i )
		{
			if( matchrec.x <= ilim && matchrec.y <= jlim )
			{
		
				d = penalty+resvec[i];
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

