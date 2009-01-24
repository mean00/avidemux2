/* build_sub44_mests.c, this file is part of the
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
#include "../fastintfns.h"
#include "../mjpeg_logging.h"
#include <math.h>
#include <stdlib.h>

/* #define AMBER_ENABLE */
/* #define AMBER_MAX_TRACES 10 */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif

#define USE_SMR_PPC
#ifdef USE_SMR_PPC
extern int sub_mean_reduction_ppc(int len, me_result_set *set, int reduction);
#endif


/* C skim rule */
#define SKIM(weight,threshold) (weight < threshold)

/* MMX skim rule */
/* #define SKIM(weight,threshold) (weight <= threshold) */


/* Rough and-ready absolute distance penalty
 * NOTE: This penalty is *vital* to correct operation
 * as otherwise the sub-mean filtering won't work on very
 * uniform images.
 */
/* C penalty calculation */
#define DISTANCE_PENALTY(x,y) (intmax(abs(x - i0),abs(y - j0))<<1)

/* MMX penalty calculation */
/* #define DISTANCE_PENALTY(x,y) (intmax(abs(x),abs(y))<<2) */

/* old MMX penalty calculation */
/* #define DISTANCE_PENALTY(x,y) (abs(x)+abs(y)) */



/* Do threshold lookahead? This also generates different results than
 * the C version, also slightly worse output. (should be faster though)
 */
#undef THRESHOLD_LOOKAHEAD
#define THRESHOLD

#ifdef THRESHOLD
#  define UPDATE_THRESHOLD(w,t) (t) = intmin((w) << 2, (t))
#else
#  define UPDATE_THRESHOLD(w,t)
#endif

#if 0
#undef THRESHOLD
#undef SKIM
#define SKIM(w,t) 1
#undef DISTANCE_PENALTY
#define DISTANCE_PENALTY(x,y) 0
#endif


/*
 *  s44org % 16 == 0
 *  s44blk % 4 == 0
 *  h == 2 or 4
 *  (rowstride % 16) == 0
 *  (ihigh-ilow)+1 % 16 == 0 very often
 *  (jhigh-jlow)+1 % 16 == 0 very often
 */
#define BUILD_SUB44_MESTS_PDECL /* {{{ */                                    \
  me_result_set *sub44set,                                                   \
  int ilow, int jlow, int ihigh, int jhigh,                                  \
  int i0, int j0,                                                            \
  int null_ctl_sad,                                                          \
  uint8_t *s44org, uint8_t *s44blk,                                          \
  int rowstride, int h,                                                      \
  int reduction                                                              \
  /* }}} */

#define BUILD_SUB44_MESTS_ARGS /* {{{ */                                     \
  sub44set,                                                                  \
  ilow, jlow, ihigh, jhigh,                                                  \
  i0, j0,                                                                    \
  null_ctl_sad,                                                              \
  s44org, s44blk,                                                            \
  rowstride, h,                                                              \
  reduction                                                                  \
  /* }}} */


/* int build_sub44_mests_altivec(BUILD_SUB44_MESTS_PDECL) {{{ */
#if defined(ALTIVEC_VERIFY) && ALTIVEC_TEST_FUNCTION(build_sub44_mests)
#define VERIFY_BUILD_SUB44_MESTS

#define VERIFY_SADS(orgblk,s44blk,rowstride,h,sads,count) if (verify) \
      { verify_sads(orgblk,s44blk,rowstride,h,sads,count); }

/* declarations */
static int _build_sub44_mests_altivec(BUILD_SUB44_MESTS_PDECL, int verify);
static void verify_sads(unsigned char *orgblk, unsigned char* s44blk,
			int rowstride, int h, unsigned int *sads, int count);

int build_sub44_mests_altivec(BUILD_SUB44_MESTS_PDECL)
{
  return _build_sub44_mests_altivec(BUILD_SUB44_MESTS_ARGS, 0 /* no verify */);
}

static int _build_sub44_mests_altivec(BUILD_SUB44_MESTS_PDECL, int verify)
#else
#define VERIFY_SADS(orgblk,s44blk,rowstride,h,sads,count) /* no verify */

int build_sub44_mests_altivec(BUILD_SUB44_MESTS_PDECL)
#endif
/* }}} */
{
    int i, j;
    int x, y;
    int xlow, xl, x16, xl1, xl2, xl3;
    uint8_t *currowblk, *curblk, *nextrowblk;
    int threshold;
    me_result_s *cres;
    me_result_s mres;
    vector unsigned char t1, t2, t3, perm;
    vector unsigned char shift, shifter, increment;
    vector unsigned char vr0, vr1;
    vector unsigned char vx0y0, vx16y0,
                         vx0y1, vx16y1;
    vector signed int sads;
    unsigned int *psads;
    unsigned int *psad;
#ifdef ALTIVEC_DST
    DataStreamControl dsc;
#endif
#ifndef USE_SMR_PPC
    int mean_weight;
#endif
#ifdef AMBER_ENABLE
    int stop_amber = 0;
#endif


#ifdef ALTIVEC_VERIFY /* {{{ */
    if (NOT_VECTOR_ALIGNED(s44org))
	mjpeg_error_exit1("build_sub44_mests: s44org %% 16 != 0 (0x%X)",
	    s44org);

    if (((unsigned long)s44blk) & 0x3 != 0)
	mjpeg_error_exit1("build_sub44_mests: s44blk %% 4 != 0 (0x%X)",
	    s44blk);

    if (NOT_VECTOR_ALIGNED(rowstride))
	mjpeg_error_exit1("build_sub44_mests: rowstride %% 16 != 0 (%d)",
		rowstride);

    if (h != 2 && h != 4)
	mjpeg_error_exit1("build_sub44_mests: h != [2|4], (%d)", h);
#endif /* }}} */

#ifdef AMBER_ENABLE
    /* enable amber for non-edge bound search radii */
    if (((ihigh - ilow) >> 1) == (ihigh - i0) &&
        ((jhigh - jlow) >> 1) == (jhigh - j0))
    {
	stop_amber = 1;
	AMBER_START;
    }
#endif

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(1,0,0);
    dsc.block.count = h;
    dsc.block.stride = rowstride;

    vec_dst(s44blk, dsc.control, 0);

    xl = ((ihigh - ilow) >> 2) + 1;

    currowblk = s44org+(ilow>>2)+rowstride*(jlow>>2);

    xl1 = (xl + 3 + 15 + ((unsigned long)currowblk & 0xf)) >> 4;

    dsc.block.size = xl1;

    vec_dst(currowblk, dsc.control, 1);

    dsc.block.count = 1;   /* loading one row at a time from now on */

#else

    xl = ((ihigh - ilow) >> 2) + 1;

    currowblk = s44org+(ilow>>2)+rowstride*(jlow>>2);

    xl1 = (xl + 3 + 15 + ((unsigned long)currowblk & 0xf)) >> 4;

#endif


    /* shift = (0x00010203, 0x01020304, 0x02030405, 0x03040506) {{{ */
    shift = vec_lvsl(0, (unsigned char*) 0);
    /* tmp(shifter) = (0,0,0,0, 1,1,1,1, 2,2,2,2, 3,3,3,3) {{{ */
    shifter = vec_splat_u8(2);
    shifter = vec_sr(shift /* lvsl */, shifter /* (2) */ );
    /* }}} */
    shift = vu8(vec_splat(vu32(shift), 0)); /* (0x00010203, ...) */
    shift = vec_add(shift, shifter);
    increment = vec_splat_u8(4);
    /* }}} */

    threshold = 6*null_ctl_sad / (4*4*reduction);

    y     = jlow  - j0;
    xlow  = ilow  - i0;

    perm = vec_lvsl(0, (unsigned char*)currowblk);
    perm = vec_splat(perm, 0);

    if (xl1 < 3) {
	/* if (xl1 == 1) # of vec_ld = ((yl + h - 1) */
	/* if (xl1 == 2) # of vec_ld = ((yl + h - 1) * 2) */
	x16 = 16 * (xl1 >> 1); /* if (xl1 == 1) x16=0  if (xl1 == 2) x16=16 */
	xl1 = xl;
	xl2 = 0;
	xl3 = 0;
    } else {
	int sh;
	/* else (xl1 > 2) # of vec_ld = ((yl + h - 1) * (xl1)) */
	x16 = 16;
	xl1 = 16;
	xl2 = xl - 16;

	xl3 = (((xl2 & 0xf) + 3) >> 2);
	sh = (xl2 >> 4) * 3;
	xl2 = (xl2 + 15) >> 4; /* (xl2 + 15) / 16 */
	xl3 = (xl3 << sh) | (((unsigned int)(~0) >> (32 - sh)) & 0x4924);
    }
    xl1 = (xl1 + 3) >> 2;  /* (xl1 + 3) / 4 */


    j = ((jhigh - jlow) >> 2) + 1;

    vr0 = vec_ld(0, (unsigned char*)s44blk);
    vr1 = vec_ld(rowstride, (unsigned char*)s44blk);
    t1 = vec_lvsl(0, (unsigned char*)s44blk);
    t1 = vu8(vec_splat(vu32(t1), 0));
    vr0 = vec_perm(vr0, vr0, t1);
    vr1 = vec_perm(vr1, vr1, t1);


    vx0y0 = vec_ld(0, (unsigned char*)currowblk);
    vx16y0 = vec_ld(x16, (unsigned char*)currowblk);

    cres = sub44set->mests;

    if (h < 4) {
	/* {{{ */
	nextrowblk = currowblk + rowstride;

	do /* while (--j) */
	{
	    vx0y1 = vec_ld(0, (unsigned char*)nextrowblk);
	    vx16y1 = vec_ld(x16, (unsigned char*)nextrowblk);

	    nextrowblk += rowstride;

#ifdef ALTIVEC_DST
	    vec_dst(nextrowblk, dsc.control, 0);
#endif

	    shifter = vec_add(shift, perm);

	    /* vector align for vec_st */
	    psad = psads = (unsigned int*)(((unsigned long)cres + 15) & (~0xf));

	    /* calculating sads in the X direction 4 at a time. */
	    i = xl1;
	    do
	    {
		sads = vec_splat_s32(0);

		t1 = vec_perm(vx0y0, vx16y0, shifter);
		t2 = vec_max(t1, vr0);  /* find largest of two      */  
		t3 = vec_min(t1, vr0);  /* find smaller of two      */  
		t3 = vec_sub(t2, t3);   /* find absolute difference */  
		sads = vs32(vec_sum4s(t3, vu32(sads)));

		t1 = vec_perm(vx0y1, vx16y1, shifter);
		t2 = vec_max(t1, vr1);
		t3 = vec_min(t1, vr1);
		t3 = vec_sub(t2, t3);
		sads = vs32(vec_sum4s(t3, vu32(sads)));

		vec_st(vu32(sads), 0, psad);
		psad += 4;

		/* increment permute for next iteration */
		shifter = vec_add(shifter, increment);

	    } while (--i);

	    if (xl2) {
		vector unsigned char vn0y0, vn16y0, vn0y1, vn16y1;
		int i2, i3;

		curblk = currowblk + 16; /* update to current pointer */

		vn16y0 = vec_sld(vx16y0, vx16y0, 0); /* vn16y0 = vx16y0 (VPU) */
		vn16y1 = vec_or(vx16y1, vx16y1);    /* vn16y1 = vx16y1 (VALU) */

		i = xl2;
		i2 = xl3;
		do {
		    curblk += 16; /* update to next pointer */

		    vn0y0 = vec_sld(vn16y0, vn16y0, 0);
		    vn16y0 = vec_ld(0, (unsigned char*)curblk);
		    vn0y1 = vec_or(vn16y1, vn16y1);
		    vn16y1 = vec_ld(rowstride, (unsigned char*)curblk);

		    shifter = vec_add(shift, perm);

		    i3 = i2 & 0x7;
		    i2 >>= 3;
		    do
		    {
			sads = vec_splat_s32(0);

			t1 = vec_perm(vn0y0, vn16y0, shifter);
			t2 = vec_max(t1, vr0);  /* find largest of two      */  
			t3 = vec_min(t1, vr0);  /* find smaller of two      */  
			t3 = vec_sub(t2, t3);   /* find absolute difference */  
			sads = vs32(vec_sum4s(t3, vu32(sads)));

			t1 = vec_perm(vn0y1, vn16y1, shifter);
			t2 = vec_max(t1, vr1);
			t3 = vec_min(t1, vr1);
			t3 = vec_sub(t2, t3);
			sads = vs32(vec_sum4s(t3, vu32(sads)));

			vec_st(vu32(sads), 0, psad);
			psad += 4;

			/* increment permute for next iteration */
			shifter = vec_add(shifter, increment);

		    } while (--i3);

		    /*
		    vn0y0 = vec_sld(vn16y0, vn16y0, 0);
		    vn0y1 = vec_or(vn16y1, vn16y1);
		    */

		} while (--i);
	    }
	    
#ifdef ALTIVEC_VERIFY
	    VERIFY_SADS(currowblk, s44blk, rowstride, h, psads, xl);
#endif
	    psad = psads;
	    mres.y = (int8_t)y;
	    x = xlow;
	    i = xl >> 2;
	    while (i--) {
		int w0, w1, w2, w3, tx;

		w0 = *psad;
		psad++;
		w1 = *psad;
		psad++;
		w2 = *psad;
		psad++;
		w3 = *psad;
		psad++;

		if (SKIM(w0, threshold)) {
		    UPDATE_THRESHOLD(w0,threshold);
		    mres.weight = (uint16_t)(w0 + DISTANCE_PENALTY(x,y));
		    mres.x = (int8_t)x;
		    *cres = mres;
		    cres++;
		}
		if (SKIM(w1, threshold)) {
		    UPDATE_THRESHOLD(w1,threshold);
		    tx = x + 4;
		    mres.weight = (uint16_t)(w1 + DISTANCE_PENALTY(tx,y));
		    mres.x = (int8_t)tx;
		    *cres = mres;
		    cres++;
		}
		if (SKIM(w2, threshold)) {
		    UPDATE_THRESHOLD(w2,threshold);
		    tx = x + 8;
		    mres.weight = (uint16_t)(w2 + DISTANCE_PENALTY(tx,y));
		    mres.x = (int8_t)tx;
		    *cres = mres;
		    cres++;
		}
		if (SKIM(w3, threshold)) {
		    UPDATE_THRESHOLD(w3,threshold);
		    tx = x + 12;
		    mres.weight = (uint16_t)(w3 + DISTANCE_PENALTY(tx,y));
		    mres.x = (int8_t)tx;
		    *cres = mres;
		    cres++;
		}
		x += 16;
	    }

	    i = xl & 0x3;
	    while (i--) {
		int weight = *psad;
		psad++;

		if (SKIM(weight, threshold)) {
		    UPDATE_THRESHOLD(weight,threshold);
		    mres.weight = (uint16_t)(weight + DISTANCE_PENALTY(x,y));
		    mres.x = (int8_t)x;
		    *cres = mres;
		    cres++;
		}

		x += 4;
	    }

	    currowblk += rowstride;

	    vx0y0 = vec_sld(vx0y1, vx0y1, 0); /* vx0y0  = vx0y1  (VPU) */
	    vx16y0 = vec_or(vx16y1, vx16y1);  /* vx16y0 = vx16y1 (VALU) */

	    y += 4;

	} while (--j);
	/* }}} */
    } else /* h == 4 */ {
	/* {{{ */
	vector unsigned char vr2, vr3;
	vector unsigned char vx0y2, vx16y2,
			     vx0y3, vx16y3;
	int rowstride2, rowstride3;
	
	rowstride2 = rowstride + rowstride;
	vr2 = vec_ld(rowstride2, (unsigned char*)s44blk);
	rowstride3 = rowstride2 + rowstride;
	vr3 = vec_ld(rowstride3, (unsigned char*)s44blk);
	vr2 = vec_perm(vr2, vr2, t1);
	vr3 = vec_perm(vr3, vr3, t1);

	curblk = currowblk + rowstride;
	vx0y1 = vec_ld(0, (unsigned char*)curblk);
	vx16y1 = vec_ld(x16, (unsigned char*)curblk);
	curblk += rowstride;
	vx0y2 = vec_ld(0, (unsigned char*)curblk);
	vx16y2 = vec_ld(x16, (unsigned char*)curblk);

	nextrowblk = curblk + rowstride;

	do
	{
	    vx0y3 = vec_ld(0, (unsigned char*)nextrowblk);
	    vx16y3 = vec_ld(x16, (unsigned char*)nextrowblk);

	    nextrowblk += rowstride;

#ifdef ALTIVEC_DST
	    vec_dst(nextrowblk, dsc.control, 0);
#endif

	    shifter = vec_add(shift, perm);

	    psad = psads = (unsigned int*)(((unsigned long)cres + 15) & (~0xf));

	    /* calculating sads in the X direction 4 at a time. */
	    i = xl1;
	    do
	    {
		sads = vec_splat_s32(0);

		t1 = vec_perm(vx0y0, vx16y0, shifter);
		t2 = vec_max(t1, vr0);  /* find largest of two      */  
		t3 = vec_min(t1, vr0);  /* find smaller of two      */  
		t3 = vec_sub(t2, t3);   /* find absolute difference */  
		sads = vs32(vec_sum4s(t3, vu32(sads)));

		t1 = vec_perm(vx0y1, vx16y1, shifter);
		t2 = vec_max(t1, vr1);
		t3 = vec_min(t1, vr1);
		t3 = vec_sub(t2, t3);
		sads = vs32(vec_sum4s(t3, vu32(sads)));

		t1 = vec_perm(vx0y2, vx16y2, shifter);
		t2 = vec_max(t1, vr2);
		t3 = vec_min(t1, vr2);
		t3 = vec_sub(t2, t3);
		sads = vs32(vec_sum4s(t3, vu32(sads)));

		t1 = vec_perm(vx0y3, vx16y3, shifter);
		t2 = vec_max(t1, vr3);
		t3 = vec_min(t1, vr3);
		t3 = vec_sub(t2, t3);
		sads = vs32(vec_sum4s(t3, vu32(sads)));

		vec_st(vu32(sads), 0, psad);
		psad += 4;

		/* increment permute for next iteration */
		shifter = vec_add(shifter, increment);

	    } while (--i);

	    if (xl2) {
		vector unsigned char vn0y0, vn16y0, vn0y1, vn16y1;
		vector unsigned char vn0y2, vn16y2, vn0y3, vn16y3;
		int i2, i3;

		curblk = currowblk + 16; /* update to current pointer */

		vn16y0 = vec_sld(vx16y0, vx16y0, 0);
		vn16y1 = vec_or(vx16y1, vx16y1);    
		vn16y2 = vec_sld(vx16y2, vx16y2, 0);
		vn16y3 = vec_or(vx16y3, vx16y3);    

		i = xl2;
		i2 = xl3;
		do {
		    curblk += 16; /* update to next pointer */

		    vn0y0 = vec_sld(vn16y0, vn16y0, 0);
		    vn16y0 = vec_ld(0, (unsigned char*)curblk);
		    vn0y1 = vec_or(vn16y1, vn16y1);
		    vn16y1 = vec_ld(rowstride, (unsigned char*)curblk);
		    vn0y2 = vec_sld(vn16y2, vn16y2, 0);
		    vn16y2 = vec_ld(rowstride2, (unsigned char*)curblk);
		    vn0y3 = vec_or(vn16y3, vn16y3);
		    vn16y3 = vec_ld(rowstride3, (unsigned char*)curblk);

		    shifter = vec_add(shift, perm);

		    i3 = i2 & 0x7;
		    i2 >>= 3;
		    do
		    {
			sads = vec_splat_s32(0);

			t1 = vec_perm(vn0y0, vn16y0, shifter);
			t2 = vec_max(t1, vr0);  /* find largest of two      */  
			t3 = vec_min(t1, vr0);  /* find smaller of two      */  
			t3 = vec_sub(t2, t3);   /* find absolute difference */  
			sads = vs32(vec_sum4s(t3, vu32(sads)));

			t1 = vec_perm(vn0y1, vn16y1, shifter);
			t2 = vec_max(t1, vr1);
			t3 = vec_min(t1, vr1);
			t3 = vec_sub(t2, t3);
			sads = vs32(vec_sum4s(t3, vu32(sads)));

			t1 = vec_perm(vn0y2, vn16y2, shifter);
			t2 = vec_max(t1, vr2);
			t3 = vec_min(t1, vr2);
			t3 = vec_sub(t2, t3);
			sads = vs32(vec_sum4s(t3, vu32(sads)));

			t1 = vec_perm(vn0y3, vn16y3, shifter);
			t2 = vec_max(t1, vr3);
			t3 = vec_min(t1, vr3);
			t3 = vec_sub(t2, t3);
			sads = vs32(vec_sum4s(t3, vu32(sads)));

			vec_st(vu32(sads), 0, psad);
			psad += 4;

			/* increment permute for next iteration */
			shifter = vec_add(shifter, increment);

		    } while (--i3);

		} while (--i);
	    }

#ifdef ALTIVEC_VERIFY
	    VERIFY_SADS(currowblk, s44blk, rowstride, h, psads, xl);
#endif
	    psad = psads;
	    mres.y = (int8_t)y;
	    x = xlow;
	    i = xl >> 2;
	    while (i--) {
		int w0, w1, w2, w3, tx;

		w0 = *psad;
		psad++;
		w1 = *psad;
		psad++;
		w2 = *psad;
		psad++;
		w3 = *psad;
		psad++;

		if (SKIM(w0, threshold)) {
		    UPDATE_THRESHOLD(w0,threshold);
		    mres.weight = (uint16_t)(w0 + DISTANCE_PENALTY(x,y));
		    mres.x = (int8_t)x;
		    *cres = mres;
		    cres++;
		}
		if (SKIM(w1, threshold)) {
		    UPDATE_THRESHOLD(w1,threshold);
		    tx = x + 4;
		    mres.weight = (uint16_t)(w1 + DISTANCE_PENALTY(tx,y));
		    mres.x = (int8_t)tx;
		    *cres = mres;
		    cres++;
		}
		if (SKIM(w2, threshold)) {
		    UPDATE_THRESHOLD(w2,threshold);
		    tx = x + 8;
		    mres.weight = (uint16_t)(w2 + DISTANCE_PENALTY(tx,y));
		    mres.x = (int8_t)tx;
		    *cres = mres;
		    cres++;
		}
		if (SKIM(w3, threshold)) {
		    UPDATE_THRESHOLD(w3,threshold);
		    tx = x + 12;
		    mres.weight = (uint16_t)(w3 + DISTANCE_PENALTY(tx,y));
		    mres.x = (int8_t)tx;
		    *cres = mres;
		    cres++;
		}
		x += 16;
	    }

	    i = xl & 0x3;
	    while (i--) {
		int weight = *psad;
		psad++;

		if (SKIM(weight, threshold)) {
		    UPDATE_THRESHOLD(weight,threshold);
		    mres.weight = (uint16_t)(weight + DISTANCE_PENALTY(x,y));
		    mres.x = (int8_t)x;
		    *cres = mres;
		    cres++;
		}

		x += 4;
	    }

	    currowblk += rowstride;

	    vx0y0 = vec_sld(vx0y1, vx0y1, 0); /* vx0y0  = vx0y1  (VPU) */
	    vx16y0 = vec_or(vx16y1, vx16y1);  /* vx16y0 = vx16y1 (VALU) */
	    vx0y1 = vec_sld(vx0y2, vx0y2, 0); /* vx0y1  = vx0y2  (VPU) */
	    vx16y1 = vec_or(vx16y2, vx16y2);  /* vx16y1 = vx16y2 (VALU) */
	    vx0y2 = vec_sld(vx0y3, vx0y3, 0); /* vx0y2  = vx0y3  (VPU) */
	    vx16y2 = vec_or(vx16y3, vx16y3);  /* vx16y2 = vx16y3 (VALU) */

	    y += 4;

	} while (--j);
	/* }}} */
    }

#ifdef ALTIVEC_DST
    vec_dss(0);
#endif

    /* sub44set->len = cres - sub44set->mests; */
    xl = cres - sub44set->mests;
    sub44set->len = xl;

#ifdef AMBER_ENABLE
    if (stop_amber) {
	AMBER_STOP;
    }
#endif

#ifdef USE_SMR_PPC
    if (xl > 1)
	xl = sub_mean_reduction_ppc(xl, sub44set, 1+(reduction > 1));
    return xl;
#else
#  if ALTIVEC_TEST_FUNCTION(sub_mean_reduction)
    ALTIVEC_TEST_SUFFIX(sub_mean_reduction)(sub44set, 1+(reduction>1), &mean_weight);
#  else
    ALTIVEC_SUFFIX(sub_mean_reduction)(sub44set, 1+(reduction>1), &mean_weight);
#  endif
  return sub44set->len;
#endif
}

#if 0 /* build_sub44_mests_altivec_test {{{ */
int build_sub44_mests_altivec_test(me_result_set *sub44set,
	int ilow, int jlow, int ihigh, int jhigh, 
	int i0, int j0,
	int null_ctl_sad,
	uint8_t *s44org, uint8_t *s44blk, 
	int qrowstride, int qh,
	int reduction)
{
    uint8_t *s44orgblk;
    me_result_s *sub44_mests = sub44set->mests;
    int istrt = ilow-i0;
    int jstrt = jlow-j0;
    int iend = ihigh-i0;
    int jend = jhigh-j0;
    int mean_weight;
    int threshold;

    int i,j;
    int s1;
    uint8_t *old_s44orgblk;
    int sub44_num_mests;

    threshold = 6*null_ctl_sad / (4*4*reduction);
    s44orgblk = s44org+(ilow>>2)+qrowstride*(jlow>>2);

    sub44_num_mests = 0;

    s44orgblk = s44org+(ilow>>2)+qrowstride*(jlow>>2);
    for (j = jstrt; j <= jend; j += 4)
    {
	old_s44orgblk = s44orgblk;
	for (i = istrt; i <= iend; i += 4)
	{
	    s1 = ((*psad_sub44)( s44orgblk,s44blk,qrowstride,qh) & 0xffff);
#ifdef THRESHOLD
	    if (s1 < threshold)
	    {
		threshold = intmin(s1<<2,threshold);
#endif
		sub44_mests[sub44_num_mests].x = i;
		sub44_mests[sub44_num_mests].y = j;
		sub44_mests[sub44_num_mests].weight = s1 + 
		    DISTANCE_PENALTY(i,j);
		++sub44_num_mests;
#ifdef THRESHOLD
	    }
#endif
	    s44orgblk += 1;
	}
	s44orgblk = old_s44orgblk + qrowstride;
    }
    sub44set->len = sub44_num_mests;

#if 0
    sub_mean_reduction(sub44set, 1+(reduction>1),  &mean_weight);
#endif

    return sub44set->len;
}
#endif /* }}} */

#if ALTIVEC_TEST_FUNCTION(build_sub44_mests) /* {{{ */

#define BUILD_SUB44_MESTS_PFMT                                               \
  "sub44set=0x%X, ilow=%d, jlow=%d, ihigh=%d, jhigh=%d, i0=%d, j0=%d, "      \
  "null_ctl_sad=%d, s44org=0x%X, s44blk=0x%X, qrowstride=%d, qh=%d, "        \
  "reduction=%d" 

#  ifdef ALTIVEC_VERIFY
int build_sub44_mests_altivec_verify(BUILD_SUB44_MESTS_PDECL)
{
  int i, len1, len2;
  unsigned long checksum1, checksum2;

  len1 = _build_sub44_mests_altivec(BUILD_SUB44_MESTS_ARGS, 1 /*verify*/);
  for (checksum1 = i = 0; i < len1; i++) {
    checksum1 += sub44set->mests[i].weight;
    checksum1 += abs(sub44set->mests[i].x);
    checksum1 += abs(sub44set->mests[i].y);
  }

  len2 = ALTIVEC_TEST_WITH(build_sub44_mests)(BUILD_SUB44_MESTS_ARGS);
  for (checksum2 = i = 0; i < len2; i++) {
    checksum2 += sub44set->mests[i].weight;
    checksum2 += abs(sub44set->mests[i].x);
    checksum2 += abs(sub44set->mests[i].y);
  }

  if (len1 != len2 || checksum1 != checksum2) {
    mjpeg_debug("build_sub44_mests(" BUILD_SUB44_MESTS_PFMT ")",
	BUILD_SUB44_MESTS_ARGS);
    mjpeg_debug("build_sub44_mests: checksums differ %d[%d] != %d[%d]",
	checksum1, len1, checksum2, len2);
#if 1
      len1 = _build_sub44_mests_altivec(BUILD_SUB44_MESTS_ARGS, 0 /*verify*/);
      for (i = 0; i < len1; i++) {
	mjpeg_debug("A: %3d, %3d, %5d",
	    sub44set->mests[i].x,
	    sub44set->mests[i].y,
	    sub44set->mests[i].weight);
      }

      len2 = ALTIVEC_TEST_WITH(build_sub44_mests)(BUILD_SUB44_MESTS_ARGS);
      for (i = 0; i < len2; i++) {
	mjpeg_debug("C: %3d, %3d, %5d",
	    sub44set->mests[i].x,
	    sub44set->mests[i].y,
	    sub44set->mests[i].weight);
      }
#endif
  }

  return len2;
}

static void verify_sads(unsigned char *orgblk, unsigned char* s44blk,
			int rowstride, int h,
                        unsigned int *sads, int count)
{
    unsigned int i, weight, cweight;
    unsigned char *pblk;

    pblk = orgblk;
    for (i = 0; i < count; i++)
    {
      weight = sads[i];
      /* pblk = orgblk + (rowstride * i); */
#if ALTIVEC_TEST_FUNCTION(sad_sub44)
      cweight = ALTIVEC_TEST_WITH(sad_sub44)(pblk,s44blk,rowstride,h) & 0xffff;
#else
      cweight = sad_sub44(pblk,s44blk,rowstride,h) & 0xffff;
#endif
      if (weight != cweight)
	mjpeg_debug("build_sub44_mests: %d != %d="
	  "sad_sub44(blk1=0x%X, blk2=0x%X, rowstride=%d, h=%d)",
	  weight, cweight, pblk, s44blk, rowstride, h);
      pblk++;
    }
}

#  else
#undef BENCHMARK_FREQUENCY
#define BENCHMARK_FREQUENCY  543   /* benchmark every (n) calls */

#undef BENCHMARK_EPILOG
#define BENCHMARK_EPILOG \
    mjpeg_info("build_sub44_mests: (ihigh-ilow)/4+1=%d, (jhigh-jlow)/4+1=%d", \
	(ihigh-ilow)/4+1, (jhigh-jlow)/4+1);

ALTIVEC_TEST(build_sub44_mests, int, (BUILD_SUB44_MESTS_PDECL),
    BUILD_SUB44_MESTS_PFMT, BUILD_SUB44_MESTS_ARGS);
#  endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
