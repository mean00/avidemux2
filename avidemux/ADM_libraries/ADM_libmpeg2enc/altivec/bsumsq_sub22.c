/* bsumsq_sub22.c, this file is part of the
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
 * Total squared difference between bidirection prediction of (8*h)
 * blocks of 2*2 subsampled pels.
 *
 * Iterate through all rows 2 at a time.
 *
 * Hints regarding input:
 *   b) ref is about 50% vector aligned and 50% 8 byte aligned
 *   c) rowstride is always a multiple of 16
 *   d) h == 4 or 8
 *
 *
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 8; i++) {
 * 	d = ((p1f[i]+p1b[i]+1)>>1) - p2[i];
 * 	sum += d * d;
 *     }
 *     p1f += rowstride;
 *     p1b += rowstride;
 *     p2 += rowstride;
 * }
 */

#define BSUMSQ_SUB22_PDECL                                                   \
  uint8_t *blk1f,                                                            \
  uint8_t *blk1b,                                                            \
  uint8_t *blk2,                                                             \
  int rowstride,                                                             \
  int h                                                                      \

#define BSUMSQ_SUB22_ARGS blk1f, blk1b, blk2, rowstride, h

int bsumsq_sub22_altivec(BSUMSQ_SUB22_PDECL)
{
    int i;
    int lt8B, lt8F;
    unsigned char *pB, *pF, *pR;
    vector unsigned char align8x2;
    vector unsigned char permB, permF, permR;
    vector unsigned char lB0, lB1, lB2, lB3;
    vector unsigned char lF0, lF1, lF2, lF3;
    vector unsigned char lR0, lR1;
    vector unsigned char B, F, R;
    vector unsigned short bH, bL, fH, fL;
    vector unsigned char min;
    vector unsigned char max;
    vector unsigned char dif;
    vector unsigned int sum;
    vector unsigned char zero;
    vector unsigned short one;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;

#ifdef ALTIVEC_VERIFY
    if (((unsigned long)blk2 % 8) != 0)
	mjpeg_error_exit1("bsumsq_sub22: blk2 %% 8 != 0, (0x%X)", blk2);

    if (NOT_VECTOR_ALIGNED(rowstride))
	mjpeg_error_exit1("bsumsq_sub22: rowstride %% 16 != 0, (%d)",
	    rowstride);

    if (h != 4 && h != 8)
	mjpeg_error_exit1("bsumsq_sub22: h != [4|8], (%d)", h);
#endif

    /* 8*h blocks calculated in 8*2 chunks */
    /* align8x2 = 0x( 00 01 02 03 04 05 06 07 10 11 12 13 14 15 16 17 ) {{{ */
    align8x2 = vec_lvsl(0, (unsigned char*)0);
    align8x2 = vec_sld(align8x2, align8x2, 8);
    permB = vec_lvsr(0, (unsigned char*)0);
    align8x2 = vec_sld(align8x2, permB, 8);
    /* }}} */


    i = (h >> 1) - 1;

    zero = vec_splat_u8(0);
    one = vec_splat_u16(1);

    sum = vec_splat_u32(0);

    pR = blk2;
    permR = vec_lvsl(0, pR);
    permR = vec_splat(permR, 0);
    permR = vec_add(permR, align8x2);

    lt8B = (((unsigned long)blk1b & 0xf) <= 8);
    lt8F = (((unsigned long)blk1f & 0xf) <= 8);

    if (lt8B && lt8F) {
	pB = blk1b;
	pF = blk1f;

	permB = vec_lvsl(0, pB);
	permB = vec_splat(permB, 0);
	permB = vec_add(permB, align8x2);

	permF = vec_lvsl(0, pF);
	permF = vec_splat(permF, 0);
	permF = vec_add(permF, align8x2);

	lB0 = vec_ld(0, pB);
	pB += rowstride;
	lB1 = vec_ld(0, pB);

	lF0 = vec_ld(0, pF);
	pF += rowstride;
	lF1 = vec_ld(0, pF);

	lR0 = vec_ld(0, pR);
	pR += rowstride;
	lR1 = vec_ld(0, pR);

	B = vec_perm(lB0, lB1, permB);
	F = vec_perm(lF0, lF1, permF);
	R = vec_perm(lR0, lR1, permR);

	do {
	    pB += rowstride;
	    lB0 = vec_ld(0, pB);
	    pB += rowstride;
	    lB1 = vec_ld(0, pB);

	    pF += rowstride;
	    lF0 = vec_ld(0, pF);
	    pF += rowstride;
	    lF1 = vec_ld(0, pF);

	    pR += rowstride;
	    lR0 = vec_ld(0, pR);
	    pR += rowstride;
	    lR1 = vec_ld(0, pR);


	    /* (unsigned short[]) pB[0-7] */
	    bH = vu16(vec_mergeh(zero, B));

	    /* (unsigned short[]) pF[0-7] */
	    fH = vu16(vec_mergeh(zero, F));

	    /* pB[i] + pF[i] */
	    bH = vec_add(bH, fH);

	    /* (unsigned short[]) pB[8-15] */
	    bL = vu16(vec_mergel(zero, B));

	    /* (unsigned short[]) pF[8-15] */
	    fL = vu16(vec_mergel(zero, F));

	    /* pB[i] + pF[i] */
	    bL = vec_add(bL, fL);

	    /* (pB[i]+pF[i]) + 1 */
	    bH = vec_add(bH, one);
	    bL = vec_add(bL, one);
					
	    /* (pB[i]+pF[i]+1) >> 1 */
	    bH = vec_sra(bH, one);
	    bL = vec_sra(bL, one);

	    /* d = abs( ((pB[i]+pF[i]+1)>>1) - pR[i] ) */
	    bH = vu16(vec_packsu(bH, bL));
	    min = vec_min(vu8(bH), R);
	    max = vec_max(vu8(bH), R);
	    dif = vec_sub(max, min);
					
	    /* sum += (d * d) */
	    sum = vec_msum(dif, dif, sum);


	    B = vec_perm(lB0, lB1, permB);
	    F = vec_perm(lF0, lF1, permF);
	    R = vec_perm(lR0, lR1, permR);

	} while (--i);

    }
    else if (lt8B || lt8F)
    {
	if (lt8F) {
	    pB = blk1b;
	    pF = blk1f;
	} else {
	    pB = blk1f;
	    pF = blk1b;
	}

	permB = vec_lvsl(0, pB);

	permF = vec_lvsl(0, pF);
	permF = vec_splat(permF, 0);
	permF = vec_add(permF, align8x2);

	lB0 = vec_ld(0, pB);
	lB1 = vec_ld(16, pB);
	pB += rowstride;
	lB2 = vec_ld(0, pB);
	lB3 = vec_ld(16, pB);

	lF0 = vec_ld(0, pF);
	pF += rowstride;
	lF1 = vec_ld(0, pF);

	lR0 = vec_ld(0, pR);
	pR += rowstride;
	lR1 = vec_ld(0, pR);


	lB0 = vec_perm(lB0, lB1, permB);
	lB2 = vec_perm(lB2, lB3, permB);
	B = vec_perm(lB0, lB2, align8x2);

	F = vec_perm(lF0, lF1, permF);
	R = vec_perm(lR0, lR1, permR);

	do {
	    pB += rowstride;
	    lB0 = vec_ld(0, pB);
	    lB1 = vec_ld(16, pB);
	    pB += rowstride;
	    lB2 = vec_ld(0, pB);
	    lB3 = vec_ld(16, pB);

	    pF += rowstride;
	    lF0 = vec_ld(0, pF);
	    pF += rowstride;
	    lF1 = vec_ld(0, pF);

	    pR += rowstride;
	    lR0 = vec_ld(0, pR);
	    pR += rowstride;
	    lR1 = vec_ld(0, pR);


	    /* (unsigned short[]) pB[0-7] */
	    bH = vu16(vec_mergeh(zero, B));

	    /* (unsigned short[]) pF[0-7] */
	    fH = vu16(vec_mergeh(zero, F));

	    /* pB[i] + pF[i] */
	    bH = vec_add(bH, fH);

	    /* (unsigned short[]) pB[8-15] */
	    bL = vu16(vec_mergel(zero, B));

	    /* (unsigned short[]) pF[8-15] */
	    fL = vu16(vec_mergel(zero, F));

	    /* pB[i] + pF[i] */
	    bL = vec_add(bL, fL);

	    /* (pB[i]+pF[i]) + 1 */
	    bH = vec_add(bH, one);
	    bL = vec_add(bL, one);
					
	    /* (pB[i]+pF[i]+1) >> 1 */
	    bH = vec_sra(bH, one);
	    bL = vec_sra(bL, one);

	    /* d = abs( ((pB[i]+pF[i]+1)>>1) - pR[i] ) */
	    bH = vu16(vec_packsu(bH, bL));
	    min = vec_min(vu8(bH), R);
	    max = vec_max(vu8(bH), R);
	    dif = vec_sub(max, min);
					
	    /* sum += (d * d) */
	    sum = vec_msum(dif, dif, sum);

	    lB0 = vec_perm(lB0, lB1, permB);
	    lB2 = vec_perm(lB2, lB3, permB);
	    B = vec_perm(lB0, lB2, align8x2);

	    F = vec_perm(lF0, lF1, permF);
	    R = vec_perm(lR0, lR1, permR);

	} while (--i);

    } else {
	pB = blk1b;
	pF = blk1f;

	permB = vec_lvsl(0, pB);
	permF = vec_lvsl(0, pF);

	lB0 = vec_ld(0, pB);
	lB1 = vec_ld(16, pB);
	pB += rowstride;
	lB2 = vec_ld(0, pB);
	lB3 = vec_ld(16, pB);

	lF0 = vec_ld(0, pF);
	lF1 = vec_ld(16, pF);
	pF += rowstride;
	lF2 = vec_ld(0, pF);
	lF3 = vec_ld(16, pF);

	lR0 = vec_ld(0, pR);
	pR += rowstride;
	lR1 = vec_ld(0, pR);

	lB0 = vec_perm(lB0, lB1, permB);
	lB2 = vec_perm(lB2, lB3, permB);
	B = vec_perm(lB0, lB2, align8x2);

	lF0 = vec_perm(lF0, lF1, permF);
	lF2 = vec_perm(lF2, lF3, permF);
	F = vec_perm(lF0, lF2, align8x2);

	R = vec_perm(lR0, lR1, permR);

	do {
	    pB += rowstride;
	    lB0 = vec_ld(0, pB);
	    lB1 = vec_ld(16, pB);
	    pB += rowstride;
	    lB2 = vec_ld(0, pB);
	    lB3 = vec_ld(16, pB);

	    pF += rowstride;
	    lF0 = vec_ld(0, pF);
	    lF1 = vec_ld(16, pF);
	    pF += rowstride;
	    lF2 = vec_ld(0, pF);
	    lF3 = vec_ld(16, pF);

	    pR += rowstride;
	    lR0 = vec_ld(0, pR);
	    pR += rowstride;
	    lR1 = vec_ld(0, pR);

	    /* (unsigned short[]) pB[0-7] */
	    bH = vu16(vec_mergeh(zero, B));

	    /* (unsigned short[]) pF[0-7] */
	    fH = vu16(vec_mergeh(zero, F));

	    /* pB[i] + pF[i] */
	    bH = vec_add(bH, fH);

	    /* (unsigned short[]) pB[8-15] */
	    bL = vu16(vec_mergel(zero, B));

	    /* (unsigned short[]) pF[8-15] */
	    fL = vu16(vec_mergel(zero, F));

	    /* pB[i] + pF[i] */
	    bL = vec_add(bL, fL);

	    /* (pB[i]+pF[i]) + 1 */
	    bH = vec_add(bH, one);
	    bL = vec_add(bL, one);

	    /* (pB[i]+pF[i]+1) >> 1 */
	    bH = vec_sra(bH, one);
	    bL = vec_sra(bL, one);

	    /* d = abs( ((pB[i]+pF[i]+1)>>1) - pR[i] ) */
	    bH = vu16(vec_packsu(bH, bL));
	    min = vec_min(vu8(bH), R);
	    max = vec_max(vu8(bH), R);
	    dif = vec_sub(max, min);

	    /* sum += (d * d) */
	    sum = vec_msum(dif, dif, sum);

	    lB0 = vec_perm(lB0, lB1, permB);
	    lB2 = vec_perm(lB2, lB3, permB);
	    B = vec_perm(lB0, lB2, align8x2);

	    lF0 = vec_perm(lF0, lF1, permF);
	    lF2 = vec_perm(lF2, lF3, permF);
	    F = vec_perm(lF0, lF2, align8x2);

	    R = vec_perm(lR0, lR1, permR);
	} while (--i);
    }

    /* (unsigned short[]) pB[0-7] */
    bH = vu16(vec_mergeh(zero, B));

    /* (unsigned short[]) pF[0-7] */
    fH = vu16(vec_mergeh(zero, F));

    /* pB[i] + pF[i] */
    bH = vec_add(bH, fH);

    /* (unsigned short[]) pB[8-15] */
    bL = vu16(vec_mergel(zero, B));

    /* (unsigned short[]) pF[8-15] */
    fL = vu16(vec_mergel(zero, F));

    /* pB[i] + pF[i] */
    bL = vec_add(bL, fL);

    /* (pB[i]+pF[i]) + 1 */
    bH = vec_add(bH, one);
    bL = vec_add(bL, one);
				
    /* (pB[i]+pF[i]+1) >> 1 */
    bH = vec_sra(bH, one);
    bL = vec_sra(bL, one);

    /* d = abs( ((pB[i]+pF[i]+1)>>1) - pR[i] ) */
    bH = vu16(vec_packsu(bH, bL));
    min = vec_min(vu8(bH), R);
    max = vec_max(vu8(bH), R);
    dif = vec_sub(max, min);
				
    /* sum += d * d */
    sum = vec_msum(dif, dif, sum);

    vo.v = vec_sums(vs32(sum), vs32(zero));

    AMBER_STOP;

    return vo.s.sum;
}

#if ALTIVEC_TEST_FUNCTION(bsumsq_sub22)
ALTIVEC_TEST(bsumsq_sub22, int, (BSUMSQ_SUB22_PDECL),
  "blk1f=0x%X, blk1b=0x%X, blk2=0x%X, rowstride=%d, h=%d",
  BSUMSQ_SUB22_ARGS);
#endif
/* vim:set foldmethod=marker foldlevel=0: */
