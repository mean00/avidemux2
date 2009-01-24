/* sad_10.c, this file is part of the
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
 * SAD with vertical half pel interpolation
 *
 * Input requirements:
 *   a) blk2 is always vector aligned
 *   b) rowstride is a multiple of 16
 *   c) h is either 8 or 16
 *
 * s = rowstride;
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 16; i++)
 * 	sum += abs( ((int)(p1[i]+p1[i+s]+1)>>1) - p2[i] );
 *     p1 += s;
 *     p2 += s;
 * }
 */


#define SAD_10_PDECL                                                         \
  uint8_t *blk1,                                                             \
  uint8_t *blk2,                                                             \
  int rowstride,                                                             \
  int h                                                                      \

#define SAD_10_ARGS blk1, blk2, rowstride, h


int sad_10_altivec(SAD_10_PDECL)
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

#ifdef ALTIVEC_VERIFY
  if (NOT_VECTOR_ALIGNED(blk2))
    mjpeg_error_exit1("sad_10: blk2 %% 16 != 0, (%d)", blk2);

  if (NOT_VECTOR_ALIGNED(rowstride))
    mjpeg_error_exit1("sad_10: rowstride %% 16 != 0, (%d)", rowstride);

  if (h != 8 && h != 16)
    mjpeg_error_exit1("sad_10: h != [8|16], (%d)", h);
#endif

    AMBER_START;


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
    /* abs( ((pB[i]+pB[i+s]+1)>>1) - pR[i] ) */                              \
    bH = vu16(vec_packsu(bH, bL));                                            \
    min = vec_min(vu8(bH), lR);                                              \
    max = vec_max(vu8(bH), lR);                                              \
    dif = vec_sub(max, min);                                                 \
                                                                             \
    /* sum += abs(((pB[i]+pB[i+s]+1)>>1)-pR[i]) */                           \
    sum = vec_sum4s(dif, sum);                                               \
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

    AMBER_STOP;

    return vo.s.sum;
}


#if ALTIVEC_TEST_FUNCTION(sad_10)
ALTIVEC_TEST(sad_10, int, (SAD_10_PDECL),
  "blk1=0x%X, blk2=0x%X, rowstride=%d, h=%d",
  SAD_10_ARGS);
#endif
/* vim:set foldmethod=marker foldlevel=0: */
