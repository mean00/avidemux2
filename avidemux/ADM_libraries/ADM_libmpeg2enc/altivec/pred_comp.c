/* pred_comp.c, this file is part of the
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

#include "altivec_predict.h"
#include "vectorize.h"
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif

#define PRED_COMP_PDECL /* {{{ */                                            \
    uint8_t *src,                                                            \
    uint8_t *dst,                                                            \
    int lx,                                                                  \
    int w, int h,                                                            \
    int x, int y,                                                            \
    int dx, int dy,                                                          \
    int addflag                                                              \
    /* }}} */
#define PRED_COMP_ARGS src, dst, lx, w, h, x, y, dx, dy, addflag
#define PRED_COMP_PFMT /* {{{ */                                             \
    "src=0x%X, dst=0x%X, lx=%d, w=%d, h=%d, x=%d, y=%d,"                     \
    " dx=%d, dy=%d, addflag=%d"                                              \
    /* }}} */

/* low level prediction routine
 *
 * src:     prediction source
 * dst:     prediction destination
 * lx:      line width (for both src and dst)
 * x,y:     destination coordinates
 * dx,dy:   half pel motion vector
 * w,h:     size of prediction block
 * addflag: store or add prediction
 *
 * src % 16 == 0
 * dst % 16 == 0
 * lx % 16 == 0
 * w == 16 | 8
 * h == 16 | 8 | 4 interlaced
 * w == h | h*2 interlaced
 */
void pred_comp_altivec(PRED_COMP_PDECL)
{
    int xint, xh, yint, yh;
    int i, j;
    uint8_t *s, *d;
    vector unsigned char lS, lS0, lS1, lS2, lS3, lD, perm, perm1;
    vector unsigned short s0H, s0L, s1H, s1L, s2H, s2L, s3H, s3L, dH, dL;
    vector unsigned char zero, eight;
    vector unsigned short one, two;

    AMBER_START;

    /* half pel scaling */
    xint = dx>>1; /* integer part */
    xh = dx & 1;  /* half pel flag */
    yint = dy>>1;
    yh = dy & 1;

    /* origins */
    s = src + lx*(y+yint) + (x+xint); /* motion vector */
    d = dst + lx*y + x;

#ifdef ALTIVEC_VERIFY /* {{{ */
    if (NOT_VECTOR_ALIGNED(src))
	mjpeg_error_exit1("pred_comp: src %% 16 != 0, (0x%X)", src);

    if (NOT_VECTOR_ALIGNED(dst))
	mjpeg_error_exit1("pred_comp: dst %% 16 != 0, (0x%X)", dst);

    if (NOT_VECTOR_ALIGNED(lx))
	mjpeg_error_exit1("pred_comp: lx %% 16 != 0, (%d)", lx);

    if (w != 16 && w != 8)
	mjpeg_error_exit1("pred_comp: w != 16|8, (%d)", w);

    if (h != 16 && h != 8 && h != 4)
	mjpeg_error_exit1("pred_comp: h != 16|8|4, (%d)", h);

    if (w != h && w != (h * 2))
	mjpeg_error_exit1("pred_comp: w != h|(h*2), (w=%d, h=%d)", w, h);

#ifdef ALTIVEC_DST
    if (lx & (~0xffff) != 0)
	mjpeg_error_exit1("pred_comp: lx > vec_dst range", lx);
#endif

    if (((unsigned long)d & 0x7) != 0)
	mjpeg_error_exit1("pred_comp: d %% 8 != 0 (0x%X)", d);

    if (((unsigned long)d & 0xf) > w)
	mjpeg_error_exit1("pred_comp: (d & 0xf) > w (d=0x%X, w=%d)", d, w);
#endif /* }}} */

    if (xh) {
	if  (yh) {
	    /* (xh && yh) {{{ */
	    if (addflag) {
		/* {{{ */
		zero = vec_splat_u8(0);
		one = vec_splat_u16(1);
		two = vec_splat_u16(2);

		if (w == 8) {
		    /* {{{ */
		    eight = vec_splat_u8(8);

		    perm = vec_lvsl(0, (unsigned char*)s);

		    if (((unsigned long)d & 0xf) == 0) {
			/* {{{ */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			lS1 = vec_slo(lS0, eight);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);
			    lS3 = vec_slo(lS2, eight);

			    lD = vec_ld(0, (unsigned char*)d);


			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) s+lx[1-8] */
			    s3H = vu16(vec_mergeh(zero, lS3));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* s[i+lx] + s[i+lx+1] */
			    s2H = vec_add(s2H, s3H);

			    /* (s[i]+s[i+1]) + (s[i+lx]+s[i+lx+1]) */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]) + 2 */
			    s0H = vec_add(s0H, two);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2) >> 2 */
			    s0H = vec_sra(s0H, two);

			    /* d[i] + ((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2) */
			    dH = vec_add(dH, s0H);

			    /* (d[i]+((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2))
			     *  + 1
			     */
			    dH = vec_add(dH, one);

			    /* ((d[i]+((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2))
			     *  + 1) >> 1
			     */
			    dH = vec_sra(dH, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;
			    lS1 = lS3;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    } else {
			/* {{{ */
			/* d is offset 8 bytes, work on dL (low bytes).
			 */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			lS1 = vec_slo(lS0, eight);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);
			    lS3 = vec_slo(lS2, eight);

			    lD = vec_ld(0, (unsigned char*)d);


			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) s+lx[1-8] */
			    s3H = vu16(vec_mergeh(zero, lS3));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* s[i+lx] + s[i+lx+1] */
			    s2H = vec_add(s2H, s3H);

			    /* (s[i]+s[i+1]) + (s[i+lx]+s[i+lx+1]) */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]) + 2 */
			    s0H = vec_add(s0H, two);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2) >> 2 */
			    s0H = vec_sra(s0H, two);

			    /* d[i] + ((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2) */
			    dL = vec_add(dL, s0H);

			    /* (d[i]+((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2))
			     *  + 1
			     */
			    dL = vec_add(dL, one);

			    /* ((d[i]+((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2))
			     *  + 1) >> 1
			     */
			    dL = vec_sra(dL, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;
			    lS1 = lS3;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    }
		    /* }}} */
		} else /* w == 16 */ {
		    /* {{{ */
		    perm = vec_lvsl(0, s);
		    perm1 = vec_splat_u8(1);
		    perm1 = vec_add(perm, perm1);

		    lS = vec_ld(0, (unsigned char*)s);
		    lS1 = vec_ld(16, (unsigned char*)s);
		    lS0 = vec_perm(lS, lS1, perm);
		    lS1 = vec_perm(lS, lS1, perm1);
		    s += lx;

		    for (i = 0; i < h; i++) {
			lS = vec_ld(0, (unsigned char*)s);
			lS3 = vec_ld(16, (unsigned char*)s);
			lS2 = vec_perm(lS, lS3, perm);
			lS3 = vec_perm(lS, lS3, perm1);

			lD = vec_ld(0, (unsigned char*)d);

			/* (unsigned short[]) s[0-7] */
			s0H = vu16(vec_mergeh(zero, lS0));

			/* (unsigned short[]) s[8-15] */
			s0L = vu16(vec_mergel(zero, lS0));

			/* (unsigned short[]) s[1-8] */
			s1H = vu16(vec_mergeh(zero, lS1));

			/* (unsigned short[]) s[9-16] */
			s1L = vu16(vec_mergel(zero, lS1));

			/* (unsigned short[]) s+lx[0-7] */
			s2H = vu16(vec_mergeh(zero, lS2));

			/* (unsigned short[]) s+lx[8-15] */
			s2L = vu16(vec_mergel(zero, lS2));

			/* (unsigned short[]) s+lx[1-8] */
			s3H = vu16(vec_mergeh(zero, lS3));

			/* (unsigned short[]) s+lx[9-16] */
			s3L = vu16(vec_mergel(zero, lS3));

			/* (unsigned short[]) d[0-7] */
			dH = vu16(vec_mergeh(zero, lD));

			/* (unsigned short[]) d[8-15] */
			dL = vu16(vec_mergel(zero, lD));

			/* s[i] + s[i+1] */
			s0H = vec_add(s0H, s1H);
			s0L = vec_add(s0L, s1L);

			/* s[i+lx] + s[i+lx+1] */
			s2H = vec_add(s2H, s3H);
			s2L = vec_add(s2L, s3L);

			/* (s[i]+s[i+1]) + (s[i+lx]+s[i+lx+1]) */
			s0H = vec_add(s0H, s2H);
			s0L = vec_add(s0L, s2L);

			/* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]) + 2 */
			s0H = vec_add(s0H, two);
			s0L = vec_add(s0L, two);

			/* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2) >> 2 */
			s0H = vec_sra(s0H, two);
			s0L = vec_sra(s0L, two);

			/* d[i] + ((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2) */
			dH = vec_add(dH, s0H);
			dL = vec_add(dL, s0L);

			/* (d[i]+((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2))
			 *  + 1
			 */
			dH = vec_add(dH, one);
			dL = vec_add(dL, one);

			/* ((d[i]+((s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2))
			 *  + 1) >> 1
			 */
			dH = vec_sra(dH, one);
			dL = vec_sra(dL, one);

			/* (unsigned char[]) d[0-7], d[8-15] */
			dH = vu16(vec_packsu(dH, dL));
			vec_st(vu8(dH), 0, (unsigned char*)d);

			lS0 = lS2;
			lS1 = lS3;

			s += lx;
			d += lx;
		    }
		    /* }}} */
		}
#if 0
		for (j=0; j<h; j++)
		{
		    for (i=0; i<w; i++)
			d[i] = (d[i] + ((unsigned int)(s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2)+1)>>1;
		    s+= lx;
		    d+= lx;
		}
#endif
		/* }}} */
	    } else /* !addflag */ {
		/* {{{ */
		zero = vec_splat_u8(0);
		one = vec_splat_u16(1);
		two = vec_splat_u16(2);

		if (w == 8) {
		    /* {{{ */
		    eight = vec_splat_u8(8);

		    perm = vec_lvsl(0, (unsigned char*)s);

		    if (((unsigned long)d & 0xf) == 0) {
			/* {{{ */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			lS1 = vec_slo(lS0, eight);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);
			    lS3 = vec_slo(lS2, eight);

			    lD = vec_ld(0, (unsigned char*)d);


			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) s+lx[1-8] */
			    s3H = vu16(vec_mergeh(zero, lS3));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* s[i+lx] + s[i+lx+1] */
			    s2H = vec_add(s2H, s3H);

			    /* (s[i]+s[i+1]) + (s[i+lx]+s[i+lx+1]) */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]) + 2 */
			    s0H = vec_add(s0H, two);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2) >> 2 */
			    s0H = vec_sra(s0H, two);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(s0H, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;
			    lS1 = lS3;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    } else {
			/* {{{ */
			/* d is offset 8 bytes, work on dL (low bytes).
			 */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			lS1 = vec_slo(lS0, eight);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);
			    lS3 = vec_slo(lS2, eight);

			    lD = vec_ld(0, (unsigned char*)d);


			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) s+lx[1-8] */
			    s3H = vu16(vec_mergeh(zero, lS3));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* s[i+lx] + s[i+lx+1] */
			    s2H = vec_add(s2H, s3H);

			    /* (s[i]+s[i+1]) + (s[i+lx]+s[i+lx+1]) */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]) + 2 */
			    s0H = vec_add(s0H, two);

			    /* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2) >> 2 */
			    s0H = vec_sra(s0H, two);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, s0H));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;
			    lS1 = lS3;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    }
		    /* }}} */
		} else /* w == 16 */ {
		    /* {{{ */
		    perm = vec_lvsl(0, s);
		    perm1 = vec_splat_u8(1);
		    perm1 = vec_add(perm, perm1);

		    lS = vec_ld(0, (unsigned char*)s);
		    lS1 = vec_ld(16, (unsigned char*)s);
		    lS0 = vec_perm(lS, lS1, perm);
		    lS1 = vec_perm(lS, lS1, perm1);
		    s += lx;

		    for (i = 0; i < h; i++) {
			lS = vec_ld(0, (unsigned char*)s);
			lS3 = vec_ld(16, (unsigned char*)s);
			lS2 = vec_perm(lS, lS3, perm);
			lS3 = vec_perm(lS, lS3, perm1);

			/* (unsigned short[]) s[0-7] */
			s0H = vu16(vec_mergeh(zero, lS0));

			/* (unsigned short[]) s[8-15] */
			s0L = vu16(vec_mergel(zero, lS0));

			/* (unsigned short[]) s[1-8] */
			s1H = vu16(vec_mergeh(zero, lS1));

			/* (unsigned short[]) s[9-16] */
			s1L = vu16(vec_mergel(zero, lS1));

			/* (unsigned short[]) s+lx[0-7] */
			s2H = vu16(vec_mergeh(zero, lS2));

			/* (unsigned short[]) s+lx[8-15] */
			s2L = vu16(vec_mergel(zero, lS2));

			/* (unsigned short[]) s+lx[1-8] */
			s3H = vu16(vec_mergeh(zero, lS3));

			/* (unsigned short[]) s+lx[9-16] */
			s3L = vu16(vec_mergel(zero, lS3));

			/* s[i] + s[i+1] */
			s0H = vec_add(s0H, s1H);
			s0L = vec_add(s0L, s1L);

			/* s[i+lx] + s[i+lx+1] */
			s2H = vec_add(s2H, s3H);
			s2L = vec_add(s2L, s3L);

			/* (s[i]+s[i+1]) + (s[i+lx]+s[i+lx+1]) */
			s0H = vec_add(s0H, s2H);
			s0L = vec_add(s0L, s2L);

			/* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]) + 2 */
			s0H = vec_add(s0H, two);
			s0L = vec_add(s0L, two);

			/* (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2) >> 2 */
			s0H = vec_sra(s0H, two);
			s0L = vec_sra(s0L, two);

			/* (unsigned char[]) d[0-7], d[8-15] */
			dH = vu16(vec_packsu(s0H, s0L));
			vec_st(vu8(dH), 0, (unsigned char*)d);

			lS0 = lS2;
			lS1 = lS3;

			s += lx;
			d += lx;
		    }
		    /* }}} */
		}
#if 0
		for (j=0; j<h; j++)
		{
		    for (i=0; i<w; i++)
			d[i] = (unsigned int)(s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2;
		    s+= lx;
		    d+= lx;
		}
#endif
		/* }}} */
	    }
	    /* }}} */
	} else /* !yh */ {
	    /* (xh && !yh) {{{ */
	    if (addflag) {
		/* {{{ */
		zero = vec_splat_u8(0);
		one = vec_splat_u16(1);

		if (w == 8) {
		    /* {{{ */
		    eight = vec_splat_u8(8);

		    perm = vec_lvsl(0, (unsigned char*)s);

		    if (((unsigned long)d & 0xf) == 0) {
			for (i = 0; i < h; i++) {
			    lS0 = vec_ld(0, (unsigned char*)s);
			    lS1 = vec_ld(16, (unsigned char*)s);
			    lS0 = vec_perm(lS0, lS1, perm);
			    lS1 = vec_slo(lS0, eight);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* (s[i]+s[i+1]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+1]+1) >> 1 */
			    s0H = vec_sra(s0H, one);

			    /* d[i] + ((s[i]+s[i+1]+1)>>1) */
			    dH = vec_add(dH, s0H);

			    /* (d[i]+((s[i]+s[i+1]+1)>>1)) + 1 */
			    dH = vec_add(dH, one);

			    /* ((d[i]+((s[i]+s[i+1]+1)>>1))+1) >> 1 */
			    dH = vec_sra(dH, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    } else {
			/* {{{ */
			/* d is offset 8 bytes, work on dL (low bytes).
			 */
			for (i = 0; i < h; i++) {
			    lS0 = vec_ld(0, (unsigned char*)s);
			    lS1 = vec_ld(16, (unsigned char*)s);
			    lS0 = vec_perm(lS0, lS1, perm);
			    lS1 = vec_slo(lS0, eight);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* (s[i]+s[i+1]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+1]+1) >> 1 */
			    s0H = vec_sra(s0H, one);

			    /* d[i] + ((s[i]+s[i+1]+1)>>1) */
			    dL = vec_add(dL, s0H);

			    /* (d[i]+((s[i]+s[i+1]+1)>>1)) + 1 */
			    dL = vec_add(dL, one);

			    /* ((d[i]+((s[i]+s[i+1]+1)>>1))+1) >> 1 */
			    dL = vec_sra(dL, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    }
		} else /* w == 16 */ {
		    /* {{{ */
		    perm = vec_lvsl(0, s);
		    perm1 = vec_splat_u8(1);
		    perm1 = vec_add(perm, perm1);

		    for (i = 0; i < h; i++) {
			lS = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS, lS1, perm);
			lS1 = vec_perm(lS, lS1, perm1);

			lD = vec_ld(0, (unsigned char*)d);

			/* (unsigned short[]) s[0-7] */
			s0H = vu16(vec_mergeh(zero, lS0));

			/* (unsigned short[]) s[8-15] */
			s0L = vu16(vec_mergel(zero, lS0));

			/* (unsigned short[]) s[1-8] */
			s1H = vu16(vec_mergeh(zero, lS1));

			/* (unsigned short[]) s[9-16] */
			s1L = vu16(vec_mergel(zero, lS1));

			/* (unsigned short[]) d[0-7] */
			dH = vu16(vec_mergeh(zero, lD));

			/* (unsigned short[]) d[8-15] */
			dL = vu16(vec_mergel(zero, lD));

			/* s[i] + s[i+1] */
			s0H = vec_add(s0H, s1H);
			s0L = vec_add(s0L, s1L);

			/* (s[i]+s[i+1]) + 1 */
			s0H = vec_add(s0H, one);
			s0L = vec_add(s0L, one);

			/* (s[i]+s[i+1]+1) >> 1 */
			s0H = vec_sra(s0H, one);
			s0L = vec_sra(s0L, one);

			/* d[i] + ((s[i]+s[i+1]+1)>>1) */
			dH = vec_add(dH, s0H);
			dL = vec_add(dL, s0L);

			/* (d[i]+((s[i]+s[i+1]+1)>>1)) + 1 */
			dH = vec_add(dH, one);
			dL = vec_add(dL, one);

			/* ((d[i]+((s[i]+s[i+1]+1)>>1))+1) >> 1 */
			dH = vec_sra(dH, one);
			dL = vec_sra(dL, one);

			/* (unsigned char[]) d[0-7], d[8-15] */
			dH = vu16(vec_packsu(dH, dL));
			vec_st(vu8(dH), 0, (unsigned char*)d);

			s += lx;
			d += lx;
		    }
#if 0
		    for (j=0; j<h; j++)
		    {
			for (i=0; i<w; i++)
			    d[i] = (d[i] +
				    ((unsigned int)(s[i]+s[i+1]+1)>>1)+1)>>1;
			s += lx;
			d += lx;
		    }
#endif
		    /* }}} */
		}
		/* }}} */
	    } else /* !addflag */ {
		/* {{{ */
		zero = vec_splat_u8(0);
		one = vec_splat_u16(1);

		if (w == 8) {
		    /* {{{ */
		    eight = vec_splat_u8(8);

		    perm = vec_lvsl(0, (unsigned char*)s);

		    if (((unsigned long)d & 0xf) == 0) {
			/* {{{ */
			for (i = 0; i < h; i++) {
			    lS0 = vec_ld(0, (unsigned char*)s);
			    lS1 = vec_ld(16, (unsigned char*)s);
			    lS0 = vec_perm(lS0, lS1, perm);
			    lS1 = vec_slo(lS0, eight);

			    lD = vec_ld(0, (unsigned char*)d);


			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* (s[i]+s[i+1]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+1]+1) >> 1 */
			    s0H = vec_sra(s0H, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(s0H, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    } else {
			/* {{{ */
			/* d is offset 8 bytes, work on dL (low bytes).
			 */
			for (i = 0; i < h; i++) {
			    lS0 = vec_ld(0, (unsigned char*)s);
			    lS1 = vec_ld(16, (unsigned char*)s);
			    lS0 = vec_perm(lS0, lS1, perm);
			    lS1 = vec_slo(lS0, eight);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s[1-8] */
			    s1H = vu16(vec_mergeh(zero, lS1));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* s[i] + s[i+1] */
			    s0H = vec_add(s0H, s1H);

			    /* (s[i]+s[i+1]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+1]+1) >> 1 */
			    s0H = vec_sra(s0H, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, s0H));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    }
		    /* }}} */
		} else /* w == 16 */ {
		    /* {{{ */
		    perm = vec_lvsl(0, s);
		    perm1 = vec_splat_u8(1);
		    perm1 = vec_add(perm, perm1);

		    for (i = 0; i < h; i++) {
			lS = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS, lS1, perm);
			lS1 = vec_perm(lS, lS1, perm1);


			/* (unsigned short[]) s[0-7] */
			s0H = vu16(vec_mergeh(zero, lS0));

			/* (unsigned short[]) s[8-15] */
			s0L = vu16(vec_mergel(zero, lS0));

			/* (unsigned short[]) s[1-8] */
			s1H = vu16(vec_mergeh(zero, lS1));

			/* (unsigned short[]) s[9-16] */
			s1L = vu16(vec_mergel(zero, lS1));

			/* s[i] + s[i+1] */
			s0H = vec_add(s0H, s1H);
			s0L = vec_add(s0L, s1L);

			/* (s[i]+s[i+1]) + 1 */
			s0H = vec_add(s0H, one);
			s0L = vec_add(s0L, one);

			/* (s[i]+s[i+1]+1) >> 1 */
			s0H = vec_sra(s0H, one);
			s0L = vec_sra(s0L, one);

			/* (unsigned char[]) d[0-7], d[8-15] */
			dH = vu16(vec_packsu(s0H, s0L));
			vec_st(vu8(dH), 0, (unsigned char*)d);

			s += lx;
			d += lx;
		    }
		    /* }}} */
		}
#if 0
		for (j=0; j<h; j++)
		{
		    for (i=0; i<w; i++)
			d[i] = (unsigned int)(s[i]+s[i+1]+1)>>1;
		    s+= lx;
		    d+= lx;
		}
#endif
		/* }}} */
	    }
	    /* }}} */
	}
    } else /* !xh */ {
	if  (yh) {
	    /* (!xh && yh) {{{ */
	    if (addflag) {
		/* {{{ */
		zero = vec_splat_u8(0);
		one = vec_splat_u16(1);

		if (w == 8) {
		    /* {{{ */

		    perm = vec_lvsl(0, (unsigned char*)s);

		    if (((unsigned long)d & 0xf) == 0) {
			/* {{{ */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+lx] */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+lx]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+lx]+1) >> 1 */
			    s0H = vec_sra(s0H, one);

			    /* d[i] + ((s[i]+s[i+lx]+1)>>1) */
			    dH = vec_add(dH, s0H);

			    /* (d[i]+((s[i]+s[i+lx]+1)>>1)) + 1 */
			    dH = vec_add(dH, one);

			    /* ((d[i]+((s[i]+s[i+lx]+1)>>1))+1) >> 1 */
			    dH = vec_sra(dH, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    } else {
			/* {{{ */
			/* d is offset 8 bytes, work on dL (low bytes).
			 */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+lx] */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+lx]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+lx]+1) >> 1 */
			    s0H = vec_sra(s0H, one);

			    /* d[i] + ((s[i]+s[i+lx]+1)>>1) */
			    dL = vec_add(dL, s0H);

			    /* (d[i]+((s[i]+s[i+lx]+1)>>1)) + 1 */
			    dL = vec_add(dL, one);

			    /* ((d[i]+((s[i]+s[i+lx]+1)>>1))+1) >> 1 */
			    dL = vec_sra(dL, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    }
		    /* }}} */
		} else /* w == 16 */ {
		    /* {{{ */

		    perm = vec_lvsl(0, s);

		    lS = vec_ld(0, (unsigned char*)s);
		    lS1 = vec_ld(16, (unsigned char*)s);
		    lS0 = vec_perm(lS, lS1, perm);
		    s += lx;

		    for (i = 0; i < h; i++) {
			lS = vec_ld(0, (unsigned char*)s);
			lS3 = vec_ld(16, (unsigned char*)s);
			lS2 = vec_perm(lS, lS3, perm);

			lD = vec_ld(0, (unsigned char*)d);

			/* (unsigned short[]) s[0-7] */
			s0H = vu16(vec_mergeh(zero, lS0));

			/* (unsigned short[]) s[8-15] */
			s0L = vu16(vec_mergel(zero, lS0));

			/* (unsigned short[]) s+lx[0-7] */
			s2H = vu16(vec_mergeh(zero, lS2));

			/* (unsigned short[]) s+lx[8-15] */
			s2L = vu16(vec_mergel(zero, lS2));

			/* (unsigned short[]) d[0-7] */
			dH = vu16(vec_mergeh(zero, lD));

			/* (unsigned short[]) d[8-15] */
			dL = vu16(vec_mergel(zero, lD));

			/* s[i] + s[i+lx] */
			s0H = vec_add(s0H, s2H);
			s0L = vec_add(s0L, s2L);

			/* (s[i]+s[i+lx]) + 1 */
			s0H = vec_add(s0H, one);
			s0L = vec_add(s0L, one);

			/* (s[i]+s[i+lx]+1) >> 1 */
			s0H = vec_sra(s0H, one);
			s0L = vec_sra(s0L, one);

			/* d[i] + ((s[i]+s[i+lx]+1)>>1) */
			dH = vec_add(dH, s0H);
			dL = vec_add(dL, s0L);

			/* (d[i]+((s[i]+s[i+lx]+1)>>1)) + 1 */
			dH = vec_add(dH, one);
			dL = vec_add(dL, one);

			/* ((d[i]+((s[i]+s[i+lx]+1)>>1))+1) >> 1 */
			dH = vec_sra(dH, one);
			dL = vec_sra(dL, one);

			/* (unsigned char[]) d[0-7], d[8-15] */
			dH = vu16(vec_packsu(dH, dL));
			vec_st(vu8(dH), 0, (unsigned char*)d);

			lS0 = lS2;

			s += lx;
			d += lx;
		    }
		    /* }}} */
		}
#if 0
		for (j=0; j<h; j++)
		{
		    for (i=0; i<w; i++)
			d[i] = (d[i] + ((unsigned int)(s[i]+s[i+lx]+1)>>1)+1)>>1;
		    s+= lx;
		    d+= lx;
		}
#endif
		/* }}} */
	    } else /* !addflag */ {
		/* {{{ */
		zero = vec_splat_u8(0);
		one = vec_splat_u16(1);

		if (w == 8) {
		    /* {{{ */

		    perm = vec_lvsl(0, (unsigned char*)s);

		    if (((unsigned long)d & 0xf) == 0) {
			/* {{{ */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* s[i] + s[i+lx] */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+lx]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+lx]+1) >> 1 */
			    s0H = vec_sra(s0H, one);


			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(s0H, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    } else {
			/* {{{ */
			/* d is offset 8 bytes, work on dL (low bytes).
			 */
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);
			s += lx;

			for (i = 0; i < h; i++) {
			    lS2 = vec_ld(0, (unsigned char*)s);
			    lS3 = vec_ld(16, (unsigned char*)s);
			    lS2 = vec_perm(lS2, lS3, perm);

			    lD = vec_ld(0, (unsigned char*)d);


			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) s+lx[0-7] */
			    s2H = vu16(vec_mergeh(zero, lS2));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* s[i] + s[i+lx] */
			    s0H = vec_add(s0H, s2H);

			    /* (s[i]+s[i+lx]) + 1 */
			    s0H = vec_add(s0H, one);

			    /* (s[i]+s[i+lx]+1) >> 1 */
			    s0H = vec_sra(s0H, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, s0H));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    lS0 = lS2;

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    }
		    /* }}} */
		} else /* w == 16 */ {
		    /* {{{ */

		    perm = vec_lvsl(0, s);

		    lS = vec_ld(0, (unsigned char*)s);
		    lS1 = vec_ld(16, (unsigned char*)s);
		    lS0 = vec_perm(lS, lS1, perm);
		    s += lx;

		    for (i = 0; i < h; i++) {
			lS = vec_ld(0, (unsigned char*)s);
			lS3 = vec_ld(16, (unsigned char*)s);
			lS2 = vec_perm(lS, lS3, perm);


			/* (unsigned short[]) s[0-7] */
			s0H = vu16(vec_mergeh(zero, lS0));

			/* (unsigned short[]) s[8-15] */
			s0L = vu16(vec_mergel(zero, lS0));

			/* (unsigned short[]) s+lx[0-7] */
			s2H = vu16(vec_mergeh(zero, lS2));

			/* (unsigned short[]) s+lx[8-15] */
			s2L = vu16(vec_mergel(zero, lS2));

			/* s[i] + s[i+lx] */
			s0H = vec_add(s0H, s2H);
			s0L = vec_add(s0L, s2L);

			/* (s[i]+s[i+lx]) + 1 */
			s0H = vec_add(s0H, one);
			s0L = vec_add(s0L, one);

			/* (s[i]+s[i+lx]+1) >> 1 */
			s0H = vec_sra(s0H, one);
			s0L = vec_sra(s0L, one);


			/* (unsigned char[]) d[0-7], d[8-15] */
			dH = vu16(vec_packsu(s0H, s0L));
			vec_st(vu8(dH), 0, (unsigned char*)d);

			lS0 = lS2;

			s += lx;
			d += lx;
		    }
		    /* }}} */
		}
#if 0
		for (j=0; j<h; j++)
		{
		    for (i=0; i<w; i++)
			d[i] = (unsigned int)(s[i]+s[i+lx]+1)>>1;
		    s+= lx;
		    d+= lx;
		}
#endif
		/* }}} */
	    }
	    /* }}} */
	} else /* !yh */ {
	    /* (!xh && !yh) {{{ */
	    if (addflag) {
		/* {{{ */
		zero = vec_splat_u8(0);
		one = vec_splat_u16(1);

		if (w == 8) {
		    /* {{{ */
		    perm = vec_lvsl(0, (unsigned char*)s);

		    if (((unsigned long)d & 0xf) == 0) {
			for (i = 0; i < h; i++) {
			    lS0 = vec_ld(0, (unsigned char*)s);
			    lS1 = vec_ld(16, (unsigned char*)s);
			    lS0 = vec_perm(lS0, lS1, perm);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* d[i] + s[i] */
			    dH = vec_add(dH, s0H);

			    /* (d[i]+s[i]) + 1 */
			    dH = vec_add(dH, one);

			    /* (d[i]+s[i]+1) >> 1 */
			    dH = vec_sra(dH, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    } else {
			/* {{{ */
			/* d is offset 8 bytes, work on dL (low bytes).
			 */
			for (i = 0; i < h; i++) {
			    lS0 = vec_ld(0, (unsigned char*)s);
			    lS1 = vec_ld(16, (unsigned char*)s);
			    lS0 = vec_perm(lS0, lS1, perm);

			    lD = vec_ld(0, (unsigned char*)d);

			    /* (unsigned short[]) s[0-7] */
			    s0H = vu16(vec_mergeh(zero, lS0));

			    /* (unsigned short[]) d[0-7] */
			    dH = vu16(vec_mergeh(zero, lD));

			    /* (unsigned short[]) d[8-15] */
			    dL = vu16(vec_mergel(zero, lD));

			    /* d[i] + s[i] */
			    dL = vec_add(dL, s0H);

			    /* (d[i]+s[i]) + 1 */
			    dL = vec_add(dL, one);

			    /* (d[i]+s[i]+1) >> 1 */
			    dL = vec_sra(dL, one);

			    /* (unsigned char[]) d[0-7], d[8-15] */
			    dH = vu16(vec_packsu(dH, dL));
			    vec_st(vu8(dH), 0, (unsigned char*)d);

			    s += lx;
			    d += lx;
			}
			/* }}} */
		    }
		} else /* w == 16 */ {
		    /* {{{ */
		    perm = vec_lvsl(0, s);

		    for (i = 0; i < h; i++) {
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);

			lD = vec_ld(0, (unsigned char*)d);

			/* (unsigned short[]) s[0-7] */
			s0H = vu16(vec_mergeh(zero, lS0));

			/* (unsigned short[]) s[8-15] */
			s0L = vu16(vec_mergel(zero, lS0));

			/* (unsigned short[]) d[0-7] */
			dH = vu16(vec_mergeh(zero, lD));

			/* (unsigned short[]) d[8-15] */
			dL = vu16(vec_mergel(zero, lD));

			/* d[i] + s[i] */
			dH = vec_add(dH, s0H);
			dL = vec_add(dL, s0L);

			/* (d[i]+s[i]) + 1 */
			dH = vec_add(dH, one);
			dL = vec_add(dL, one);

			/* (d[i]+s[i]+1) >> 1 */
			dH = vec_sra(dH, one);
			dL = vec_sra(dL, one);


			/* (unsigned char[]) d[0-7], d[8-15] */
			dH = vu16(vec_packsu(dH, dL));
			vec_st(vu8(dH), 0, (unsigned char*)d);

			s += lx;
			d += lx;
		    }
		    /* }}} */
		}
#if 0
		for (j=0; j<h; j++)
		{
		    for (i=0; i<w; i++)
			d[i] = (unsigned int)(d[i]+s[i]+1)>>1;
		    s+= lx;
		    d+= lx;
		}
#endif
		/* }}} */
	    } else /* !addflag */ {
		/* {{{ */
		if (w == 8) {
		    /* {{{ */
		    /* w / 4, int size copying instead of byte size */
		    for (j = 0; j < h; j++) {
			*((unsigned int*)d)   = *((unsigned int*)s);
			*((unsigned int*)d+1) = *((unsigned int*)s+1);
			s += lx;
			d += lx;
		    }
		    /* }}} */
		} else /* w == 16 */ {
		    /* {{{ */
		    perm = vec_lvsl(0, s);

		    for (i = 0; i < h; i++) {
			lS0 = vec_ld(0, (unsigned char*)s);
			lS1 = vec_ld(16, (unsigned char*)s);
			lS0 = vec_perm(lS0, lS1, perm);

			vec_st(vu8(lS0), 0, (unsigned char*)d);

			s += lx;
			d += lx;
		    }
		    /* }}} */
		}
		/* }}} */
	    }
	    /* }}} */
	}
    }

    AMBER_STOP;
}

#if ALTIVEC_TEST_FUNCTION(pred_comp) /* {{{ */
#  ifdef ALTIVEC_VERIFY

void pred_comp_altivec_verify(PRED_COMP_PDECL)
{
    int i, j;
    uint8_t *s, *d;
    unsigned long checksum1, checksum2;
    /* dst is used as input and output, it must be saved/restored */
    uint8_t dstcpy[16][16];
    uint8_t dc;

    /* save dst */
    d = dst + lx*y + x;
    for (j = 0; j < h; j++) {
	for (i = 0; i < w; i++) {
	    dstcpy[j][i] = d[i];
	}
	d += lx;
    }

    pred_comp_altivec(PRED_COMP_ARGS);
    d = dst + lx*y + x;
    for (checksum1 = j = 0; j < h; j++) {
	for (i = 0; i < w; i++) {
	    checksum1 += d[i];
	}
	d += lx;
    }

    /* restore/swap dst & dstcpy */
    d = dst + lx*y + x;
    for (j = 0; j < h; j++) {
	for (i = 0; i < w; i++) {
	    dc = d[i];
	    d[i] = dstcpy[j][i];
	    dstcpy[j][i] = dc;
	}
	d += lx;
    }

    ALTIVEC_TEST_WITH(pred_comp)(PRED_COMP_ARGS);
    d = dst + lx*y + x;
    for (checksum2 = j = 0; j < h; j++) {
	for (i = 0; i < w; i++) {
	    checksum2 += d[i];
	}
	d += lx;
    }

    if (checksum1 != checksum2) {
	s = src + lx*(y+(dy>>1)) + (x+(dx>>1));
	d = dst + lx*y + x;

	mjpeg_debug("pred_comp(" PRED_COMP_PFMT ") s=0x%X, d=0x%x",
	    PRED_COMP_ARGS, s, d);
	mjpeg_debug("pred_comp: checksums differ %d != %d",
	    checksum1, checksum2);

	/* d = dst + lx*y + x; */
	for (j = 0; j < h; j++) {
	    for (i = 0; i < w; i++) {
		if (dstcpy[j][i] != d[i]) {
		    mjpeg_debug("pred_comp: dst[%d][%d] %d != %d",
			j, i, dstcpy[j][i], d[i]);
		}
	    }
	    d += lx;
	}
    }
}

#  else
ALTIVEC_TEST(pred_comp, void, (PRED_COMP_PDECL),
    PRED_COMP_PFMT, PRED_COMP_ARGS);
#  endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
