/* add_pred.c, this file is part of the
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

#include "altivec_transform.h"
#include "vectorize.h"
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


#define ADD_PRED_PDECL uint8_t *pred, uint8_t *cur, int lx, int16_t *blk
#define ADD_PRED_ARGS pred, cur, lx, blk
#define ADD_PRED_PFMT "pred=0x%X, cur=0x%X, lx=%d, blk=0x%X"

/*
 * add prediction and prediction error, saturate to 0...255
 * pred % 8 == 0
 * cur % 8 == 0
 * lx % 16 == 0
 * blk % 16 == 0
 */
void add_pred_altivec(ADD_PRED_PDECL)
{
#ifdef ALTIVEC_DST
    unsigned int dst;
#endif
    uint8_t *pCA, *pCB, *pPA, *pPB;
    int16_t *pBA, *pBB;
    vector unsigned char zero;
    vector unsigned char predA, predB, curA, curB;
    vector signed short blkA, blkB;


#ifdef ALTIVEC_VERIFY /* {{{ */
    if ((((unsigned long)pred) & 0x7) != 0)
	mjpeg_error_exit1("add_pred: pred %% 8 != 0, (0x%X)", pred);

    if ((((unsigned long)cur) & 0x7) != 0)
	mjpeg_error_exit1("add_pred: cur %% 8 != 0, (0x%X)", cur);

    if (NOT_VECTOR_ALIGNED(lx))
	mjpeg_error_exit1("add_pred: lx %% 16 != 0, (%d)", lx);

    if (NOT_VECTOR_ALIGNED(blk))
	mjpeg_error_exit1("add_pred: blk %% 16 != 0, (%d)", blk);

    if (((unsigned long)pred & 0xf) != ((unsigned long)cur & 0xf))
	mjpeg_error_exit1("add_pred: (pred(0x%X) %% 16) != (cur(0x%X) %% 16)",
		pred, cur);
#ifdef ALTIVEC_DST
    if (lx & (~0xffff) != 0)
	mjpeg_error_exit1("add_pred: lx=%d > vec_dst range", lx);
#endif
#endif /* }}} */

/* MACROS expand differently depending on input */
#define ABBA(symbol,ab)		_ABBA(ABBA_##ab,symbol) /* {{{ */
#define _ABBA(abba_ab,symbol)	abba_ab(symbol)
#define ABBA_A(symbol)		symbol##B
#define ABBA_B(symbol)		symbol##A
/* }}} */
#define HLLH(symbol,hl)		_HLLH(HLLH_##hl,symbol) /* {{{ */
#define _HLLH(hllh_hl,symbol)	hllh_hl(symbol)
#define HLLH_h(symbol)		symbol##l
#define HLLH_l(symbol)		symbol##h
/* }}} */
#define PACKSU(hl,st,ld)	_PACKSU(PACKSU_##hl,st,ld) /* {{{ */
#define _PACKSU(psu,st,ld)	psu(st,ld)
#define PACKSU_h(st,ld)		vec_packsu(st,ld)
#define PACKSU_l(st,ld)		vec_packsu(ld,st)
/* }}} */


#define	PERFORM_ITERATION(hl,ab,iter) /* iter {{{ */                         \
	pred##ab = vec_merge##hl(zero, pred##ab);                            \
	cur##ab = HLLH(vec_merge,hl)(zero, cur##ab);                         \
	blk##ab = vec_add(blk##ab, vs16(pred##ab));                          \
	blk##ab = vec_max(blk##ab, vs16(zero));                              \
	cur##ab = PACKSU(hl, vu16(blk##ab), vu16(cur##ab));                  \
	vec_st(cur##ab, 0, pC##ab);                                          \
	/* }}} */

#define PREPARE_ITERATION(hl,ab,iter) /* iter {{{ */                         \
	pP##ab = ABBA(pP,ab) + lx;                                           \
	pC##ab = ABBA(pC,ab) + lx;                                           \
	pB##ab = ABBA(pB,ab) + 8;                                            \
	pred##ab = vec_ld(0, pP##ab);                                        \
	cur##ab = vec_ld(0, pC##ab);                                         \
	blk##ab = vec_ld(0, pB##ab);                                         \
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
    vec_dst(pBA, dst, 2);
#endif

    predA = vec_ld(0, pPA);
    curA = vec_ld(0, pCA);  NO_RESCHEDULE;
    pPB = pPA + lx;         NO_RESCHEDULE;
    blkA = vec_ld(0, pBA);  NO_RESCHEDULE;
    pCB = pCA + lx;         NO_RESCHEDULE;
    predB = vec_ld(0, pPB); NO_RESCHEDULE;
    pBB = pBA + 8;          NO_RESCHEDULE;
    curB = vec_ld(0, pCB);  NO_RESCHEDULE;
    zero = vec_splat_u8(0); NO_RESCHEDULE;
    blkB = vec_ld(0, pBB);


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


#if ALTIVEC_TEST_FUNCTION(add_pred) /* {{{ */
#  ifdef ALTIVEC_VERIFY

void add_pred_altivec_verify(ADD_PRED_PDECL)
{
    int i, j;
    unsigned long checksum1, checksum2;
    uint8_t *pcur;
    uint8_t curcpy[8][8];

    add_pred_altivec(ADD_PRED_ARGS);
    pcur = cur;
    checksum1 = 0;
    for (j = 0; j < 8; j++) {
	for (i = 0; i < 8; i++) {
	    checksum1 += pcur[i];
	    curcpy[j][i] = pcur[i];
	}
	pcur += lx;
    }

    ALTIVEC_TEST_WITH(add_pred)(ADD_PRED_ARGS);
    pcur = cur;
    checksum2 = 0;
    for (j = 0; j < 8; j++) {
	for (i = 0; i < 8; i++)
	    checksum2 += pcur[i];
	pcur += lx;
    }

    if (checksum1 != checksum2) {
	mjpeg_debug("add_pred(" ADD_PRED_PFMT ")", ADD_PRED_ARGS);
	mjpeg_debug("add_pred: checksums differ %d != %d",
	    checksum1, checksum2);

	pcur = cur;
	checksum1 = 0;
	for (j = 0; j < 8; j++) {
	    for (i = 0; i < 8; i++) {
		if (curcpy[j][i] != pcur[i])
		    mjpeg_debug("add_pred: cur[%d][%d] %d != %d",
			j, i, curcpy[j][i], pcur[i]);
	    }
	    pcur += lx;
	}
    }
}

#  else
ALTIVEC_TEST(add_pred, void, (ADD_PRED_PDECL), ADD_PRED_PFMT, ADD_PRED_ARGS);
#  endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
