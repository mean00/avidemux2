/* variance.c, this file is part of the
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


#undef HANDLE_ALL


#define VARIANCE_PDECL uint8_t *p, int size, int rowstride, \
                       unsigned int *p_var, unsigned int *p_mean
#define VARIANCE_ARGS p, size, rowstride, p_var, p_mean
#define VARIANCE_PFMT "p=0x%X, size=%d, rowstride=%d, p_var=0x%X, p_mean=0x%X"

/*
 * variance of a (size*size) block, multiplied by 256
 * p:  address of top left pel of block
 * rowstride: distance (in bytes) of vertically adjacent pels
 * SIZE is a multiple of 8.
 */
void variance_altivec(VARIANCE_PDECL)
{
    unsigned int s, s2, sz;
#ifdef ALTIVEC_DST
    unsigned int dst;
#endif
    vector unsigned char zero;
    vector unsigned char lA, lB, vA, vB;
    vector signed short sum;
    vector signed int msum;
    union {
	vector signed int v;
	struct {
	    signed int pad[2];
	    signed int sum;
	    signed int msum;
	} s;
    } vo;

#ifdef ALTIVEC_VERIFY /* {{{ */
    if ((((unsigned long)p) & 0x7) != 0)
	mjpeg_error_exit1("variance: p %% 8 != 0, (0x%X)", p);

    if ((size & 0x7) != 0)
	mjpeg_error_exit1("variance: size %% 8 != 0, (%d)", size);

    if (size == 16 && NOT_VECTOR_ALIGNED(p))
	mjpeg_error_exit1("variance: size == 16 && p %% 16 != 0 (0x%X)", p);

    if (NOT_VECTOR_ALIGNED(rowstride))
	mjpeg_error_exit1("variance: rowstride %% 16 != 0, (%d)", rowstride);

    if (rowstride & (~0xffff) != 0)
	mjpeg_error_exit1("variance: rowstride > vec_dst range", rowstride);
#endif /* }}} */

    AMBER_START;

#ifdef HANDLE_ALL
    if (size <= 16) {
#endif
	zero = vec_splat_u8(0);
	sum = vec_splat_s16(0);
	msum = vec_splat_s32(0);

	if (size == 8) { 
#ifdef ALTIVEC_DST
	    dst = 0x01080000 | rowstride;
	    vec_dst(p, dst, 0);
#endif
	    sz = 6;
#define PREPARE_FIRST_ITERATION(ab,hl) /* {{{ */                             \
		l##ab = vec_ld(0, (unsigned char*)p);                        \
		v##ab = vec_merge##hl(zero, l##ab);                          \
		/* }}} */

#define	PERFORM_ITERATION(ab,iteration) /* {{{ */                            \
		sum = vec_add(vs16(v##ab), sum);                             \
		msum = vec_msum(vs16(v##ab), vs16(v##ab), msum);             \
		/* }}} */

#define PREPARE_ITERATION(ab,hl) /* {{{ */                                   \
		p += rowstride;                                              \
		l##ab = vec_ld(0, (unsigned char*)p);                        \
		v##ab = vec_merge##hl(zero, l##ab);                          \
		/* }}} */

	    if (((unsigned long)p & 0xf) == 0) {
		PREPARE_FIRST_ITERATION(A,h); PERFORM_ITERATION(A,0);
		PREPARE_ITERATION(B,h);       PERFORM_ITERATION(B,1);
		PREPARE_ITERATION(A,h);       PERFORM_ITERATION(A,2);
		PREPARE_ITERATION(B,h);       PERFORM_ITERATION(B,3);
		PREPARE_ITERATION(A,h);       PERFORM_ITERATION(A,4);
		PREPARE_ITERATION(B,h);       PERFORM_ITERATION(B,5);
		PREPARE_ITERATION(A,h);       PERFORM_ITERATION(A,6);
		PREPARE_ITERATION(B,h);       PERFORM_ITERATION(B,7);
	    } else {
		PREPARE_FIRST_ITERATION(A,l); PERFORM_ITERATION(A,0);
		PREPARE_ITERATION(B,l);       PERFORM_ITERATION(B,1);
		PREPARE_ITERATION(A,l);       PERFORM_ITERATION(A,2);
		PREPARE_ITERATION(B,l);       PERFORM_ITERATION(B,3);
		PREPARE_ITERATION(A,l);       PERFORM_ITERATION(A,4);
		PREPARE_ITERATION(B,l);       PERFORM_ITERATION(B,5);
		PREPARE_ITERATION(A,l);       PERFORM_ITERATION(A,6);
		PREPARE_ITERATION(B,l);       PERFORM_ITERATION(B,7);
	    }

#undef PREPARE_FIRST_ITERATION
#undef PERFORM_ITERATION
#undef PREPARE_ITERATION

	} else /* size == 16 */ {
#ifdef ALTIVEC_DST
	    dst = 0x01100000 | rowstride;
	    vec_dst(p, dst, 0);
#endif
	    sz = 8;
#define PREPARE_FIRST_ITERATION(ab) /* {{{ */                                \
	    l##ab = vec_ld(0, (unsigned char*)p);                            \
	    vA = vec_mergeh(zero, l##ab);                                    \
	    vB = vec_mergel(zero, l##ab);                                    \
	    /* }}} */

#define	PERFORM_ITERATION(ab,iteration) /* {{{ */                            \
	    sum = vec_add(vs16(vA), sum);                                    \
	    sum = vec_add(vs16(vB), sum);                                    \
	    msum = vec_msum(vs16(vA), vs16(vA), msum);                       \
	    msum = vec_msum(vs16(vB), vs16(vB), msum);                       \
	    /* }}} */

#define PREPARE_ITERATION(ab) /* {{{ */                                      \
	    p += rowstride;                                                  \
	    l##ab = vec_ld(0, (unsigned char*)p);                            \
	    vA = vec_mergeh(zero, l##ab);                                    \
	    vB = vec_mergel(zero, l##ab);                                    \
	    /* }}} */

	    PREPARE_FIRST_ITERATION(A); PERFORM_ITERATION(A,0);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,1);
	    PREPARE_ITERATION(A);       PERFORM_ITERATION(A,2);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,3);
	    PREPARE_ITERATION(A);       PERFORM_ITERATION(A,4);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,5);
	    PREPARE_ITERATION(A);       PERFORM_ITERATION(A,6);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,7);
	    PREPARE_ITERATION(A);       PERFORM_ITERATION(A,8);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,9);
	    PREPARE_ITERATION(A);       PERFORM_ITERATION(A,10);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,11);
	    PREPARE_ITERATION(A);       PERFORM_ITERATION(A,12);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,13);
	    PREPARE_ITERATION(A);       PERFORM_ITERATION(A,14);
	    PREPARE_ITERATION(B);       PERFORM_ITERATION(B,15);

#undef PREPARE_FIRST_ITERATION
#undef PERFORM_ITERATION
#undef PREPARE_ITERATION
	}

#ifdef HANDLE_ALL
    } else {
	/* I don't think this branch of code will be ever be used, but
	 * just in case.
	 */
	mjpeg_info("variance(size > 16): altivec me");
	return variance(VARIANCE_ARGS);
    }
#endif

    sum = vs16(vec_sum4s(sum, vs32(zero)));
    sum = vs16(vec_sums(vs32(sum), vs32(zero)));
    msum = vec_sums(msum, vs32(zero));

#ifdef ALTIVEC_DST
    vec_dss(0);
#endif

    vo.v = vec_mergel(vs32(sum), vs32(msum));

    s = vo.s.sum;
    s2 = vo.s.msum;

    /* var = s2 - ((s * s) >> sz); */ /* ((s*s)/(size*size)) size=8|16 */

    *p_var = s2 - ((s * s) >> sz); /* ((s*s)/(size*size)) size=8|16 */
    *p_mean = s >> sz;             /* (s/(size*size))     size=8|16 */

    AMBER_STOP;
}

#if ALTIVEC_TEST_FUNCTION(variance)
#  ifdef ALTIVEC_VERIFY
void variance_altivec_verify(VARIANCE_PDECL)
{
  unsigned int c_var, c_mean;
  unsigned int v_var, v_mean;

  ALTIVEC_TEST_WITH(variance)(p, size, rowstride, &c_var, &c_mean);
  variance_altivec(p, size, rowstride, &v_var, &v_mean);

  if (v_var != c_var)
    mjpeg_debug("*p_var: %d != %d variance(" VARIANCE_PFMT ")",
	v_var, c_var, VARIANCE_ARGS);

  if (v_mean != c_mean)
    mjpeg_debug("*p_mean: %d != %d variance(" VARIANCE_PFMT ")",
	v_mean, c_mean, VARIANCE_ARGS);

  *p_var = c_var;
  *p_mean = c_mean;
}
#  else
ALTIVEC_TEST(variance, void, (VARIANCE_PDECL), VARIANCE_PFMT, VARIANCE_ARGS);
#  endif
#endif
/* vim:set foldmethod=marker foldlevel=0: */
