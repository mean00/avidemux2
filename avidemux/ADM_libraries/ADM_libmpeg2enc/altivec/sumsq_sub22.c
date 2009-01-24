/* sumsq_sub22.c, this file is part of the
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "altivec_motion.h"
#include "vectorize.h"
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


/*
 * Total squared difference between bidriection prediction of (8*h)
 * blocks of 2*2 subsampled pels.
 *
 * Iterate through all rows 2 at a time.
 *
 * Hints regarding input:
 *   b) blk2 is about 50% vector aligned and 50% 8 byte aligned
 *   c) rowstride is always a multiple of 16
 *   d) h == 4 or 8
 *
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 8; i++) {
 *         d = p1[i] - p2[i];
 *         sum += d * d;
 *     }
 *     p1 += rowstride;
 *     p2 += rowstride;
 * }
 */


#define SUMSQ_SUB22_PDECL                                                    \
  uint8_t *blk1,                                                             \
  uint8_t *blk2,                                                             \
  int rowstride,                                                             \
  int h                                                                      \

#define SUMSQ_SUB22_ARGS blk1, blk2, rowstride, h

int sumsq_sub22_altivec(SUMSQ_SUB22_PDECL)
{
    int i;
    unsigned char *pB, *pR;
    vector unsigned char align8x2;
    vector unsigned char lB0, lB1, lB2, lB3, lR0, lR1;
    vector unsigned char B, R;
    vector unsigned char min;
    vector unsigned char max;
    vector unsigned char dif;
    vector unsigned int sum;
    vector signed int zero;
    vector unsigned char perm1, perm2;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;

#ifdef ALTIVEC_VERIFY
    if (((unsigned long)blk2 % 8) != 0)
	mjpeg_error_exit1("sumsq_sub22: blk2 %% 8 != 0, (0x%X)", blk2);

    if (NOT_VECTOR_ALIGNED(rowstride))
	mjpeg_error_exit1("sumsq_sub22: rowstride %% 16 != 0, (%d)", rowstride);

    if (h != 4 && h != 8)
	mjpeg_error_exit1("sumsq_sub22: h != [4|8], (%d)", h);
#endif

    /* 8*h blocks calculated in 8*2 chunks */
    /* align8x2 = 0x( 00 01 02 03 04 05 06 07 10 11 12 13 14 15 16 17 ) {{{ */
    align8x2 = vec_lvsl(0, (unsigned char*)0);
    align8x2 = vec_sld(align8x2, align8x2, 8);
    perm1 = vec_lvsr(0, (unsigned char*)0);
    align8x2 = vec_sld(align8x2, perm1, 8);
    /* }}} */


    pB = blk1;
    pR = blk2;
    i = (h >> 1) - 1;

    zero = vec_splat_s32(0);
    sum = vec_splat_u32(0);

    perm1 = vec_lvsl(0, pB);

    perm2 = vec_lvsl(0, pR);
    perm2 = vec_splat(perm2, 0);
    perm2 = vec_add(perm2, align8x2);


    if (((unsigned long)pB & 0xf) <= 8) {

	perm1 = vec_splat(perm1, 0);
	perm1 = vec_add(perm1, align8x2);

	lB0 = vec_ld(0, pB);
	pB += rowstride;
	lB1 = vec_ld(0, pB);

	lR0 = vec_ld(0, pR);
	pR += rowstride;
	lR1 = vec_ld(0, pR);

	B = vec_perm(lB0, lB1, perm1);
	R = vec_perm(lR0, lR1, perm2);

	do {
	    pB += rowstride;
	    lB0 = vec_ld(0, pB);
	    pB += rowstride;
	    lB1 = vec_ld(0, pB);

	    pR += rowstride;
	    lR0 = vec_ld(0, pR);
	    pR += rowstride;
	    lR1 = vec_ld(0, pR);


	    max = vec_max(B, R);
	    min = vec_min(B, R);
	    dif = vec_sub(max, min);
	    sum = vec_msum(dif, dif, sum);


	    B = vec_perm(lB0, lB1, perm1);

	    R = vec_perm(lR0, lR1, perm2);
	} while (--i);

    } else {

	lB0 = vec_ld(0, pB);
	lB1 = vec_ld(16, pB);
	pB += rowstride;
	lB2 = vec_ld(0, pB);
	lB3 = vec_ld(16, pB);

	lR0 = vec_ld(0, pR);
	pR += rowstride;
	lR1 = vec_ld(0, pR);

	lB0 = vec_perm(lB0, lB1, perm1);
	lB2 = vec_perm(lB2, lB3, perm1);
	B = vec_perm(lB0, lB2, align8x2);

	R = vec_perm(lR0, lR1, perm2);


	do {
	    pB += rowstride;
	    lB0 = vec_ld(0, pB);
	    lB1 = vec_ld(16, pB);
	    pB += rowstride;
	    lB2 = vec_ld(0, pB);
	    lB3 = vec_ld(16, pB);

	    pR += rowstride;
	    lR0 = vec_ld(0, pR);
	    pR += rowstride;
	    lR1 = vec_ld(0, pR);


	    max = vec_max(B, R);
	    min = vec_min(B, R);
	    dif = vec_sub(max, min);
	    sum = vec_msum(dif, dif, sum);


	    lB0 = vec_perm(lB0, lB1, perm1);
	    lB2 = vec_perm(lB2, lB3, perm1);
	    B = vec_perm(lB0, lB2, align8x2);

	    R = vec_perm(lR0, lR1, perm2);
	} while (--i);
    }

    max = vec_max(B, R);
    min = vec_min(B, R);
    dif = vec_sub(max, min);
    sum = vec_msum(dif, dif, sum);


    vo.v = vec_sums(vs32(sum), zero);

    AMBER_STOP;

    return vo.s.sum;
}


#if ALTIVEC_TEST_FUNCTION(sumsq_sub22)
ALTIVEC_TEST(sumsq_sub22, int, (SUMSQ_SUB22_PDECL),
  "blk1=0x%X, blk2=0x%X, rowstride=%d, h=%d",
  SUMSQ_SUB22_ARGS);
#endif
/* vim:set foldmethod=marker foldlevel=0: */
