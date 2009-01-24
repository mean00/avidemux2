/* sad_11.c, this file is part of the
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
 * SAD with horizontal and vertical half pel interpolation
 *
 * Input requirements:
 *   a) blk2 is always vector aligned
 *   b) rowstride is a multiple of 16
 *   c) h is either 8 or 16
 *
 * s = rowstride
 * for (j = 0; j < h; j++) {
 *     for (i = 0; i < 16; i++)
 *         sum += abs( ((int)(pB[i]+pB[i+1]+pB[i+s]+pB[i+s+1]+2)>>2) - pR[i] );
 *     pB += s;
 *     pR += s;
 * }
 */

#define SAD_11_PDECL /* {{{ */                                               \
    uint8_t *blk1,                                                           \
    uint8_t *blk2,                                                           \
    int rowstride,                                                           \
    int h                                                                    \
    /* }}} */
#define SAD_11_ARGS blk1, blk2, rowstride, h
#define SAD_11_PFMT "blk1=0x%X, blk2=0x%X, rowstride=%d, h=%d"

int sad_11_altivec(SAD_11_PDECL)
{
    int i;
    unsigned char *pB, *pR;
    vector unsigned char l0, l1, l2, l3, lR, lB0, lB1, lB2, lB3, perm, perm1;
    vector unsigned short b0H, b0L, b1H, b1L, b2H, b2L, b3H, b3L;
    vector unsigned short bH, bL;
    vector unsigned char max, min, dif;
    vector unsigned int sum;
    vector unsigned char zero;
    vector unsigned short two;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;

#ifdef ALTIVEC_VERIFY
    if (NOT_VECTOR_ALIGNED(blk2))
	mjpeg_error_exit1("sad_11: blk2 %% 16 != 0, (%d)", blk2);

    if (NOT_VECTOR_ALIGNED(rowstride))
	mjpeg_error_exit1("sad_11: rowstride %% 16 != 0, (%d)", rowstride);

    if (h != 8 && h != 16)
	mjpeg_error_exit1("sad_11: h != [8|16], (%d)", h);
#endif

    AMBER_START;

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

/*
 * TODO: some of vec_add()'s might be consolidated since they
 * calculate the same values multiple times.
 */
#define ISAD(b0H,b0L,b1H,b1L,b2H,b2L,b3H,b3L) /* {{{ */                      \
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
    /* abs( ((pB[i]+pB[i+1]+pB[i+s]+pB[i+s+1]+2)>>2) - pR[i] )*/             \
    bH = vu16(vec_packsu(bH, bL));                                            \
    min = vec_min(vu8(bH), lR);                                              \
    max = vec_max(vu8(bH), lR);                                              \
    dif = vec_sub(max, min);                                                 \
                                                                             \
    /* d += abs(((pB[i]+pB[i+1]+pB[i+s]+pB[i+s+1]+2)>>2)-pR[i]) */           \
    sum = vec_sum4s(dif, sum);                                               \
    /* }}} */


    i = (h >> 1) - 1;
    do {
	ISAD(b0H,b0L,b1H,b1L,b2H,b2L,b3H,b3L);
				

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

	ISAD(b2H,b2L,b3H,b3L,b0H,b0L,b1H,b1L);

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

    ISAD(b0H,b0L,b1H,b1L,b2H,b2L,b3H,b3L);
			
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

    ISAD(b2H,b2L,b3H,b3L,b0H,b0L,b1H,b1L);

    vo.v = vec_sums(vs32(sum), vs32(zero));

    AMBER_STOP;

    return vo.s.sum;

#undef ISAD
}

#if ALTIVEC_TEST_FUNCTION(sad_11) /* {{{ */
ALTIVEC_TEST(sad_11, int, (SAD_11_PDECL), SAD_11_PFMT, SAD_11_ARGS);
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
