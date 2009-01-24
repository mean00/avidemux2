/* build_sub22_mests.c, this file is part of the
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
#include <math.h>
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif

#define USE_SMR_PPC
#ifdef USE_SMR_PPC
extern int sub_mean_reduction_ppc(int len, me_result_set *set, int reduction);
#endif


/*
 * Get SAD for 2*2 subsampled macroblocks:
 *  (0,0) (+2,0) (0,+2) (+2,+2) pixel-space coordinates
 *  (0,0) (+1,0) (0,+1) (+1,+1) 2*2 subsampled coordinates
 *
 *   blk         (blk)
 *   blk(+2,  0) (blk += 1)
 *   blk( 0, +2) (blk += rowstride-1)
 *   blk(+2, +2) (blk += 1)
 *
 * Iterate through all rows 2 at a time, calculating all 4 sads as we go.
 *
 * Hints regarding input:
 *   a) blk may be vector aligned, mostly not aligned
 *   b) ref is about 50% vector aligned and 50% 8 byte aligned
 *   c) rowstride is always a multiple of 16
 *   d) h == 4 or 8
 *
 * NOTES: Since ref is always 8 byte aligned and we are only interested in
 *        the first 8 bytes, the data can always be retreived with one vec_ld.
 *        This "one vec_ld" optimization is also attempted for blk.
 *
 *        The permutation vectors only need to be calculated once since
 *        rowstride is always a multiple of 16.
 */

#define BUILD_SUB22_MESTS_PDECL /* {{{ */                                    \
  me_result_set *sub44set,                                                   \
  me_result_set *sub22set,                                                   \
  int i0,  int j0, int ihigh, int jhigh,                                     \
  int null_ctl_sad,                                                          \
  uint8_t *s22org,  uint8_t *s22blk,                                         \
  int rowstride, int h,                                                      \
  int reduction                                                              \
  /* }}} */

#define BUILD_SUB22_MESTS_ARGS /* {{{ */                                     \
  sub44set, sub22set,                                                        \
  i0,  j0, ihigh, jhigh,                                                     \
  null_ctl_sad,                                                              \
  s22org,  s22blk,                                                           \
  rowstride, h,                                                              \
  reduction                                                                  \
  /* }}} */

/* int build_sub22_mests_altivec(BUILD_SUB22_MESTS_PDECL) {{{ */
#if defined(ALTIVEC_VERIFY) && ALTIVEC_TEST_FUNCTION(build_sub22_mests)
#define VERIFY_BUILD_SUB22_MESTS

static void verify_sads(uint8_t *blk1, uint8_t *blk2, int stride, int h,
			int *sads, int count);

static int _build_sub22_mests_altivec(BUILD_SUB22_MESTS_PDECL, int verify);
int build_sub22_mests_altivec(BUILD_SUB22_MESTS_PDECL)
{
  return _build_sub22_mests_altivec(BUILD_SUB22_MESTS_ARGS, 0 /* no verify */);
}

static int _build_sub22_mests_altivec(BUILD_SUB22_MESTS_PDECL, int verify)
#else
int build_sub22_mests_altivec(BUILD_SUB22_MESTS_PDECL)
#endif /* }}} */
{
    int i, ih;
    int x, y;
    uint8_t *s22orgblk;
    int len;
    me_result_s *sub44mests;
    me_result_s *cres;
    me_result_s mres;
    /* */
    vector unsigned int  zero;
    vector unsigned char lvsl;
    vector unsigned char perm2;
    vector unsigned char align8x2;
    vector unsigned int  sads;
    vector signed char   xy22,
			 xylim;
    vector unsigned char xint,
			 yint;
    vector unsigned int  vthreshold;
    unsigned int         threshold;
    int stride1, stride2, stride1_16, stride2_16;
    union {
	vector unsigned char _align16;
	struct {
	    me_result_s xylim;
	    unsigned int threshold;
	} init;
	me_result_s xy;
	me_result_s mests[4];
    } vio;
#ifdef ALTIVEC_DST
    DataStreamControl dsc;
#endif
#ifndef USE_SMR_PPC
    int min_weight;
#endif

#ifdef ALTIVEC_VERIFY /* {{{ */
  if (((unsigned long)s22blk & 0x7) != 0)
   mjpeg_error_exit1("build_sub22_mests: s22blk %% 8 != 0, (0x%X)", s22blk);

  if (NOT_VECTOR_ALIGNED(rowstride))
    mjpeg_error_exit1("build_sub22_mests: rowstride %% 16 != 0, (%d)",
		rowstride);

  if (h != 4 && h != 8)
    mjpeg_error_exit1("build_sub22_mests: h != [4|8], (%d)", h);

#if 0
  if (NOT_VECTOR_ALIGNED(cres))
    mjpeg_warn("build_sub22_mests: cres %% 16 != 0, (0x%X)",cres);
#endif

#endif /* }}} */

    AMBER_START;

    len = sub44set->len;
    if (len < 1) {	    /* sub44set->len is sometimes zero. we can */
	sub22set->len = 0;  /* save a lot of effort if we stop short.  */
	return 0;
    }

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(1,0,0);
    dsc.block.count = h;
    dsc.block.stride = rowstride;

    vec_dst(s22blk, dsc.control, 0);

    /* increase size to 2 and increment count */
    dsc.control += DATA_STREAM_CONTROL(1,1,0);
#endif

    sub44mests = sub44set->mests;
    cres = sub22set->mests;
    cres--; /* decrement cres so all stores can be done with stwu */


    /* execute instructions that are not dependent on pack_bits */
    zero  = vec_splat_u32(0); /* initialize to zero */
    /* lvsl = 0x(00,01,02,03,04,05,06,07,08,09,0A,0B,0C,0D,0E,0F) {{{ */
    lvsl = vec_lvsl(0, (unsigned char*) 0);
    /* }}} */

    /* 8*8 or 8*4 calculated in 8*2 chunks */
    /* align8x2 = 0x(00 01 02 03 04 05 06 07 10 11 12 13 14 15 16 17) {{{ */
    align8x2 = vec_sld(lvsl, lvsl, 8);
    perm2    = vec_lvsr(0, (unsigned char*)0);
    align8x2 = vec_sld(align8x2, perm2, 8);
    /* }}} */

    mres.weight = 0;	    /* weight must be zero */
    mres.x = ihigh - i0;    /* x <= (ihigh - i0) */
    mres.y = jhigh - j0;    /* y <= (jhigh - j0) */
    vio.init.xylim = mres;
    threshold = 6 * null_ctl_sad / (reduction << 2);
    vio.init.threshold = threshold;
    xy22 = (vector signed char)VCONST(0,0,0,0, 0,0,2,0, 0,0,0,2, 0,0,2,2);
    xint = vu8(vec_splat_u32(0xf));
    xint = vec_add(xint, lvsl);
    yint = vu8(vec_splat_u32(1));
    yint = vec_add(yint, xint);

    perm2 = vec_lvsl(0, s22blk);
    perm2 = vec_splat(perm2, 0);
    perm2 = vec_add(perm2, align8x2);

    stride1 = rowstride;
    stride2 = rowstride + rowstride;
    stride1_16 = stride1 + 16;
    stride2_16 = stride2 + 16;

    ih = (h >> 1) - 1;

    vthreshold = vec_ld(0, (unsigned int*) &vio.init);
    xylim = vs8(vec_splat(vu32(vthreshold), 0));      /* vio.init.xylim */
    vthreshold = vu32(vec_splat(vu32(vthreshold), 1)); /* vio.init.threshold */

    do { /* while (--len) */

	mres = *sub44mests;
	x = mres.x;
	y = mres.y;

	s22orgblk = s22org + ((y+j0)>>1)*rowstride + ((x+i0)>>1);
#ifdef ALTIVEC_DST
	vec_dst(s22orgblk, dsc.control, 1);
#endif
	mres.weight = 0; /* weight must be zero */
	vio.xy = mres;
	sub44mests++;

	/* calculate SADs for 2*2 subsampled macroblocks: {{{ */
	{
	    vector unsigned int  sad20, sad02, sad22;
	    vector unsigned char max, min, dif;
	    vector unsigned char perm1;
	    vector unsigned char align8x2_0, align8x2_2;
	    vector unsigned char ld0, ld1, ld3;
	    vector unsigned char v8x1a, v8x1b;
	    vector unsigned char vblk8x2;
	    vector unsigned char vref8x2;
	    uint8_t *pblk, *pref;

	    sads = zero;
	    sad20 = zero;
	    sad02 = zero;
	    sad22 = zero;

	    pblk = s22orgblk;
	    pref = s22blk;

	    perm1 = vec_lvsl(0, pblk); /* initialize permute vector */

	    if (((unsigned long)pblk & 0xf) < 8) {
		/* {{{ */
		v8x1a = vec_ld(0, pblk);
		/* pblk += rowstride; */
		v8x1b = vec_ld(stride1, pblk);

		vref8x2 = vec_ld(0, pref);
		/* pref += rowstride; */
		ld3 = vec_ld(stride1, pref);

		align8x2_0 = vec_splat(perm1, 0);
		align8x2_0 = vec_add(align8x2_0, align8x2);
		align8x2_2 = vec_splat(perm1, 1);
		align8x2_2 = vec_add(align8x2_2, align8x2);

		vref8x2 = vec_perm(vref8x2, ld3, perm2);

		i = ih;
		do { /* while (--i) */
		    /* load next row */
		    /* pblk += rowstride; */
		    pblk += stride2;
		    ld0 = vec_ld(0, pblk);

		    /* calculate (0,0) */
		    vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_0);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);
		    dif = vec_sub(max, min);
		    sads = vec_sum4s(dif, sads);

		    /* calculate (2,0) */
		    vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_2);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);
		    dif = vec_sub(max, min);
		    sad20 = vec_sum4s(dif, sad20);

		    /* load into v8x1a, then v8x1b will be the top row */
		    v8x1a = vec_sld(ld0, ld0, 0); /* v8x1a = ld0; */
		    /* load next row */
		    /* pblk += rowstride; */
		    ld0 = vec_ld(stride1, pblk);

		    /* calculate (0,2) */
		    vblk8x2 = vec_perm(v8x1b, v8x1a, align8x2_0);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);
		    dif = vec_sub(max, min);
		    sad02 = vec_sum4s(dif, sad02);

		    /* calculate (2,2) */
		    vblk8x2 = vec_perm(v8x1b, v8x1a, align8x2_2);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);

		    /* pref += rowstride; */
		    pref += stride2;
		    vref8x2 = vec_ld(0, pref);
		    /* pref += rowstride; */
		    ld3 = vec_ld(stride1, pref);

		    dif = vec_sub(max, min);
		    sad22 = vec_sum4s(dif, sad22);

		    v8x1b = vec_sld(ld0, ld0, 0); /* v8x1b = ld0; */

		    vref8x2 = vec_perm(vref8x2, ld3, perm2);
		} while (--i);

		/* load next row */
		/* pblk += rowstride; */
		pblk += stride2;
		ld0 = vec_ld(0, pblk);

		/* calculate (0,0) */
		vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_0);
		max = vec_max(vblk8x2, vref8x2);
		min = vec_min(vblk8x2, vref8x2);
		dif = vec_sub(max, min);
		sads = vec_sum4s(dif, sads);

		/* calculate (2,0) */
		vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_2);
		/* load into v8x1a, then v8x1b will be the top row */
		v8x1a = vec_sld(ld0, ld0, 0); /* v8x1a = ld0; */
		/* }}} */
	    } else {
		/* {{{ */
		v8x1a = vec_ld(0, pblk);
		ld0 = vec_ld(16, pblk);

		/* pblk += rowstride; */
		v8x1b = vec_ld(stride1, pblk);
		ld1 = vec_ld(stride1_16, pblk);

		/* align8x2_0 = align8x2 */
		align8x2_0 = vec_sld(align8x2, align8x2, 0);
		align8x2_2 = vec_splat_u8(1); 
		align8x2_2 = vec_add(align8x2, align8x2_2 /* (1) */ );

		vref8x2 = vec_ld(0, pref);
		/* pref += rowstride; */
		ld3 = vec_ld(stride1, pref);

		v8x1a = vec_perm(v8x1a, ld0, perm1);
		v8x1b = vec_perm(v8x1b, ld1, perm1);

		vref8x2 = vec_perm(vref8x2, ld3, perm2);

		i = ih;
		do { /* while (--i) */
		    /* load next row */
		    /* pblk += rowstride; */
		    pblk += stride2;
		    ld0 = vec_ld(0, pblk);
		    ld1 = vec_ld(16, pblk);

		    /* calculate (0,0) */
		    vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_0);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);
		    dif = vec_sub(max, min);
		    sads = vec_sum4s(dif, sads);

		    /* calculate (2,0) */
		    vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_2);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);
		    dif = vec_sub(max, min);
		    sad20 = vec_sum4s(dif, sad20);

		    /* load into v8x1a, then v8x1b will be the top row */
		    v8x1a = vec_perm(ld0, ld1, perm1);
		    /* load next row */
		    /* pblk += rowstride; */
		    ld0 = vec_ld(stride1, pblk);
		    ld1 = vec_ld(stride1_16, pblk);

		    /* calculate (0,2) */
		    vblk8x2 = vec_perm(v8x1b, v8x1a, align8x2_0);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);
		    dif = vec_sub(max, min);
		    sad02 = vec_sum4s(dif, sad02);

		    /* calculate (2,2) */
		    vblk8x2 = vec_perm(v8x1b, v8x1a, align8x2_2);
		    max = vec_max(vblk8x2, vref8x2);
		    min = vec_min(vblk8x2, vref8x2);

		    /* pref += rowstride; */
		    pref += stride2;
		    vref8x2 = vec_ld(0, pref);
		    /* pref += rowstride; */
		    ld3 = vec_ld(stride1, pref);

		    dif = vec_sub(max, min);
		    sad22 = vec_sum4s(dif, sad22);

		    v8x1b = vec_perm(ld0, ld1, perm1);

		    vref8x2 = vec_perm(vref8x2, ld3, perm2);
		} while (--i);

		/* load next row */
		/* pblk += rowstride; */
		pblk += stride2;
		ld0 = vec_ld(0, pblk);
		ld1 = vec_ld(16, pblk);

		/* calculate (0,0) */
		vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_0);
		max = vec_max(vblk8x2, vref8x2);
		min = vec_min(vblk8x2, vref8x2);
		dif = vec_sub(max, min);
		sads = vec_sum4s(dif, sads);

		/* calculate (2,0) */
		vblk8x2 = vec_perm(v8x1a, v8x1b, align8x2_2);
		/* load into v8x1a, then v8x1b will be the top row */
		v8x1a = vec_perm(ld0, ld1, perm1);
		/* }}} */
	    }

	    /* calculate (2,0) */
	    max = vec_max(vblk8x2, vref8x2);
	    min = vec_min(vblk8x2, vref8x2);
	    dif = vec_sub(max, min);
	    sad20 = vec_sum4s(dif, sad20);

	    /* calculate (0,2) */
	    vblk8x2 = vec_perm(v8x1b, v8x1a, align8x2_0);
	    max = vec_max(vblk8x2, vref8x2);
	    min = vec_min(vblk8x2, vref8x2);
	    dif = vec_sub(max, min);
	    sad02 = vec_sum4s(dif, sad02);

	    /* calculate (2,2) */
	    vblk8x2 = vec_perm(v8x1b, v8x1a, align8x2_2);
	    max = vec_max(vblk8x2, vref8x2);
	    min = vec_min(vblk8x2, vref8x2);
	    dif = vec_sub(max, min);
	    sad22 = vec_sum4s(dif, sad22);

	    /* calculate final sums {{{ */
	    sads = vu32(vec_sums(vs32(sads), vs32(zero)));
	    sad20 = vu32(vec_sums(vs32(sad20), vs32(zero)));
	    sad02 = vu32(vec_sums(vs32(sad02), vs32(zero)));
	    sad22 = vu32(vec_sums(vs32(sad22), vs32(zero)));
	    /* }}} */

	    /* sads = {sads, sad20, sad02, sad22} {{{ */
	    sads = vu32(vec_mergel(vu32(sads), vu32(sad02)));
	    sad20 = vu32(vec_mergel(vu32(sad20), vu32(sad22)));
	    sads = vu32(vec_mergel(vu32(sads), vu32(sad20)));
	    /* }}} */
	} /* }}} */

#ifdef VERIFY_BUILD_SUB22_MESTS /* {{{ */
	if (verify)
	    verify_sads(s22orgblk, s22blk, rowstride, h, (int*)&sads, 4);
#endif /* }}} */

	/* add penalty, clip xy, arrange into me_result_s ... {{{ */
	{
	    vector signed char xy;

	    xy = vec_ld(0, (signed char*) &vio.xy);
	    xy = vs8(vec_splat(vu32(xy), 0)); /* splat vio.xy */
	    xy = vs8(vec_add(xy, xy22)); /* adjust xy values for elements 1-3 */

	    /* add distance penalty {{{ */
	    /* penalty = (max(abs(x),abs(y))<<3) */
	    {
		vector signed char  xyabs;
		vector unsigned int xxxx, yyyy;
		vector unsigned int xymax, penalty;

		/* (abs(x),abs(y)) */
		xyabs = vec_subs(vs8(zero), xy);
		xyabs = vec_max(xyabs, xy);

		/* xxxx = (x, x, x, x), yyyy = (y, y, y, y)
		 * (0,0,x,y, 0,0,x,y, 0,0,x,y, 0,0,x,y) |/- permute vector  -\|
		 * (0,0,0,x, 0,0,0,x, 0,0,0,x, 0,0,0,x) |lvsl+(0x0000000F,...)| 
		 * (0,0,0,y, 0,0,0,y, 0,0,0,y, 0,0,0,y) |lvsl+(0x00000010,...)|
		 */
		xxxx = vu32(vec_perm(vs8(zero), xyabs, xint));
		yyyy = vu32(vec_perm(vs8(zero), xyabs, yint));

		/* penalty = max(abs(x),abs(y)) << 3  */
		xymax = vec_max(xxxx, yyyy);
		penalty = vec_splat_u32(3);
		penalty = vec_sl(xymax, penalty /* (3,...) */ );

		sads = vec_add(sads, penalty);
	    } /* }}} */

	    /* mask sads  x <= (ihigh - i0) && y <= (jhigh - j0) {{{ */
	    /* the first cmpgt (s8) will flag any x and/or y coordinates... {{{
	     * as out of bounds. the second cmpgt (u32) will complete the
	     * mask if the x or y flag for that result is set.
	     *
	     * Example: {{{ 
	     *        X  Y         X  Y         X  Y         X  Y
	     * [0  0  <  <] [0  0  <  <] [0  0  >  <] [0  0  <  >]
	     * vb8(xymask)  = vec_cmpgt(vu8(xy), xylim)
	     * [0  0  0  0] [0  0  0  0] [0  0  1  0] [0  0  0  1]
	     * vb32(xymask) = vec_cmpgt(vu32(xymask), vu32(zero))
	     * [0  0  0  0] [0  0  0  0] [1  1  1  1] [1  1  1  1]
	     *
	     * Legend: 0=0x00  (<)=(xy[n] <= xymax[n])
	     *         1=0xff  (>)=(xy[n] >  xymax[n])
	     * }}}
	     */ /* }}} */
	    {
		vector bool int xymask;

		xymask = vb32(vec_cmpgt(xy, xylim));
		xymask = vec_cmpgt(vu32(xymask), zero);

		/* add (saturated) xymask to sads thereby forcing
		 * masked values above the threshold.
		 */
		sads = vec_adds(sads, vu32(xymask));
	    } /* }}} */

	    /* arrange sad and xy into me_result_s form and store {{{ */
	    {
		vector unsigned int mests;
		/* mests = ( sad,  xy, sad,  xy, sad,  xy, sad,  xy ) {{{
		 *
		 * (   0, sad,   0, sad,   0, sad,   0, sad )
		 * ( sad, sad, sad, sad, sad, sad, sad, sad )
		 *
		 * (   0,  xy,   0,  xy,   0,  xy,   0,  xy )
		 * (  xy,  xy,  xy,  xy,  xy,  xy,  xy,  xy )
		 *
		 * ( sad,  xy, sad,  xy, sad,  xy, sad,  xy )
		 */ /* }}} */
		xy = vs8(vec_pack(vu32(xy), vu32(xy)));
		mests = vu32(vec_pack(vu32(sads), vu32(sads)));
		mests = vu32(vec_mergeh(vu16(mests), vu16(xy)));

		vec_st(mests, 0, (unsigned int*)&vio.mests);
	    } /* }}} */
	} /* }}} */

	if (vec_any_lt(sads, vthreshold)) {
	    me_result_s m0, m1, m2, m3;
	    unsigned int w0, w1, w2, w3;

	    m0 = vio.mests[0];
	    m1 = vio.mests[1];
	    m2 = vio.mests[2];
	    m3 = vio.mests[3];

	    w0 = m0.weight;
	    w1 = m1.weight;
	    w2 = m2.weight;
	    w3 = m3.weight;

	    if (w0 < threshold)
		*(++cres) = m0;
	    if (w1 < threshold)
		*(++cres) = m1;
	    if (w2 < threshold)
		*(++cres) = m2;
	    if (w3 < threshold)
		*(++cres) = m3;
	}
    } while (--len);

    cres++; /* increment to account for earlier decrement */
    len = cres - sub22set->mests;
    sub22set->len = len;

    AMBER_STOP;

#ifdef USE_SMR_PPC
    if ((len | reduction) > 0)
	len = sub_mean_reduction_ppc(len, sub22set, reduction);
    return len;
#else
#if ALTIVEC_TEST_FUNCTION(sub_mean_reduction)
    ALTIVEC_TEST_SUFFIX(sub_mean_reduction)(sub22set, reduction, &min_weight);
#else
    ALTIVEC_SUFFIX(sub_mean_reduction)(sub22set, reduction, &min_weight);
#endif
    return sub22set->len;
#endif
}

#if ALTIVEC_TEST_FUNCTION(build_sub22_mests) /* {{{ */

#define BUILD_SUB22_MESTS_PFMT                                               \
  "sub44set=0x%X, sub22set=0x%X, i0=%d, j0=%d, ihigh=%d, jhigh=%d, "         \
  "null_ctl_sad=%d, s22org=0x%X, s22blk=0x%X, rowstride=%d, h=%d, "          \
  "reduction=%d"

#  ifdef ALTIVEC_VERIFY
int build_sub22_mests_altivec_verify(BUILD_SUB22_MESTS_PDECL)
{
  int i, len1, len2;
  unsigned long checksum1, checksum2;

  len1 = _build_sub22_mests_altivec(BUILD_SUB22_MESTS_ARGS, 1 /*verify*/);
  for (checksum1 = i = 0; i < len1; i++) {
    checksum1 += sub22set->mests[i].weight;
    checksum1 += abs(sub22set->mests[i].x);
    checksum1 += abs(sub22set->mests[i].y);
  }

  len2 = ALTIVEC_TEST_WITH(build_sub22_mests)(BUILD_SUB22_MESTS_ARGS);
  for (checksum2 = i = 0; i < len2; i++) {
    checksum2 += sub22set->mests[i].weight;
    checksum2 += abs(sub22set->mests[i].x);
    checksum2 += abs(sub22set->mests[i].y);
  }

  if (len1 != len2 || checksum1 != checksum2) {
    mjpeg_debug("build_sub22_mests(" BUILD_SUB22_MESTS_PFMT ")",
	BUILD_SUB22_MESTS_ARGS);
    mjpeg_debug("build_sub22_mests: sub44set->len=%d", sub44set->len);
    mjpeg_debug("build_sub22_mests: checksums differ %d[%d] != %d[%d]",
	checksum1, len1, checksum2, len2);
  }
#if 0
 else {
    mjpeg_info("build_sub22_mests(" BUILD_SUB22_MESTS_PFMT ")",
	BUILD_SUB22_MESTS_ARGS);
    mjpeg_info("build_sub22_mests: sub44set->len=%d", sub44set->len);
    mjpeg_info("build_sub22_mests: checksum %d[%d]",
	checksum1, len1);
  }
#endif

  return len2;
}

static void verify_sads(uint8_t *blk1, uint8_t *blk2, int stride, int h,
			int *sads, int count)
{
    int i, s, s2;
    uint8_t *pblk;

    pblk = blk1;
    for (i = 0; i < count; i++) {
	s2 = sads[i];
	/* s = sad_sub22(pblk, blk2, stride, h); {{{ */
#if ALTIVEC_TEST_FUNCTION(sad_sub22)
	s = ALTIVEC_TEST_WITH(sad_sub22)(pblk, blk2, stride, h);
#else
	s = sad_sub22(pblk, blk2, stride, h);
#endif /* }}} */
	if (s2 != s) {
	    mjpeg_debug("build_sub22_mests: sads[%d]=%d != %d"
			"=sad_sub22(blk1=0x%X(0x%X), blk2=0x%X, "
			"stride=%d, h=%d)",
			i, s2, s, pblk, blk1, blk2, stride, h);
	}

	if (i == 1)
	    pblk += stride - 1;
	else
	    pblk += 1;
    }
}

#  else

#undef BENCHMARK_EPILOG
#define BENCHMARK_EPILOG \
    mjpeg_info("build_sub22_mests: sub44set->len=%d", sub44set->len); \
    mjpeg_info("build_sub22_mests: sub22set->len=%d", sub22set->len);

ALTIVEC_TEST(build_sub22_mests, int, (BUILD_SUB22_MESTS_PDECL),
  BUILD_SUB22_MESTS_PFMT, BUILD_SUB22_MESTS_ARGS);
#  endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
