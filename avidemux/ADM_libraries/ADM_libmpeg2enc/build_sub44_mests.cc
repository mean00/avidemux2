#include "config.h"
#include "mmxsse_motion.h"

int (*pmblocks_sub44_mests)( uint8_t *blk,  uint8_t *ref,
									int ilow, int jlow,
									int ihigh, int jhigh, 
									int h, int rowstride, 
									int threshold,
									me_result_s *resvec);

int build_sub44_mests_mmx( me_result_set *sub44set,
						   int ilow, int jlow, int ihigh, int jhigh, 
						   int i0, int j0,
						   int null_ctl_sad,
						   uint8_t *s44org, uint8_t *s44blk, 
						   int qrowstride, int qh,
						   int reduction )
{
	uint8_t *s44orgblk;
	me_result_s *sub44_mests = sub44set->mests;
	int istrt = ilow-i0;
	int jstrt = jlow-j0;
	int iend = ihigh-i0;
	int jend = jhigh-j0;
	int mean_weight;
	int threshold;

	
	threshold = 6*null_ctl_sad / (4*4*reduction);
	s44orgblk = s44org+(ilow>>2)+qrowstride*(jlow>>2);
	
	sub44set->len = (*pmblocks_sub44_mests)( s44orgblk, s44blk,
											  istrt, jstrt,
											  iend, jend, 
											  qh, qrowstride, 
											  threshold,
											  sub44_mests);
	
   /* If we're really pushing quality we reduce once otherwise twice. */
			
	sub_mean_reduction( sub44set, 1+(reduction>1),  &mean_weight);


	return sub44set->len;
}
