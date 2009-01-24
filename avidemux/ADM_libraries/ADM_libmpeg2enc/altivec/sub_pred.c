/* sub_pred.c, this file is part of the
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


#define SUB_PRED_PDECL uint8_t *pred, uint8_t *cur, int lx, int16_t *blk
#define SUB_PRED_ARGS pred, cur, lx, blk
#define SUB_PRED_PFMT "pred=0x%X, cur=0x%X, lx=%d, blk=0x%X"

/*
 * subtract prediction from block data
 * pred % 8 == 0
 * cur % 8 == 0
 * lx % 16 == 0
 * blk % 16 == 0
 */
void sub_pred_altivec(SUB_PRED_PDECL)
{
    unsigned int dst;
    uint8_t *pCA, *pCB, *pPA, *pPB;
    int16_t *pBA, *pBB;
    vector unsigned char zero;
    vector unsigned char predA, predB, curA, curB;
    vector signed short blkA, blkB;


#ifdef ALTIVEC_VERIFY /* {{{ */
    if ((((unsigned long)pred) & 0x7) != 0)
	mjpeg_error_exit1("sub_pred: pred %% 8 != 0, (0x%X)", pred);

    if ((((unsigned long)cur) & 0x7) != 0)
	mjpeg_error_exit1("sub_pred: cur %% 8 != 0, (0x%X)", cur);

    if (NOT_VECTOR_ALIGNED(lx))
	mjpeg_error_exit1("sub_pred: lx %% 16 != 0, (%d)", lx);

    if (NOT_VECTOR_ALIGNED(blk))
	mjpeg_error_exit1("sub_pred: blk %% 16 != 0, (%d)", blk);

    if (((unsigned long)pred & 0xf) != ((unsigned long)cur & 0xf))
	mjpeg_error_exit1("sub_pred: (pred(0x%X) %% 16) != (cur(0x%X) %% 16)",
	    pred, cur);

#ifdef ALTIVEC_DST
    if (lx & (~0xffff) != 0)
	mjpeg_error_exit1("sub_pred: lx > vec_dst range", lx);
#endif
#endif /* }}} */


/* A->B, B->A expand differently depending on input */
#define ABBA(symbol,ab)		_ABBA(ABBA_##ab,symbol) /* {{{ */
#define _ABBA(abba_ab,symbol)	abba_ab(symbol)
#define ABBA_A(symbol)		symbol##B
#define ABBA_B(symbol)		symbol##A
/* }}} */


#define	PERFORM_ITERATION(hl,ab,iter) /* iter {{{ */                         \
	pred##ab = vec_merge##hl(zero, pred##ab);                            \
	cur##ab = vec_merge##hl(zero, cur##ab);                              \
	blk##ab = vec_sub(vs16(cur##ab), vs16(pred##ab));                    \
	vec_st(blk##ab, 0, (signed short*)pB##ab);                           \
	/* }}} */

#define PREPARE_ITERATION(hl,ab,iter) /* iter {{{ */                         \
	pP##ab = ABBA(pP,ab) + lx;                                           \
	pC##ab = ABBA(pC,ab) + lx;                                           \
	pB##ab = ABBA(pB,ab) + 8;                                            \
	pred##ab = vec_ld(0, pP##ab);                                        \
	cur##ab = vec_ld(0, pC##ab);                                         \
	/* }}} */

#define NO_RESCHEDULE	asm volatile ("")

    AMBER_START;

    pPA = pred;
    pCA = cur;
    pBA = blk;

#ifdef ALTIVEC_DST
    dst = 0x01080000 | lx;
    vec_dst(pPA, dst, 0);
    vec_dst(pCA, dst, 1);
    dst = 0x01080010;
    vec_dstst(pBA, dst, 2);
#endif

    pPB = pPA + lx;         NO_RESCHEDULE;
    predA = vec_ld(0, pPA); NO_RESCHEDULE;
    pCB = pCA + lx;         NO_RESCHEDULE;
    curA = vec_ld(0, pCA);  NO_RESCHEDULE;
    pBB = pBA + 8;          NO_RESCHEDULE;
    predB = vec_ld(0, pPB); NO_RESCHEDULE;
    zero = vec_splat_u8(0); NO_RESCHEDULE;
    curB = vec_ld(0, pCB);

    if (VECTOR_ALIGNED(pPA)) {
	PERFORM_ITERATION(h,A,0);
	PREPARE_ITERATION(h,A,2);   /* prepare next A iteration */
	PERFORM_ITERATION(h,B,1);
	PREPARE_ITERATION(h,B,3);   /* prepare next B iteration */
	PERFORM_ITERATION(h,A,2);
	PREPARE_ITERATION(h,A,4);
	PERFORM_ITERATION(h,B,3);
	PREPARE_ITERATION(h,B,5);
	PERFORM_ITERATION(h,A,4);
	PREPARE_ITERATION(h,A,6);
	PERFORM_ITERATION(h,B,5);
	PREPARE_ITERATION(h,B,7);
	PERFORM_ITERATION(h,A,6);
	PERFORM_ITERATION(h,B,7);
    } else {
	PERFORM_ITERATION(l,A,0);
	PREPARE_ITERATION(l,A,2);   /* prepare next A iteration */
	PERFORM_ITERATION(l,B,1);
	PREPARE_ITERATION(l,B,3);   /* prepare next B iteration */
	PERFORM_ITERATION(l,A,2);
	PREPARE_ITERATION(l,A,4);
	PERFORM_ITERATION(l,B,3);
	PREPARE_ITERATION(l,B,5);
	PERFORM_ITERATION(l,A,4);
	PREPARE_ITERATION(l,A,6);
	PERFORM_ITERATION(l,B,5);
	PREPARE_ITERATION(l,B,7);
	PERFORM_ITERATION(l,A,6);
	PERFORM_ITERATION(l,B,7);
    }

#ifdef ALTIVEC_DST
    vec_dssall();
#endif

    AMBER_STOP;
}


#if ALTIVEC_TEST_FUNCTION(sub_pred) /* {{{ */
#  ifdef ALTIVEC_VERIFY

void sub_pred_altivec_verify(SUB_PRED_PDECL)
{
    int i;
    unsigned long checksum1, checksum2;
    signed short blkcpy[8*8];

    sub_pred_altivec(SUB_PRED_ARGS);
    for (checksum1 = i = 0; i < 8*8; i++)
	checksum1 += abs(blk[i]);

    memcpy(blkcpy, blk, 8*8*sizeof(short));

    ALTIVEC_TEST_WITH(sub_pred)(SUB_PRED_ARGS);
    for (checksum2 = i = 0; i < 8*8; i++)
	checksum2 += abs(blk[i]);

    if (checksum1 != checksum2) {
	mjpeg_debug("sub_pred(" SUB_PRED_PFMT ")", SUB_PRED_ARGS);
	mjpeg_debug("sub_pred: checksums differ %d != %d",
	    checksum1, checksum2);

	for (i = 0; i < 8*8; i++) {
	    if (blkcpy[i] != blk[i]) {
		mjpeg_debug("sub_pred: blk[%d] %d != %d",
		    i, blkcpy[i], blk[i]);
	    }
	}
    }
}

#  else
ALTIVEC_TEST(sub_pred, void, (SUB_PRED_PDECL), SUB_PRED_PFMT, SUB_PRED_ARGS);
#  endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
