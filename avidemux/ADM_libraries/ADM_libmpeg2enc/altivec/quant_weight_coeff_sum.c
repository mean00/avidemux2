/* quant_weight_coeff_sum.c, this file is part of the
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

#include "altivec_quantize.h"
#include "vectorize.h"
#include "mjpeg_logging.h"
#include "quantize_precomp.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif
 

#define QUANT_WEIGHT_COEFF_INTRA_PDECL \
    struct QuantizerWorkSpace *wsp, int16_t *blk
#define QUANT_WEIGHT_COEFF_INTRA_ARGS wsp, blk
#define QUANT_WEIGHT_COEFF_INTRA_PFMT "wsp=0x%X, blk=0x%X"

#define QUANT_WEIGHT_COEFF_INTER_PDECL \
    struct QuantizerWorkSpace *wsp, int16_t *blk
#define QUANT_WEIGHT_COEFF_INTER_ARGS wsp, blk
#define QUANT_WEIGHT_COEFF_INTER_PFMT "wsp=0x%X, blk=0x%X"


static int quant_weight_coeff_sum_altivec(uint16_t *i_quant_mat, int16_t *blk)
{
    int16_t *pb;
    uint16_t *pq;
    vector signed short zero;
    vector signed short vA, vB;
    vector signed short nA, nB;
    vector signed short absA, absB;
    vector unsigned short qA, qB;
    vector signed int sum;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;

#ifdef ALTIVEC_VERIFY /* {{{ */
  if (NOT_VECTOR_ALIGNED(i_quant_mat))
    mjpeg_error_exit1("quant_weight_coeff_sum: i_quant_mat %% 16 != 0, (%d)",
	i_quant_mat);

  if (NOT_VECTOR_ALIGNED(blk))
    mjpeg_error_exit1("quant_weight_coeff_sum: blk %% 16 != 0, (%d)", blk);
#endif /* }}} */

    AMBER_START;

    pb = blk;
    pq = i_quant_mat;
#ifdef ALTIVEC_DST
    vec_dst(pb, 0x01080010, 0); /* vec_dst complete size of blk */
    vec_dst(pq, 0x01080010, 1); /* vec_dst complete size of i_quant_mat */
#endif

    zero = vec_splat_s16(0);
    sum = vec_splat_s32(0);


#define PREPARE_FIRST_ITERATION /* {{{ */                                    \
    vA = vec_ld(0, pb);                                                      \
    qA = vec_ld(0, pq);                                                      \
    pb += 8; vB = vec_ld(0, pb);                                             \
    pq += 8; qB = vec_ld(0, pq);                                             \
    /* }}} */

#define PERFORM_ITERATION(iteration) /* i = iteration {{{ */                 \
    nA = vec_subs(zero, vA);                                                 \
    nB = vec_subs(zero, vB);                                                 \
    absA = vec_max(nA, vA);                                                  \
    absB = vec_max(nB, vB);                                                  \
    sum = vec_msum(absA, vs16(qA), sum);                                     \
    sum = vec_msum(absB, vs16(qB), sum);                                     \
    /* }}} */

#define PREPARE_ITERATION /* {{{ */                                          \
    pb += 8;  vA = vec_ld(0, pb);                                            \
    pq += 8;  qA = vec_ld(0, pq);                                            \
    pb += 8;  vB = vec_ld(0, pb);                                            \
    pq += 8;  qB = vec_ld(0, pq);                                            \
    /* }}} */


#if 1
    PREPARE_FIRST_ITERATION; PERFORM_ITERATION(0);
    PREPARE_ITERATION;       PERFORM_ITERATION(1);
    PREPARE_ITERATION;       PERFORM_ITERATION(2);
    PREPARE_ITERATION;       PERFORM_ITERATION(3);
#else
    int i;
    for (i = 0; i < 64/8/2; i++) {
	vA = vec_ld(0, pb); pb += 8;
	vB = vec_ld(0, pb); pb += 8;
	qA = vec_ld(0, pq); pq += 8;
	qB = vec_ld(0, pq); pq += 8;

	nA = vec_subs(zero, vA);
	nB = vec_subs(zero, vB);
	absA = vec_max(nA, vA);
	absB = vec_max(nB, vB);

	sum = vec_msum(absA, vs16(qA), sum);
	sum = vec_msum(absB, vs16(qB), sum);
    }
#endif

#ifdef ALTIVEC_DST
    vec_dssall();
#endif

    vo.v = vec_sums(vs32(sum), vs32(zero));

    AMBER_STOP;

    return vo.s.sum;
}


int quant_weight_coeff_intra_altivec(QUANT_WEIGHT_COEFF_INTRA_PDECL)
{
    return quant_weight_coeff_sum_altivec(wsp->i_intra_q_mat, blk);
}


int quant_weight_coeff_inter_altivec(QUANT_WEIGHT_COEFF_INTER_PDECL)
{
    return quant_weight_coeff_sum_altivec(wsp->i_inter_q_mat, blk);
}


#if ALTIVEC_TEST_FUNCTION(quant_weight_coeff_intra)
ALTIVEC_TEST(quant_weight_coeff_intra, int, (QUANT_WEIGHT_COEFF_INTRA_PDECL),
    QUANT_WEIGHT_COEFF_INTRA_PFMT, QUANT_WEIGHT_COEFF_INTRA_ARGS);
#endif

#if ALTIVEC_TEST_FUNCTION(quant_weight_coeff_inter)
ALTIVEC_TEST(quant_weight_coeff_inter, int, (QUANT_WEIGHT_COEFF_INTER_PDECL),
    QUANT_WEIGHT_COEFF_INTER_PFMT, QUANT_WEIGHT_COEFF_INTER_ARGS);
#endif

/* vim:set foldmethod=marker foldlevel=0: */
