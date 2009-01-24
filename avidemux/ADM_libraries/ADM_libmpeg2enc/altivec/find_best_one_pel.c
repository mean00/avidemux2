/* find_best_one_pel.c, this file is part of the
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

#include <limits.h>

#include "altivec_motion.h"
#include "vectorize.h"
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
/* #define AMBER_MAX_TRACES 10 */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


/*
 * Search for the best 1-pel match within 1-pel of a good 2*2-pel
 *
 * Input requirements:
 *   a) ref is always vector aligned
 *   b) rowstride is a multiple of 16
 *   c) h is either 8 or 16
 *
 */
#define FIND_BEST_ONE_PEL_PDECL /* {{{ */                                    \
  me_result_set *sub22set,                                                   \
  uint8_t *org, uint8_t *ref,                                                \
  int i0, int j0,                                                            \
  int ihigh, int jhigh,                                                      \
  int rowstride, int h,                                                      \
  me_result_s *best_so_far                                                   \
  /* }}} */

#define FIND_BEST_ONE_PEL_ARGS /* {{{ */                                     \
  sub22set, org, ref,                                                        \
  i0, j0, ihigh, jhigh,                                                      \
  rowstride, h, best_so_far                                                  \
  /* }}} */

/* void find_best_one_pel_altivec(FIND_BEST_ONE_PEL_PDECL) {{{ */
#if defined(ALTIVEC_VERIFY) && ALTIVEC_TEST_FUNCTION(find_best_one_pel)
#define VERIFY_FIND_BEST_ONE_PEL
static void verify_sads(uint8_t *blk1, uint8_t *blk2, int stride,
			int h, signed int *sads, int count);

static void _find_best_one_pel_altivec(FIND_BEST_ONE_PEL_PDECL, int verify);
void find_best_one_pel_altivec(FIND_BEST_ONE_PEL_PDECL)
{
  _find_best_one_pel_altivec(FIND_BEST_ONE_PEL_ARGS, 0 /* no verify */);
}

static void _find_best_one_pel_altivec(FIND_BEST_ONE_PEL_PDECL, int verify)
#else
void find_best_one_pel_altivec(FIND_BEST_ONE_PEL_PDECL)
#endif
/* }}} */
{
    int i;
    uint8_t *orgblk;
    me_result_s *sub22mests;
    int len;
    uint8_t *pblk, *pref;
    int x, y;
    me_result_s mres;
    vector unsigned char t0, t1, t2;
    vector unsigned char l0, l1;
    vector unsigned char perm0, perm1;
    vector unsigned char blk1_0, blk1_1;
    vector unsigned char vref;
    vector unsigned int zero;
    vector unsigned int sad00, sad10, sad01, sad11;
    vector unsigned int sads;
    vector unsigned int minsad;
    vector bool int minsel;
    vector signed char xy;
    vector signed char xylim;
    vector signed char minxy;
    vector signed char xy11;
    vector unsigned char xint,
			 yint;
    union {
	vector unsigned int _align16;
	struct {
	    me_result_s xylim;
	} init;
	me_result_s xy;
	me_result_s best;
    } vio;
#ifdef ALTIVEC_DST
    DataStreamControl dsc;
#endif
#ifdef VERIFY_FIND_BEST_ONE_PEL
    vector signed int versads;
#endif

#ifdef ALTIVEC_VERIFY /* {{{ */
  if (NOT_VECTOR_ALIGNED(org))
    mjpeg_error_exit1("find_best_one_pel: org %% 16 != 0, (0x%X)", org);

  if (NOT_VECTOR_ALIGNED(ref))
    mjpeg_error_exit1("find_best_one_pel: ref %% 16 != 0, (0x%X)", ref);

  if (NOT_VECTOR_ALIGNED(rowstride))
    mjpeg_error_exit1("find_best_one_pel: rowstride %% 16 != 0, (%d)",
      rowstride);

  if (h != 8 && h != 16)
    mjpeg_error_exit1("find_best_one_pel: h != [8|16], (%d)", h);
#endif /* }}} */

    AMBER_START;

    len = sub22set->len;
    if (len < 1) {			/* sub22set->len is sometimes zero.  */
	best_so_far->weight = 255*255;	/* we can save a lot of effort if we */
	return;				/* stop short.                       */
    }

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(1,0,0);
    dsc.block.count = h;
    dsc.block.stride = rowstride;
    vec_dst(ref, dsc.control, 0);

    /* increase size to 2 and increment count */
    dsc.control += DATA_STREAM_CONTROL(1,1,0);
#endif

    xy11 = (vector signed char)VCONST(0,0,0,0, 0,0,1,0, 0,0,0,1, 0,0,1,1);

    mres.weight = 0;		/* weight must be zero */
    mres.x = ihigh - i0;	/* x <= xylim.x */
    mres.y = jhigh - j0;	/* y <= xylim.y */
    vio.init.xylim = mres;

    yint = vec_lvsl(0, (unsigned char*)0);
    xint = vu8(vec_splat_u32(0xf));
    xint = vec_add(xint, yint /* lvsl */ );
    yint = vu8(vec_splat_u32(1));
    yint = vec_add(yint, xint);

    /* initialize to zero */
    zero = vec_splat_u32(0);

    xylim = vec_ld(0, (signed char*) &vio.init.xylim);
    xylim = vs8(vec_splat(vu32(xylim), 0));

    minsad = vu32(vec_splat_s8(-1));
    
    sub22mests = sub22set->mests;

    do {
	mres = *sub22mests;
	x = mres.x;
	y = mres.y;

	orgblk = org + (i0 + x) + rowstride*(j0 + y);
#ifdef ALTIVEC_DST
	vec_dst(orgblk, dsc.control, 1);
#endif

	mres.weight = 0; /* weight must be zero */
	vio.xy = mres;
	sub22mests++;
    

#ifdef ALTIVEC_VERIFY
	/* orgblk alignment should always be a multiple of 2 {0,2,4,6,8,A,C,E}
	 * this is important to avoid the edge case where (orgblk&15)==15
	 */
	if (((unsigned int)orgblk & 1) != 0)
	    mjpeg_warn("find_best_one_pel: orgblk %% 2 != 0 (0x%X)", orgblk);
#endif

    
	/* calculate SAD for macroblocks:
	 * orgblk(0, 0), orgblk(+1, 0),
	 * orgblk(0,+1), orgblk(+1,+1)
	 */

	/* initialize to sad vectors to zero {{{ */
	sad00 = vec_splat_u32(0);
	sad10 = vec_splat_u32(0);
	sad01 = vec_splat_u32(0);
	sad11 = vec_splat_u32(0);
	/* }}} */

	pblk = orgblk; /* always aligned by 2 {0,2,4,6,8,A,C,E} */
	l0 = vec_ld(0, pblk);                      
	l1 = vec_ld(16, pblk);

	pref = ref;
	vref = vec_ld(0, pref);

	perm0 = vec_lvsl(0, pblk);
	perm1 = vec_splat_u8(1);
	perm1 = vec_add(perm0, perm1);

	blk1_0 = vec_perm(l0, l1, perm0);    
	blk1_1 = vec_perm(l0, l1, perm1);    

	i = h - 1;
	do {
	    /* start loading next */
	    pblk += rowstride;
	    l0 = vec_ld(0, pblk);                      
	    l1 = vec_ld(16, pblk);

	    t0 = vec_max(blk1_0, vref); 
	    t1 = vec_min(blk1_0, vref); 
	    t2 = vec_sub(t0, t1);          
	    sad00  = vec_sum4s(t2, sad00);         

	    t0 = vec_max(blk1_1, vref); 
	    t1 = vec_min(blk1_1, vref); 
	    t2 = vec_sub(t0, t1);          
	    sad10  = vec_sum4s(t2, sad10);         


	    blk1_0 = vec_perm(l0, l1, perm0);    
	    blk1_1 = vec_perm(l0, l1, perm1);    


	    t0 = vec_max(blk1_0, vref); 
	    t1 = vec_min(blk1_0, vref); 
	    t2 = vec_sub(t0, t1);          
	    sad01  = vec_sum4s(t2, sad01);         

	    t0 = vec_max(blk1_1, vref); 
	    t1 = vec_min(blk1_1, vref); 
    
	    pref += rowstride;
	    vref = vec_ld(0, pref);

	    t2 = vec_sub(t0, t1);          
	    sad11  = vec_sum4s(t2, sad11);         
	} while (--i);

	/* start loading last */
	pblk += rowstride;
	l0 = vec_ld(0, pblk);                      
	l1 = vec_ld(16, pblk);

	t0 = vec_max(blk1_0, vref); 
	t1 = vec_min(blk1_0, vref); 
	t2 = vec_sub(t0, t1);          
	sad00  = vec_sum4s(t2, sad00);         

	t0 = vec_max(blk1_1, vref); 
	t1 = vec_min(blk1_1, vref); 
	t2 = vec_sub(t0, t1);          
	sad10  = vec_sum4s(t2, sad10);         

	blk1_0 = vec_perm(l0, l1, perm0);    
	blk1_1 = vec_perm(l0, l1, perm1);    

	t0 = vec_max(blk1_0, vref); 
	t1 = vec_min(blk1_0, vref); 
	t2 = vec_sub(t0, t1);          
	sad01  = vec_sum4s(t2, sad01);         

	t0 = vec_max(blk1_1, vref); 
	t1 = vec_min(blk1_1, vref); 
	t2 = vec_sub(t0, t1);          
	sad11  = vec_sum4s(t2, sad11);         


	/* calculate final sums {{{ */
	sad00 = vu32(vec_sums(vs32(sad00), vs32(zero)));
	sad10 = vu32(vec_sums(vs32(sad10), vs32(zero)));
	sad01 = vu32(vec_sums(vs32(sad01), vs32(zero)));
	sad11 = vu32(vec_sums(vs32(sad11), vs32(zero)));
	/* }}} */

	/* sads = {sad00, sad10, sad01, sad11} {{{ */
	sad00 = vu32(vec_mergel(vu32(sad00), vu32(sad01)));
	sad10 = vu32(vec_mergel(vu32(sad10), vu32(sad11)));
	sads = vu32(vec_mergel(vu32(sad00), vu32(sad10)));
	/* }}} */

#ifdef VERIFY_FIND_BEST_ONE_PEL /* {{{ */
	if (verify) {
	    vec_st(sads, 0, (unsigned int*)&versads);
	    verify_sads(orgblk, ref, rowstride, h, (signed int*)&versads, 4);
	}
#endif /* }}} */

	/* add penalty, clip xy, arrange into me_result_s ... {{{ */
	{
	    xy = vec_ld(0, (signed char*) &vio.xy);
	    xy = vs8(vec_splat(vu32(xy), 0)); /* splat vio.xy */

	    /* add distance penalty {{{ */
	    /* penalty = (abs(x) + abs(y)) << 3 */
	    {
		vector signed char  xyabs;
		vector unsigned int xxxx, yyyy;
		vector unsigned int penalty;

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

		/* penalty = (abs(x) + abs(y)) << 3 */
		xxxx = vec_add(xxxx, yyyy);
		penalty = vec_splat_u32(3);
		penalty = vec_sl(xxxx, penalty /* (3,...) */ );

		sads = vec_add(sads, penalty);
	    } /* }}} */


	    /* original version adds same penalty for each sad
	     * so xy adjustment must be after penalty calc.
	     */
	    xy = vec_add(xy, xy11); /* adjust xy values for elements 1-3 */

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

		/* 'or' xymask to sads thereby forcing
		 * masked values above the threshold.
		 */
		sads = vec_or(sads, vu32(xymask));
	    } /* }}} */
	} /* }}} */

	/* find sads lower than minsad */
	minsel = vec_cmplt(sads, minsad);

	minsad = vec_sel(minsad, sads, minsel);
	minxy = vec_sel(minxy, xy, vb8(minsel));

#define minsad32 vu32(t0)
#define minxy32  vs8(t1)
	t0 = vu8(vec_sld(vu32(zero), vu32(minsad), 12));
	t1 = vu8(vec_sld(vu32(zero), vu32(minxy), 12));

	minsel = vec_cmplt(minsad, minsad32);
	minsad = vec_sel(minsad32, minsad, minsel);
	minxy = vec_sel(minxy32, minxy, vb8(minsel));
#undef minsad32 /* t0 */
#undef minxy32  /* t1 */

#define minsad64 vu32(t0)
#define minxy64  vs8(t1)
	t0 = vu8(vec_sld(vu32(zero), vu32(minsad), 8));
	t1 = vu8(vec_sld(vu32(zero), vu32(minxy), 8));

	minsel = vec_cmplt(minsad, minsad64);
	minsad = vec_sel(minsad64, minsad, minsel);
	minxy = vec_sel(minxy64, minxy, vb8(minsel));
#undef minsad64 /* t0 */
#undef minxy64  /* t1 */

	minsad = vec_splat(minsad, 3);
	minxy = vs8(vec_splat(vu32(minxy), 3));
	/* }}} */
    } while (--len);


    /* arrange sad and xy into me_result_s form {{{ */
    /* (   0, sad,   0, sad,   0, sad,   0, sad )
     * ( sad, sad, sad, sad, sad, sad, sad, sad )
     *
     * (   0,  xy,   0,  xy,   0,  xy,   0,  xy )
     * (  xy,  xy,  xy,  xy,  xy,  xy,  xy,  xy )
     *
     * ( sad,  xy, sad,  xy, sad,  xy, sad,  xy )
     */
    minsad = vu32(vec_pack(vu32(minsad), vu32(minsad)));
    minxy = vs8(vec_pack(vu32(minxy), vu32(minxy)));
    minsad = vu32(vec_mergeh(vu16(minsad), vu16(minxy)));
    /* }}} */

    /* store mests to vo for scalar access */
    vec_st(minsad, 0, (unsigned int*) &vio.best);

    mres = vio.best;
    if (mres.weight > 255*255)
	mres.weight = 255*255;

    *best_so_far = mres;

  AMBER_STOP;

#undef sads
}


#if ALTIVEC_TEST_FUNCTION(find_best_one_pel) /* {{{ */

#define FIND_BEST_ONE_PEL_PFMT                                               \
  "sub22set=0x%X, org=0x%X, blk=0x%X, i0=%d, j0=%d, ihigh=%d, jhigh=%d, "    \
  "rowstride=%d, h=%d, best_so_far=0x%X"

#  ifdef ALTIVEC_VERIFY
void find_best_one_pel_altivec_verify(FIND_BEST_ONE_PEL_PDECL)
{
  me_result_s best, best1, best2;

  best = *best_so_far; /* save best */
  _find_best_one_pel_altivec(FIND_BEST_ONE_PEL_ARGS, 1 /* verify */);
  best1 = *best_so_far;

  *best_so_far = best; /* restore best */
  ALTIVEC_TEST_WITH(find_best_one_pel)(FIND_BEST_ONE_PEL_ARGS);
  best2 = *best_so_far;

  if (best1.weight != best2.weight ||
      best1.x != best2.x ||
      best1.y != best2.y)
  {
    mjpeg_debug("find_best_one_pel(" FIND_BEST_ONE_PEL_PFMT ")",
		FIND_BEST_ONE_PEL_ARGS);
    mjpeg_debug("find_best_one_pel: sub22set->len=%d", sub22set->len);
    mjpeg_debug("find_best_one_pel: best_so_far "
		"{weight=%d,x=%d,y=%d} != {weight=%d,x=%d,y=%d}",
		best1.weight, best1.x, best1.y,
		best2.weight, best2.x, best2.y);
  }
}

static void verify_sads(uint8_t *blk1, uint8_t *blk2, int stride,
			int h, signed int *sads, int count)
{
  int i, d, d2, dmin;
  uint8_t *pblk;

  pblk = blk1;
  dmin = INT_MAX;

  for (i = 0; i < count; i++) {
    /* d = sad_00(blk1, blk2, stride, h, dmin); {{{ */
#if ALTIVEC_TEST_FUNCTION(sad_00)
    d = ALTIVEC_TEST_WITH(sad_00)(pblk, blk2, stride, h, dmin);
#else
    d = sad_00_altivec(pblk, blk2, stride, h, dmin);
#endif /* }}} */
    d2 = sads[i];
    if (d != d2 && d2 <= dmin) {
      mjpeg_debug("find_best_one_pel: %d[%d] != %d=sad_00"
	"(blk1=0x%X(0x%X), blk2=0x%X, stride=%d, h=%d, dmin=%d)",
	d2, i, d, pblk, blk1, blk2, stride, h, dmin);
    }

    if (i == 1)
      pblk += stride-1;
    else
      pblk += 1;
  }
}

#  else
#undef BENCHMARK_FREQUENCY
#define BENCHMARK_FREQUENCY 543

#undef BENCHMARK_EPILOG
#define BENCHMARK_EPILOG                                                     \
  mjpeg_info("find_best_one_pel: sub22set->len=%d", sub22set->len);

ALTIVEC_TEST(find_best_one_pel, void, (FIND_BEST_ONE_PEL_PDECL),
    FIND_BEST_ONE_PEL_PFMT, FIND_BEST_ONE_PEL_ARGS);
#  endif
#endif /* }}} */
/* vim:set sw=4 softtabstop=4 foldmethod=marker foldlevel=0: */
