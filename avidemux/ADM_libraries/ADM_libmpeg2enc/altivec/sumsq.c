/* sumsq.c, this file is part of the
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
 * Input requirements:
 *   b) blk2 is always vector aligned
 *   c) rowstride is a multiple of 16
 *   d) h is either 8 or 16
 */

#define SUMSQ_PDECL \
  uint8_t *blk1,    \
  uint8_t *blk2,    \
  int rowstride,    \
  int hx,           \
  int hy,           \
  int h             \

#define SUMSQ_ARGS blk1, blk2, rowstride, hx, hy, h


/*
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 16; i++) {
 *         d = blk[i] - ref[i];
 *         sum += d * d;
 *     }
 * }
 */
static int sumsq_00(SUMSQ_PDECL) /* {{{ */
{
    int i;
    unsigned char *pblk1, *pblk2;
    vector unsigned char blk1A, blk2A, blk1B, blk2B;
    vector unsigned char blk1A0, blk1B0;
    vector unsigned char blk1A1, blk1B1;
    vector unsigned char minA, minB;
    vector unsigned char maxA, maxB;
    vector unsigned char difA, difB;
    vector unsigned int sum;
    vector signed int zero;
    vector unsigned char perm;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;


    pblk1 = blk1;
    pblk2 = blk2;
    i = (h >> 1) - 1;

    zero = vec_splat_s32(0);
    sum = vec_splat_u32(0);


    if (VECTOR_ALIGNED(pblk1)) {

	blk1A = vec_ld(0, pblk1);
	pblk1 += rowstride;
	blk1B = vec_ld(0, pblk1);

	blk2A = vec_ld(0, pblk2);
	pblk2 += rowstride;
	blk2B = vec_ld(0, pblk2);

	do {
	    maxA = vec_max(blk1A, blk2A);
	    minA = vec_min(blk1A, blk2A);

	    pblk1 += rowstride;
	    blk1A = vec_ld(0, pblk1);
	    pblk2 += rowstride;
	    blk2A = vec_ld(0, pblk2);

	    difA = vec_sub(maxA, minA);
	    sum = vec_msum(difA, difA, sum);

	    maxB = vec_max(blk1B, blk2B);
	    minB = vec_min(blk1B, blk2B);

	    pblk1 += rowstride;
	    blk1B = vec_ld(0, pblk1);
	    pblk2 += rowstride;
	    blk2B = vec_ld(0, pblk2);

	    difB = vec_sub(maxB, minB);
	    sum = vec_msum(difB, difB, sum);
	} while (--i);

    } else {

	perm = vec_lvsl(0, pblk1);

	blk1A0 = vec_ld(0, pblk1);
	blk1A1 = vec_ld(16, pblk1);
	pblk1 += rowstride;
	blk1B0 = vec_ld(0, pblk1);
	blk1B1 = vec_ld(16, pblk1);

	blk2A = vec_ld(0, pblk2);
	pblk2 += rowstride;
	blk2B = vec_ld(0, pblk2);

	do {
	    blk1A = vec_perm(blk1A0, blk1A1, perm);

	    pblk1 += rowstride;
	    blk1A0 = vec_ld(0, pblk1);
	    blk1A1 = vec_ld(16, pblk1);

	    maxA = vec_max(blk1A, blk2A);
	    minA = vec_min(blk1A, blk2A);

	    pblk2 += rowstride;
	    blk2A = vec_ld(0, pblk2);

	    difA = vec_sub(maxA, minA);
	    sum = vec_msum(difA, difA, sum);


	    blk1B = vec_perm(blk1B0, blk1B1, perm);
	    pblk1 += rowstride;
	    blk1B0 = vec_ld(0, pblk1);
	    blk1B1 = vec_ld(16, pblk1);

	    maxB = vec_max(blk1B, blk2B);
	    minB = vec_min(blk1B, blk2B);

	    pblk2 += rowstride;
	    blk2B = vec_ld(0, pblk2);

	    difB = vec_sub(maxB, minB);
	    sum = vec_msum(difB, difB, sum);
	} while (--i);

	blk1A = vec_perm(blk1A0, blk1A1, perm);
	blk1B = vec_perm(blk1B0, blk1B1, perm);

    }

    maxA = vec_max(blk1A, blk2A);
    minA = vec_min(blk1A, blk2A);
    difA = vec_sub(maxA, minA);
    sum = vec_msum(difA, difA, sum);

    maxB = vec_max(blk1B, blk2B);
    minB = vec_min(blk1B, blk2B);
    difB = vec_sub(maxB, minB);
    sum = vec_msum(difB, difB, sum);

    vo.v = vec_sums(vs32(sum), zero);

    AMBER_STOP;

    return vo.s.sum;
} /* }}} */

/*
 * s = rowstride
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 16; i++) {
 * 	d = ((int)(p1[i]+p1[i+1]+1)>>1) - p2[i];
 * 	sum += d * d;
 *     }
 *     p1 += s;
 *     p2 += s;
 * }
 */
static int sumsq_10(SUMSQ_PDECL) /* {{{ */
{
    int i;
    unsigned char *pB, *pR;
    vector unsigned char l0, l1, l2, l3, lR, lB0, lB1, perm0, perm1;
    vector unsigned short b0H, b0L, b1H, b1L;
    vector unsigned short bH, bL;
    vector unsigned char max, min, dif;
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

#define ISAD() /* {{{ */                                                     \
    /* pB[i] + pB[i+1] */                                                    \
    bH = vec_add(b0H, b1H);                                                  \
    bL = vec_add(b0L, b1L);                                                  \
                                                                             \
    /* (pB[i]+pB[i+1]) + 1 */                                                \
    bH = vec_add(bH, one);                                                   \
    bL = vec_add(bL, one);                                                   \
                                                                             \
    /* (pB[i]+pB[i+1]+1) >> 1 */                                             \
    bH = vec_sra(bH, one);                                                   \
    bL = vec_sra(bL, one);                                                   \
                                                                             \
    /* d = abs( ((pB[i]+pB[i+1]+1)>>1) - pR[i] ) */                          \
    bH = vu16(vec_packsu(bH, bL));                                            \
    min = vec_min(vu8(bH), lR);                                              \
    max = vec_max(vu8(bH), lR);                                              \
    dif = vec_sub(max, min);                                                 \
                                                                             \
    /* sum += d * d */                                                       \
    sum = vec_msum(dif, dif, sum);                                           \
    /* }}} */

    pB = blk1,
    pR = blk2;

    l0 = vec_ld(0, pB);
    l1 = vec_ld(16, pB);

    pB += rowstride;
    l2 = vec_ld(0, pB);
    l3 = vec_ld(16, pB);

    lR = vec_ld(0, pR);

    /* initialize constants */
    zero = vec_splat_u8(0);
    one = vec_splat_u16(1);

    sum = vec_splat_u32(0);


    perm0 = vec_lvsl(0, pB);
    perm1 = vec_splat_u8(1);
    perm1 = vec_add(perm0, perm1);


    i = (h >> 1) - 1;
    do { /* while (--i) */

	lB0 = vec_perm(l0, l1, perm0);
	lB1 = vec_perm(l0, l1, perm1);

	pB += rowstride;
	l0 = vec_ld(0, pB);
	l1 = vec_ld(16, pB);

	/* (unsigned short[]) pB[0-7] */
	b0H = vu16(vec_mergeh(zero, lB0));

	/* (unsigned short[]) pB[8-15] */
	b0L = vu16(vec_mergel(zero, lB0));

	/* (unsigned short[]) pB[1-8] */
	b1H = vu16(vec_mergeh(zero, lB1));

	/* (unsigned short[]) pB[9-16] */
	b1L = vu16(vec_mergel(zero, lB1));

	ISAD();

	pR += rowstride;
	lR = vec_ld(0, pR);

	lB0 = vec_perm(l2, l3, perm0);
	lB1 = vec_perm(l2, l3, perm1);

	pB += rowstride;
	l2 = vec_ld(0, pB);
	l3 = vec_ld(16, pB);

	/* (unsigned short[]) pB[0-7] */
	b0H = vu16(vec_mergeh(zero, lB0));

	/* (unsigned short[]) pB[8-15] */
	b0L = vu16(vec_mergel(zero, lB0));

	/* (unsigned short[]) pB[1-8] */
	b1H = vu16(vec_mergeh(zero, lB1));

	/* (unsigned short[]) pB[9-16] */
	b1L = vu16(vec_mergel(zero, lB1));

	ISAD();

	pR += rowstride;
	lR = vec_ld(0, pR);

    } while (--i);

    lB0 = vec_perm(l0, l1, perm0);
    lB1 = vec_perm(l0, l1, perm1);

    /* (unsigned short[]) pB[0-7] */
    b0H = vu16(vec_mergeh(zero, lB0));

    /* (unsigned short[]) pB[8-15] */
    b0L = vu16(vec_mergel(zero, lB0));

    /* (unsigned short[]) pB[1-8] */
    b1H = vu16(vec_mergeh(zero, lB1));

    /* (unsigned short[]) pB[9-16] */
    b1L = vu16(vec_mergel(zero, lB1));

    ISAD();

    pR += rowstride;
    lR = vec_ld(0, pR);

    lB0 = vec_perm(l2, l3, perm0);
    lB1 = vec_perm(l2, l3, perm1);

    /* (unsigned short[]) pB[0-7] */
    b0H = vu16(vec_mergeh(zero, lB0));

    /* (unsigned short[]) pB[8-15] */
    b0L = vu16(vec_mergel(zero, lB0));

    /* (unsigned short[]) pB[1-8] */
    b1H = vu16(vec_mergeh(zero, lB1));

    /* (unsigned short[]) pB[9-16] */
    b1L = vu16(vec_mergel(zero, lB1));

    ISAD();

    vo.v = vec_sums(vs32(sum), vs32(zero));

    return vo.s.sum;

#undef ISAD
} /* }}} */


/*
 * s = rowstride
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 16; i++) {
 * 	d = ((int)(p1[i]+p1[i+s]+1)>>1) - p2[i];
 * 	sum += d * d;
 *     }
 *     p1 += s;
 *     p2 += s;
 * }
 */
static int sumsq_01(SUMSQ_PDECL) /* {{{ */
{
    int i;
    unsigned char *pB, *pR;
    vector unsigned char l0, l1, lR, lB0, lB1, perm;
    vector unsigned short b0H, b0L, b1H, b1L;
    vector unsigned short bH, bL;
    vector unsigned char max, min, dif;
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


#define ISAD() /* {{{ */                                                     \
    /* pB[i] + pB[i+s] */                                                    \
    bH = vec_add(b0H, b1H);                                                  \
    bL = vec_add(b0L, b1L);                                                  \
                                                                             \
    /* (pB[i]+pB[i+s]) + 1 */                                                \
    bH = vec_add(bH, one);                                                   \
    bL = vec_add(bL, one);                                                   \
                                                                             \
    /* (pB[i]+pB[i+s]+1) >> 1 */                                             \
    bH = vec_sra(bH, one);                                                   \
    bL = vec_sra(bL, one);                                                   \
                                                                             \
    /* d = abs( ((pB[i]+pB[i+s]+1)>>1) - pR[i] ) */                          \
    bH = vu16(vec_packsu(bH, bL));                                            \
    min = vec_min(vu8(bH), lR);                                              \
    max = vec_max(vu8(bH), lR);                                              \
    dif = vec_sub(max, min);                                                 \
                                                                             \
    /* sum += d * d */                                                       \
    sum = vec_msum(dif, dif, sum);                                           \
    /* }}} */

    pB = blk1,
    pR = blk2;

    /* initialize constants */
    zero = vec_splat_u8(0);
    one = vec_splat_u16(1);

    sum = vec_splat_u32(0);

    i = (h >> 1) - 1;

    lB0 = vec_ld(0, pB);

    if (VECTOR_ALIGNED(pB)) {

	/* lB0 = vec_ld(0, pB); */
	pB += rowstride;
	lB1 = vec_ld(0, pB);
	pB += rowstride;
	l0 = vec_ld(0, pB);

	lR = vec_ld(0, pR);

	/* (unsigned short[]) pB[0-7] */
	b0H = vu16(vec_mergeh(zero, lB0));

	/* (unsigned short[]) pB[8-15] */
	b0L = vu16(vec_mergel(zero, lB0));

	/* (unsigned short[]) pB+s[0-7] */
	b1H = vu16(vec_mergeh(zero, lB1));

	/* (unsigned short[]) pB+s[8-15] */
	b1L = vu16(vec_mergel(zero, lB1));

	lB0 = vec_sld(l0, l0, 0);

	do { /* while (--i) */
	    pB += rowstride;
	    lB1 = vec_ld(0, pB);

	    ISAD();

	    /* start loading next lR */
	    pR += rowstride;
	    lR = vec_ld(0, pR);

	    /* (unsigned short[]) pB[0-7] */
	    b0H = vu16(vec_mergeh(zero, lB0));

	    /* (unsigned short[]) pB[8-15] */
	    b0L = vu16(vec_mergel(zero, lB0));

	    pB += rowstride;
	    lB0 = vec_ld(0, pB);

	    ISAD();

	    /* start loading next lR */
	    pR += rowstride;
	    lR = vec_ld(0, pR);

	    /* (unsigned short[]) pB[0-7] */
	    b1H = vu16(vec_mergeh(zero, lB1));

	    /* (unsigned short[]) pB[8-15] */
	    b1L = vu16(vec_mergel(zero, lB1));


	} while (--i);

	ISAD();

	pR += rowstride;
	lR = vec_ld(0, pR);

    } else {

	perm = vec_lvsl(0, pB);

	/* lB0 = vec_ld(0, pB); */
	l0 = vec_ld(16, pB);

	pB += rowstride;
	lB1  = vec_ld(0, pB);
	l1 = vec_ld(16, pB);

	lR = vec_ld(0, pR);

	lB0 = vec_perm(lB0, l0, perm);
	lB1 = vec_perm(lB1, l1, perm);

	/* (unsigned short[]) pB[0-7] */
	b0H = vu16(vec_mergeh(zero, lB0));

	/* (unsigned short[]) pB[8-15] */
	b0L = vu16(vec_mergel(zero, lB0));

	/* (unsigned short[]) pB+s[0-7] */
	b1H = vu16(vec_mergeh(zero, lB1));

	/* (unsigned short[]) pB+s[8-15] */
	b1L = vu16(vec_mergel(zero, lB1));

	pB += rowstride;
	l0 = vec_ld(0, pB);
	l1 = vec_ld(16, pB);

	do { /* while (--i) */

	    ISAD();

	    lB0 = vec_perm(l0, l1, perm);

	    pB += rowstride;
	    l0 = vec_ld(0, pB);
	    l1 = vec_ld(16, pB);

	    /* (unsigned short[]) pB[0-7] */
	    b0H = vu16(vec_mergeh(zero, lB0));

	    /* (unsigned short[]) pB[8-15] */
	    b0L = vu16(vec_mergel(zero, lB0));

	    /* start loading next lR */
	    pR += rowstride;
	    lR = vec_ld(0, pR);

	    ISAD();

	    lB1 = vec_perm(l0, l1, perm);

	    pB += rowstride;
	    l0 = vec_ld(0, pB);
	    l1 = vec_ld(16, pB);

	    /* (unsigned short[]) pB[0-7] */
	    b1H = vu16(vec_mergeh(zero, lB1));

	    /* (unsigned short[]) pB[8-15] */
	    b1L = vu16(vec_mergel(zero, lB1));


	    /* start loading next lR */
	    pR += rowstride;
	    lR = vec_ld(0, pR);

	} while (--i);

	ISAD();

	pR += rowstride;
	lR = vec_ld(0, pR);

	lB0 = vec_perm(l0, l1, perm);
    }


    /* (unsigned short[]) pB[0-7] */
    b0H = vu16(vec_mergeh(zero, lB0));

    /* (unsigned short[]) pB[8-15] */
    b0L = vu16(vec_mergel(zero, lB0));

    ISAD();

    vo.v = vec_sums(vs32(sum), vs32(zero));

    return vo.s.sum;

#undef ISAD
} /* }}} */


/*
 * s = rowstride
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 16; i++)
 *         d = ((int)(pB[i]+pB[i+1]+pB[i+s]+pB[i+s+1]+2)>>2) - pR[i];
 *         sum += d * d;
 *     pB += s;
 *     pR += s;
 * }
 */
static int sumsq_11(SUMSQ_PDECL) /* {{{ */
{
    int i;
    unsigned char *pB, *pR;
    vector unsigned char l0, l1, l2, l3, lR, lB0, lB1, lB2, lB3, perm, perm1;
    vector unsigned short b0H, b0L, b1H, b1L, b2H, b2L, b3H, b3L;
    vector unsigned short bH, bL;
    vector unsigned char zero;
    vector unsigned short two;
    vector unsigned char max, min, dif;
    vector unsigned int sum;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;


    pB = blk1,
    pR = blk2;

    /* start loading first blocks */
    l0 = vec_ld(0, pB);
    l1 = vec_ld(16, pB);
    pB += rowstride;
    l2 = vec_ld(0, pB);
    l3 = vec_ld(16, pB);

    /* initialize constants */
    zero = vec_splat_u8(0);
    two  = vec_splat_u16(2);

    sum = vec_splat_u32(0);


    perm = vec_lvsl(0, blk1);
    perm1 = vec_splat_u8(1);
    perm1 = vec_add(perm, perm1);

    /* permute 1st set of loaded blocks  */
    lB0 = vec_perm(l0, l1, perm);
    lB1 = vec_perm(l0, l1, perm1);

    /* start loading 3rd set */
    pB += rowstride;
    l0 = vec_ld(0, pB);
    l1 = vec_ld(16, pB);

    /* permute 2nd set of loaded blocks  */
    lB2 = vec_perm(l2, l3, perm);
    lB3 = vec_perm(l2, l3, perm1);

    /* start loading lR */
    lR = vec_ld(0, pR);

    /* (unsigned short[]) pB[0-7] */
    b0H = vu16(vec_mergeh(zero, lB0));

    /* (unsigned short[]) pB[8-15] */
    b0L = vu16(vec_mergel(zero, lB0));

    /* (unsigned short[]) pB[1-8] */
    b1H = vu16(vec_mergeh(zero, lB1));

    /* (unsigned short[]) pB[9-16] */
    b1L = vu16(vec_mergel(zero, lB1));

    /* (unsigned short[]) pB+s[0-7] */
    b2H = vu16(vec_mergeh(zero, lB2));
			
    /* (unsigned short[]) pB+s[8-15] */
    b2L = vu16(vec_mergel(zero, lB2));
			
    /* (unsigned short[]) pB+s[1-8] */
    b3H = vu16(vec_mergeh(zero, lB3));
			
    /* (unsigned short[]) pB+s[9-16] */
    b3L = vu16(vec_mergel(zero, lB3));

#define ISUMSQ(b0H,b0L,b1H,b1L,b2H,b2L,b3H,b3L) /* {{{ */                    \
    /* pB[i] + pB[i+1] */                                                    \
    bH = vec_add(b0H, b1H);                                                  \
    bL = vec_add(b0L, b1L);                                                  \
                                                                             \
    /* (pB[i]+pB[i+1]) + pB[i+s] */                                          \
    bH = vec_add(bH, b2H);                                                   \
    bL = vec_add(bL, b2L);                                                   \
                                                                             \
    /* (pB[i]+pB[i+1]+pB[i+s]) + pB[i+s+1] */                                \
    bH = vec_add(bH, b3H);                                                   \
    bL = vec_add(bL, b3L);                                                   \
                                                                             \
    /* (pB[i]+pB[i+1]+pB[i+s]+pB[i+s+1]) + 2 */                              \
    bH = vec_add(bH, two);                                                   \
    bL = vec_add(bL, two);                                                   \
                                                                             \
    /* (pB[i]+pB[i+1]+pB[i+s]+pB[i+s+1]+2) >> 2 */                           \
    bH = vec_sra(bH, two);                                                   \
    bL = vec_sra(bL, two);                                                   \
                                                                             \
    /* absolute value is used increase parallelism, x16 instead of x8 */     \
    /* d = abs( ((int)(pB[i]+pB[i+1]+pB[i+s]+pB[i+s+1]+2)>>2) - pR[i] ) */   \
    bH = vu16(vec_packsu(bH, bL));                                            \
    min = vec_min(vu8(bH), lR);                                              \
    max = vec_max(vu8(bH), lR);                                              \
    dif = vec_sub(max, min);                                                 \
                                                                             \
    /* sum += d * d; */                                                      \
    sum = vec_msum(dif, dif, sum);                                           \
    /* }}} */


    i = (h >> 1) - 1;
    do {
	ISUMSQ(b0H,b0L,b1H,b1L,b2H,b2L,b3H,b3L);
				

	/* start loading next lR */
	pR += rowstride;
	lR = vec_ld(0, pR);
				
	/* perm loaded set */
	lB0 = vec_perm(l0, l1, perm);
	lB1 = vec_perm(l0, l1, perm1);

	/* start loading next set */
	pB += rowstride;
	l0 = vec_ld(0, pB);
	l1 = vec_ld(16, pB);
				

	/* (unsigned short[]) pB[0-7] */
	b0H = vu16(vec_mergeh(zero, lB0));

	/* (unsigned short[]) pB[8-15] */
	b0L = vu16(vec_mergel(zero, lB0));

	/* (unsigned short[]) pB[1-8] */
	b1H = vu16(vec_mergeh(zero, lB1));

	/* (unsigned short[]) pB[9-16] */
	b1L = vu16(vec_mergel(zero, lB1));

	ISUMSQ(b2H,b2L,b3H,b3L,b0H,b0L,b1H,b1L);


	/* start loading next lR */
	pR += rowstride;
	lR = vec_ld(0, pR);
				
	/* perm loaded set */
	lB2 = vec_perm(l0, l1, perm);
	lB3 = vec_perm(l0, l1, perm1);

	/* start loading next set */
	pB += rowstride;
	l0 = vec_ld(0, pB);
	l1 = vec_ld(16, pB);

					
	/* (unsigned short[]) pB+s[0-7] */
	b2H = vu16(vec_mergeh(zero, lB2));

	/* (unsigned short[]) pB+s[8-15] */
	b2L = vu16(vec_mergel(zero, lB2));

	/* (unsigned short[]) pB+s[1-8] */
	b3H = vu16(vec_mergeh(zero, lB3));

	/* (unsigned short[]) pB+s[9-16] */
	b3L = vu16(vec_mergel(zero, lB3));
    } while (--i);

    ISUMSQ(b0H,b0L,b1H,b1L,b2H,b2L,b3H,b3L);

    pR += rowstride;
    lR = vec_ld(0, pR);
			
    lB0 = vec_perm(l0, l1, perm);
    lB1 = vec_perm(l0, l1, perm1);
			
    /* (unsigned short[]) pB[0-7] */
    b0H = vu16(vec_mergeh(zero, lB0));

    /* (unsigned short[]) pB[8-15] */
    b0L = vu16(vec_mergel(zero, lB0));

    /* (unsigned short[]) pB[1-8] */
    b1H = vu16(vec_mergeh(zero, lB1));

    /* (unsigned short[]) pB[9-16] */
    b1L = vu16(vec_mergel(zero, lB1));

    ISUMSQ(b2H,b2L,b3H,b3L,b0H,b0L,b1H,b1L);

    vo.v = vec_sums(vs32(sum), vs32(zero));

    return vo.s.sum;

#undef ISUMSQ
} /* }}} */

int sumsq_altivec(SUMSQ_PDECL)
{
    int sumsq;

#ifdef ALTIVEC_VERIFY
    if (NOT_VECTOR_ALIGNED(blk2))
	mjpeg_error_exit1("sumsq: blk2 %% 16 != 0, (%d)", blk2);

    if (NOT_VECTOR_ALIGNED(rowstride))
	mjpeg_error_exit1("sumsq: rowstride %% 16 != 0, (%d)", rowstride);

    if (h != 8 && h != 16)
	mjpeg_error_exit1("sumsq: h != [8|16], (%d)", h);
#endif

    AMBER_START;

    if (hx == 0) {
	if (hy == 0)
	    sumsq = sumsq_00(SUMSQ_ARGS);
	else
	    sumsq = sumsq_01(SUMSQ_ARGS);
    } else {
	if (hy == 0)
	    sumsq = sumsq_10(SUMSQ_ARGS);
	else
	    sumsq = sumsq_11(SUMSQ_ARGS);
    }

    AMBER_STOP;

    return sumsq;
}

#if ALTIVEC_TEST_FUNCTION(sumsq)
ALTIVEC_TEST(sumsq, int, (SUMSQ_PDECL),
  "blk1=0x%x, blk2=0x%x, rowstride=%d, hx=%d, hy=%d, h=%d",
  SUMSQ_ARGS);
#endif
/* vim:set foldmethod=marker foldlevel=0: */
