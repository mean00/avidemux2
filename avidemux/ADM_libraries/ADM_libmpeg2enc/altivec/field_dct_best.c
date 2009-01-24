/* field_dct_best.c, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2003  James Klicman <james@klicman.org>
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

#include "altivec_transform.h"
#include "vectorize.h"
#include "../mjpeg_logging.h"

#include <math.h> /* sqrt */

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


#define FRAME_DCT 0 /* frame DCT */
#define FIELD_DCT 1 /* field DCT */


#define FIELD_DCT_BEST_PDECL uint8_t *cur_lum_mb, uint8_t *pred_lum_mb, int stride
#define FIELD_DCT_BEST_ARGS cur_lum_mb, pred_lum_mb, stride


int field_dct_best_altivec(FIELD_DCT_BEST_PDECL)
{
    /*
     * calculate prediction error (cur-pred) for top (blk0)
     * and bottom field (blk1)
     */
    double r, d;
    int sumtop, sumbot, sumsqtop, sumsqbot, sumbottop;
    int topvar, botvar;
    int whichdct;

    int i;
    vector unsigned char ct, pt, cb, pb;
    vector unsigned char *ctp, *ptp, *cbp, *pbp;
    unsigned int offset, stride2;
    vector signed short cur, pred;
    vector signed short dift, difb;
    vector signed int vsumtop, vsumbot, vsumsqtop, vsumsqbot, vsumbottop;
    vector signed int t0, t1, t2, t3;
    vector signed int zero;
    union {
	vector signed int v;
	struct {
	    signed int top;
	    signed int bot;
	    signed int sqtop;
	    signed int sqbot;
	} sum;
	struct {
	    signed int pad[3];
	    signed int sum;
	} bottop;
    } vo;


    AMBER_START;


#ifdef ALTIVEC_VERIFY
    if (NOT_VECTOR_ALIGNED(cur_lum_mb))
	mjpeg_error_exit1("field_dct_best: cur_lum_mb %% 16 != 0, (%d)\n",
	    cur_lum_mb);

    if (NOT_VECTOR_ALIGNED(pred_lum_mb))
	mjpeg_error_exit1("field_dct_best: pred_lum_mb %% 16 != 0, (%d)\n",
	    pred_lum_mb);

    if (NOT_VECTOR_ALIGNED(stride))
	mjpeg_error_exit1("field_dct_best: stride %% 16 != 0, (%d)\n", stride);
#endif


    zero = vec_splat_s32(0);
    vsumtop = vec_splat_s32(0);
    vsumbot = vec_splat_s32(0);
    vsumsqtop = vec_splat_s32(0);
    vsumsqbot = vec_splat_s32(0);
    vsumbottop = vec_splat_s32(0);

    ctp = (vector unsigned char*) cur_lum_mb;
    ptp = (vector unsigned char*) pred_lum_mb;
    cbp = (vector unsigned char*)(cur_lum_mb + stride);
    pbp = (vector unsigned char*)(pred_lum_mb + stride);
    offset = 0;
    stride2 = stride << 1;

#if 1
    ct = vec_ld(offset, ctp);
    pt = vec_ld(offset, ptp);
    cb = vec_ld(offset, cbp);
    pb = vec_ld(offset, pbp);

    i = 16/2 - 1;
    do {
	cur = (vector signed short)vec_mergeh(vu8(zero), ct);
	pred = (vector signed short)vec_mergeh(vu8(zero), pt);
	dift = vec_sub(cur, pred);

	cur = (vector signed short)vec_mergeh(vu8(zero), cb);
	pred = (vector signed short)vec_mergeh(vu8(zero), pb);
	difb = vec_sub(cur, pred);

	vsumtop = vec_sum4s(dift, vsumtop);
	vsumbot = vec_sum4s(difb, vsumbot);

	vsumsqtop = vec_msum(dift, dift, vsumsqtop);
	vsumsqbot = vec_msum(difb, difb, vsumsqbot);

	vsumbottop = vec_msum(dift, difb, vsumbottop);

	cur = (vector signed short)vec_mergel(vu8(zero), ct);
	pred = (vector signed short)vec_mergel(vu8(zero), pt);
	dift = vec_sub(cur, pred);

	cur = (vector signed short)vec_mergel(vu8(zero), cb);
	pred = (vector signed short)vec_mergel(vu8(zero), pb);
	difb = vec_sub(cur, pred);

	offset += stride2;
	ct = vec_ld(offset, ctp);
	pt = vec_ld(offset, ptp);
	cb = vec_ld(offset, cbp);
	pb = vec_ld(offset, pbp);

	vsumtop = vec_sum4s(dift, vsumtop);
	vsumbot = vec_sum4s(difb, vsumbot);

	vsumsqtop = vec_msum(dift, dift, vsumsqtop);
	vsumsqbot = vec_msum(difb, difb, vsumsqbot);

	vsumbottop = vec_msum(dift, difb, vsumbottop);
    } while (--i);
    cur = (vector signed short)vec_mergeh(vu8(zero), ct);
    pred = (vector signed short)vec_mergeh(vu8(zero), pt);
    dift = vec_sub(cur, pred);

    cur = (vector signed short)vec_mergeh(vu8(zero), cb);
    pred = (vector signed short)vec_mergeh(vu8(zero), pb);
    difb = vec_sub(cur, pred);

    vsumtop = vec_sum4s(dift, vsumtop);
    vsumbot = vec_sum4s(difb, vsumbot);

    vsumsqtop = vec_msum(dift, dift, vsumsqtop);
    vsumsqbot = vec_msum(difb, difb, vsumsqbot);

    vsumbottop = vec_msum(dift, difb, vsumbottop);

    cur = (vector signed short)vec_mergel(vu8(zero), ct);
    pred = (vector signed short)vec_mergel(vu8(zero), pt);
    dift = vec_sub(cur, pred);

    cur = (vector signed short)vec_mergel(vu8(zero), cb);
    pred = (vector signed short)vec_mergel(vu8(zero), pb);
    difb = vec_sub(cur, pred);

    vsumtop = vec_sum4s(dift, vsumtop);
    vsumbot = vec_sum4s(difb, vsumbot);

    vsumsqtop = vec_msum(dift, dift, vsumsqtop);
    vsumsqbot = vec_msum(difb, difb, vsumsqbot);

    vsumbottop = vec_msum(dift, difb, vsumbottop);
#else
    for (i = 0; i < 16/2; i++) { /* {{{ */
	ct = vec_ld(offset, ctp);
	pt = vec_ld(offset, ptp);
	cb = vec_ld(offset, cbp);
	pb = vec_ld(offset, pbp);

	cur = (vector signed short)vec_mergeh(vu8(zero), ct);
	pred = (vector signed short)vec_mergeh(vu8(zero), pt);
	dift = vec_sub(cur, pred);

	cur = (vector signed short)vec_mergeh(vu8(zero), cb);
	pred = (vector signed short)vec_mergeh(vu8(zero), pb);
	difb = vec_sub(cur, pred);

	vsumtop = vec_sum4s(dift, vsumtop);
	vsumbot = vec_sum4s(difb, vsumbot);

	vsumsqtop = vec_msum(dift, dift, vsumsqtop);
	vsumsqbot = vec_msum(difb, difb, vsumsqbot);

	vsumbottop = vec_msum(dift, difb, vsumbottop);

	cur = (vector signed short)vec_mergel(vu8(zero), ct);
	pred = (vector signed short)vec_mergel(vu8(zero), pt);
	dift = vec_sub(cur, pred);

	cur = (vector signed short)vec_mergel(vu8(zero), cb);
	pred = (vector signed short)vec_mergel(vu8(zero), pb);
	difb = vec_sub(cur, pred);

	vsumtop = vec_sum4s(dift, vsumtop);
	vsumbot = vec_sum4s(difb, vsumbot);

	vsumsqtop = vec_msum(dift, dift, vsumsqtop);
	vsumsqbot = vec_msum(difb, difb, vsumsqbot);

	vsumbottop = vec_msum(dift, difb, vsumbottop);

	offset += stride2;
    } /* }}} */
#endif

    /* transpose [sumtop, sumbot, sumsqtop, sumsqbot] {{{ */
    t0 = vec_mergel(vsumtop, vsumsqtop);
    t1 = vec_mergeh(vsumtop, vsumsqtop);
    t2 = vec_mergel(vsumbot, vsumsqbot);
    t3 = vec_mergeh(vsumbot, vsumsqbot);
    vsumtop = vec_mergeh(t1, t3);
    vsumbot = vec_mergel(t1, t3);
    vsumsqtop = vec_mergeh(t0, t2);
    vsumsqbot = vec_mergel(t0, t2);
    /* }}} */

    /* sum final values for sumtop, sumbot, sumsqtop, sumsqbot */
    vsumtop = vec_add(vsumtop, vsumbot);
    vsumsqtop = vec_add(vsumsqtop, vsumsqbot);
    vo.v = vec_add(vsumtop, vsumsqtop);

    sumtop = vo.sum.top;
    sumbot = vo.sum.bot;
    sumsqtop = vo.sum.sqtop;
    sumsqbot = vo.sum.sqbot;

    vsumbottop = vec_sums(vsumbottop, zero);

    vo.v = vsumbottop;


    /* Calculate Variances top and bottom.  If they're of similar
       sign estimate correlation if its good use frame DCT otherwise
       use field.
     */
    whichdct = FIELD_DCT;
    r = 0.0;
    topvar = sumsqtop-sumtop*sumtop/128;
    botvar = sumsqbot-sumbot*sumbot/128;
    if (!((topvar > 0) ^ (botvar > 0)))
    {
	sumbottop = vo.bottop.sum;

	d = ((double) topvar) * ((double)botvar);
	r = (sumbottop-(sumtop*sumbot)/128);
	if (r > (0.5 * sqrt(d)))
	    whichdct = FRAME_DCT;
    }

    AMBER_STOP;

    return whichdct;
}


#if ALTIVEC_TEST_FUNCTION(field_dct_best) /* {{{ */
ALTIVEC_TEST(field_dct_best, int, (FIELD_DCT_BEST_PDECL),
  "cur_lum_mb=0x%X, pred_lum_mb=0%X, stride=%d",
  FIELD_DCT_BEST_ARGS);
#endif /* }}} */

/* vim:set foldmethod=marker foldlevel=0: */
