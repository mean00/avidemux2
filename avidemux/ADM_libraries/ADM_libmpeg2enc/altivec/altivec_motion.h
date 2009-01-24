/* altivec_motion.h, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdint.h>
#include "../motionsearch.h"

#include "altivec_conf.h"


#define ALTIVEC_TEST_MOTION /* {{{ */                                        \
    ( ( defined(ALTIVEC_BENCHMARK) || defined(ALTIVEC_VERIFY) ) &&           \
      ( ALTIVEC_TEST_FUNCTION(build_sub44_mests) ||                          \
	ALTIVEC_TEST_FUNCTION(build_sub22_mests) ||                          \
        ALTIVEC_TEST_FUNCTION(find_best_one_pel) ||                          \
        ALTIVEC_TEST_FUNCTION(sub_mean_reduction) ||                         \
        ALTIVEC_TEST_FUNCTION(sad_00) ||                                     \
        ALTIVEC_TEST_FUNCTION(sad_01) ||                                     \
        ALTIVEC_TEST_FUNCTION(sad_10) ||                                     \
        ALTIVEC_TEST_FUNCTION(sad_11) ||                                     \
        ALTIVEC_TEST_FUNCTION(bsad) ||                                       \
        ALTIVEC_TEST_FUNCTION(sumsq) ||                                      \
        ALTIVEC_TEST_FUNCTION(sumsq_sub22) ||                                \
        ALTIVEC_TEST_FUNCTION(bsumsq) ||                                     \
        ALTIVEC_TEST_FUNCTION(bsumsq_sub22) ||                               \
        ALTIVEC_TEST_FUNCTION(subsample_image) ||                            \
        ALTIVEC_TEST_FUNCTION(variance) ) )                                  \
    /* }}} */


#ifdef __cplusplus
extern "C" {
#endif

void enable_altivec_motion(void);

ALTIVEC_FUNCTION(build_sub44_mests, int,
  (
    me_result_set *sub44set,
    int ilow, int jlow, int ihigh, int jhigh, 
    int i0, int j0,
    int null_ctl_sad,
    uint8_t *s44org, uint8_t *s44blk, 
    int qrowstride, int qh,
    int reduction
  ));

ALTIVEC_FUNCTION(build_sub22_mests, int,
  (
    me_result_set *sub44set,
    me_result_set *sub22set,
    int i0,  int j0, int ihigh, int jhigh, 
    int null_ctl_sad,
    uint8_t *s22org,  uint8_t *s22blk, 
    int frowstride, int fh,
    int reduction
  ));

ALTIVEC_FUNCTION(find_best_one_pel, void,
  (
    me_result_set *sub22set,
    uint8_t *org, uint8_t *blk,
    int i0, int j0,
    int ihigh, int jhigh,
    int rowstride, int h,
    me_result_s *best_so_far
  ));

ALTIVEC_FUNCTION(sub_mean_reduction, void,
	(me_result_set *matchset, int times, int *minweight_res));

ALTIVEC_FUNCTION(sad_00,int,
	(uint8_t *blk1, uint8_t *blk2, int rowstride, int h, int distlim));

ALTIVEC_FUNCTION(sad_01, int,
	(uint8_t *blk1, uint8_t *blk2, int rowstride, int h));

ALTIVEC_FUNCTION(sad_10, int,
	(uint8_t *blk1, uint8_t *blk2, int rowstride, int h));

ALTIVEC_FUNCTION(sad_11, int,
	(uint8_t *blk1, uint8_t *blk2, int rowstride, int h));

ALTIVEC_FUNCTION(bsad, int,
	(uint8_t *pf, uint8_t *pb, uint8_t *p2, int rowstride,
	 int hxf, int hyf, int hxb, int hyb, int h));


ALTIVEC_FUNCTION(sumsq, int,
	(uint8_t *blk1, uint8_t *blk2, int rowstride, int hx, int hy, int h));

ALTIVEC_FUNCTION(sumsq_sub22, int,
	(uint8_t *blk1, uint8_t *blk2, int rowstride, int h));


ALTIVEC_FUNCTION(bsumsq, int,
	(uint8_t *pf, uint8_t *pb, uint8_t *p2, int rowstride,
	 int hxf, int hyf, int hxb, int hyb, int h));

ALTIVEC_FUNCTION(bsumsq_sub22, int,
	(uint8_t *blk1f, uint8_t *blk1b, uint8_t *blk2, int rowstride, int h));


ALTIVEC_FUNCTION(subsample_image, void,
	(uint8_t *image, int rowstride,
	 uint8_t *sub22_image, uint8_t *sub44_image));

ALTIVEC_FUNCTION(variance, void,
	(uint8_t *p, int size, int rowstride,
	 unsigned int *p_var, unsigned int *p_mean));

#ifdef __cplusplus
}
#endif
