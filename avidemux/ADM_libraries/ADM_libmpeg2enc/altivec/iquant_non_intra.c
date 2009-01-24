/* iquant_non_intra_m1.c, this file is part of the
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

#if defined(ALTIVEC_VERIFY) && ALTIVEC_TEST_FUNCTION(iquant_non_intra_m1)
#  include <stdlib.h>
#endif

#include "vectorize.h"
#include "mjpeg_logging.h"
#include "quantize_precomp.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


#define IQUANT_NON_INTRA_PDECL \
    struct QuantizerWorkSpace *wsp, int16_t *src, int16_t *dst, int mquant
#define IQUANT_NON_INTRA_ARGS wsp, src, dst, mquant
#define IQUANT_NON_INTRA_PFMT "wsp=0x%X, src=0x%X, dst=0x%X, mquant=%d"


void iquant_non_intra_m1_altivec(IQUANT_NON_INTRA_PDECL)
{
    int i;
    vector signed short vsrc;
    uint16_t *qmat;
    vector unsigned short vqmat;
    vector unsigned short vmquant;
    vector signed short val, t0;
    vector bool short ltzero, eqzero;
    vector signed short zero, one;
    vector unsigned int five;
    vector signed short min, max;
    union {
	vector unsigned short v;
	unsigned short mquant[8];
    } vu;
#ifdef ALTIVEC_DST
    DataStreamControl dsc;
#endif

#ifdef ALTIVEC_VERIFY /* {{{ */
    if (NOT_VECTOR_ALIGNED(wsp->inter_q_mat))
	mjpeg_error_exit1("iquant_non_intra_m1: wsp->inter_q_mat %% 16 != 0, "
	    "(%d)", wsp->inter_q_mat);

    if (NOT_VECTOR_ALIGNED(src))
	mjpeg_error_exit1("iquant_non_intra_m1: src %% 16 != 0, (%d)", src);

    if (NOT_VECTOR_ALIGNED(dst))
	mjpeg_error_exit1("iquant_non_intra_m1: dst %% 16 != 0, (%d)", dst);

    for (i = 0; i < 64; i++)
	if (src[i] < -256 || src[i] > 255)
	    mjpeg_error_exit1("iquant_non_intra_m1: -256 > src[%i] > 255, (%d)",
		i, src[i]);
#endif /* }}} */

    AMBER_START;

    qmat = (uint16_t*)wsp->inter_q_mat;

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(64/8,1,0);
    vec_dst(src, dsc.control, 0);
    vec_dst(qmat, dsc.control, 1);
#endif

    vu.mquant[0] = (unsigned short)mquant;
    vmquant = vec_splat(vu.v, 0);

    vsrc = vec_ld(0, (signed short*)src);
    vqmat = vec_ld(0, qmat);
    zero = vec_splat_s16(0);
    one = vec_splat_s16(1);
    five = vec_splat_u32(5);
    /* max = (2047); min = (-2048); {{{ */
    max = vs16(vec_splat_u8(0x7));
    t0 = vec_splat_s16(-1); /* 0xffff */
    max = vs16(vec_mergeh(vu8(max), vu8(t0))); /* 0x07ff == 2047 */
    min = vec_sub(t0, max);
    /* }}} */

    i = (64/8) - 1;
    do {
	/* inter_q[i] * mquant */
	vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

	ltzero = vec_cmplt(vsrc, zero);
	eqzero = vec_cmpeq(vsrc, zero);

	/* val = abs(src) */
	t0 = vec_sub(zero, vsrc);
	val = vec_max(t0, vsrc);

	/* val = val + val + 1 */
	val = vec_add(val, val);
	val = vec_add(val, one);

	/* val = (val * quant) >> 5 */
	t0 = vs16(vec_mule(vu16(val), vqmat));
	t0 = vs16(vec_sra(vu32(t0), five));
	t0 = vs16(vec_pack(vu32(t0), vu32(t0)));
	val = vs16(vec_mulo(vu16(val), vqmat));
	val = vs16(vec_sra(vu32(val), five));
	val = vs16(vec_pack(vu32(val), vu32(val)));
	val = vs16(vec_mergeh(vu16(t0), vu16(val)));

	src += 8;
	vsrc = vec_ld(0, (signed short*)src);
	qmat += 8;
	vqmat = vec_ld(0, (unsigned short*)qmat);

	/* val = val - 1&~(val|val==0) */
	t0 = vec_or(val, eqzero);
	t0 = vec_andc(one, t0);
	val = vec_sub(val, t0);

	/* val = samesign(src, val) */
	t0 = vec_sub(zero, val);
	val = vec_sel(val, t0, ltzero);

	/* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
	val = vec_min(val, max);
	val = vec_max(val, min);

	/* if (src[i] == 0) dst[i] = 0 */
	val = vec_andc(val, eqzero);

	vec_st(val, 0, dst);
	dst += 8;
    } while (--i);

    /* inter_q[i] * mquant */
    vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

    ltzero = vec_cmplt(vsrc, zero);
    eqzero = vec_cmpeq(vsrc, zero);

    /* val = abs(src) */
    t0 = vec_sub(zero, vsrc);
    val = vec_max(t0, vsrc);

    /* val = val + val + 1 */
    val = vec_add(val, val);
    val = vec_add(val, one);

    /* val = (val * quant) >> 5 */
    t0 = vs16(vec_mule(vu16(val), vqmat));
    t0 = vs16(vec_sra(vu32(t0), five));
    t0 = vs16(vec_pack(vu32(t0), vu32(t0)));
    val = vs16(vec_mulo(vu16(val), vqmat));
    val = vs16(vec_sra(vu32(val), five));
    val = vs16(vec_pack(vu32(val), vu32(val)));
    val = vs16(vec_mergeh(vu16(t0), vu16(val)));

    /* val = val - 1&~(val|val==0) */
    t0 = vec_or(val, eqzero);
    t0 = vec_andc(one, t0);
    val = vec_sub(val, t0);

    /* val = samesign(src, val) */
    t0 = vec_sub(zero, val);
    val = vec_sel(val, t0, ltzero);

    /* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
    val = vec_min(val, max);
    val = vec_max(val, min);

    /* if (src[i] == 0) dst[i] = 0 */
    val = vec_andc(val, eqzero);

    vec_st(val, 0, dst);

    AMBER_STOP;
}



void iquant_non_intra_m2_altivec(IQUANT_NON_INTRA_PDECL)
{
    int i;
    vector signed short vsrc;
    uint16_t *qmat;
    vector unsigned short vqmat;
    vector unsigned short vmquant;
    vector signed short val, t0;
    vector bool short ltzero, eqzero;
    vector signed short zero, one;
    vector unsigned int five;
    vector signed short min, max;
    vector signed int sum;
    int offset, offset2;
    union {
	vector unsigned short vu16;
	unsigned short mquant;
	vector signed int vs32;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vu;
#ifdef ALTIVEC_DST
    DataStreamControl dsc;
#endif

#ifdef ALTIVEC_VERIFY /* {{{ */
    if (NOT_VECTOR_ALIGNED(wsp->inter_q_mat))
	mjpeg_error_exit1("iquant_non_intra_m2: wsp->inter_q_mat %% 16 != 0, "
	    "(%d)", wsp->inter_q_mat);

    if (NOT_VECTOR_ALIGNED(src))
	mjpeg_error_exit1("iquant_non_intra_m2: src %% 16 != 0, (%d)", src);

    if (NOT_VECTOR_ALIGNED(dst))
	mjpeg_error_exit1("iquant_non_intra_m2: dst %% 16 != 0, (%d)", dst);

    for (i = 0; i < 64; i++)
	if (src[i] < -256 || src[i] > 255)
	    mjpeg_error_exit1("iquant_non_intra_m2: -256 > src[%i] > 255, (%d)",
		i, src[i]);
#endif /* }}} */

    AMBER_START;

    qmat = (uint16_t*)wsp->inter_q_mat;

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(64/8,1,0);
    vec_dst(src, dsc.control, 0);
    vec_dst(qmat, dsc.control, 1);
#endif

    /* vmquant = (vector unsigned short)(mquant); */
    vu.mquant = (unsigned short)mquant;
    vmquant = vec_splat(vu.vu16, 0);

    sum = vec_splat_s32(0);
    zero = vec_splat_s16(0);
    one = vec_splat_s16(1);
    five = vec_splat_u32(5);
    /* max = (2047); min = (-2048); {{{ */
    max = vs16(vec_splat_u8(0x7));
    t0 = vec_splat_s16(-1); /* 0xffff */
    max = vs16(vec_mergeh(vu8(max), vu8(t0))); /* 0x07ff == 2047 */
    min = vec_sub(t0, max);
    /* }}} */
    offset = 0;

#if 1
    vsrc = vec_ld(offset, (signed short*)src);
    vqmat = vec_ld(offset, (unsigned short*)qmat);
    i = (64/8) - 1;
    do {
	/* inter_q[i] * mquant */
	vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

	ltzero = vec_cmplt(vsrc, zero);
	eqzero = vec_cmpeq(vsrc, zero);

	/* val = abs(src) */
	t0 = vec_sub(zero, vsrc);
	val = vec_max(t0, vsrc);

	/* val = val + val + 1 */
	val = vec_add(val, val);
	val = vec_add(val, one);

	/* val = (val * quant) >> 5 */
	t0 = vs16(vec_mule(vu16(val), vqmat));
	t0 = vs16(vec_sra(vu32(t0), five));
	t0 = vs16(vec_pack(vu32(t0), vu32(t0)));
	val = vs16(vec_mulo(vu16(val), vqmat));
	val = vs16(vec_sra(vu32(val), five));
	val = vs16(vec_pack(vu32(val), vu32(val)));
	val = vs16(vec_mergeh(vu16(t0), vu16(val)));

	offset2 = offset;
	offset += 8*sizeof(int16_t);
	vsrc = vec_ld(offset, (signed short*)src);
	vqmat = vec_ld(offset, (unsigned short*)qmat);

	/* val = samesign(src, val) */
	t0 = vec_sub(zero, val);
	val = vec_sel(val, t0, ltzero);

	/* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
	val = vec_min(val, max);
	val = vec_max(val, min);

	/* if (src[i] == 0) dst[i] = 0 */
	val = vec_andc(val, eqzero);

	sum = vec_sum4s(val, sum);

	vec_st(val, offset2, dst);
    } while (--i);

    /* inter_q[i] * mquant */
    vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

    ltzero = vec_cmplt(vsrc, zero);
    eqzero = vec_cmpeq(vsrc, zero);

    /* val = abs(src) */
    t0 = vec_sub(zero, vsrc);
    val = vec_max(t0, vsrc);

    /* val = val + val + 1 */
    val = vec_add(val, val);
    val = vec_add(val, one);

    /* val = (val * quant) >> 5 */
    t0 = vs16(vec_mule(vu16(val), vqmat));
    t0 = vs16(vec_sra(vu32(t0), five));
    t0 = vs16(vec_pack(vu32(t0), vu32(t0)));
    val = vs16(vec_mulo(vu16(val), vqmat));
    val = vs16(vec_sra(vu32(val), five));
    val = vs16(vec_pack(vu32(val), vu32(val)));
    val = vs16(vec_mergeh(vu16(t0), vu16(val)));

    /* val = samesign(src, val) */
    t0 = vec_sub(zero, val);
    val = vec_sel(val, t0, ltzero);

    /* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
    val = vec_min(val, max);
    val = vec_max(val, min);

    /* if (src[i] == 0) dst[i] = 0 */
    val = vec_andc(val, eqzero);

    sum = vec_sum4s(val, sum);

    vec_st(val, offset, dst);

#else
    /* {{{ */
    i = (64/8);
    do {
	vsrc = vec_ld(offset, (signed short*)src);
	vqmat = vec_ld(offset, (unsigned short*)qmat);

	/* inter_q[i] * mquant */
	vu16(vqmat) = vec_mulo(vu8(vqmat), vu8(vmquant));

	ltzero = vec_cmplt(vsrc, zero);
	eqzero = vec_cmpeq(vsrc, zero);

	/* val = abs(src) */
	t0 = vec_sub(zero, vsrc);
	val = vec_max(t0, vsrc);

	/* val = val + val + 1 */
	val = vec_add(val, val);
	val = vec_add(val, one);

	/* val = (val * quant) >> 5 */
	vu32(t0) = vec_mule(vu16(val), vqmat);
	vu32(t0) = vec_sra(vu32(t0), five);
	vu16(t0) = vec_pack(vu32(t0), vu32(t0));
	vu32(val) = vec_mulo(vu16(val), vqmat);
	vu32(val) = vec_sra(vu32(val), five);
	vu16(val) = vec_pack(vu32(val), vu32(val));
	vu16(val) = vec_mergeh(vu16(t0), vu16(val));

	/* val = samesign(src, val) */
	t0 = vec_sub(zero, val);
	val = vec_sel(val, t0, ltzero);

	/* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
	val = vec_min(val, max);
	val = vec_max(val, min);

	/* if (src[i] == 0) dst[i] = 0 */
	val = vec_andc(val, eqzero);

	sum = vec_sum4s(val, sum);

	vec_st(val, offset, dst);

	offset += 8*sizeof(int16_t);
    } while (--i);
    /* }}} */
#endif

    /* mismatch control */
    vu.vs32 = vec_sums(sum, vs32(zero));
#if 1
    dst[63] ^= !(vu.s.sum & 1);
#else
    if ((vu.s.sum & 1) == 0)
	dst[63] ^= 1;
#endif

    AMBER_STOP;
}

/* iquant_non_intra_altivec_verify {{{ */
#if ALTIVEC_TEST_FUNCTION(iquant_non_intra_m1) || \
    ALTIVEC_TEST_FUNCTION(iquant_non_intra_m2)

static void iquant_non_intra_altivec_verify(IQUANT_NON_INTRA_PDECL,
    void (*test)(IQUANT_NON_INTRA_PDECL),
    void (*verify)(IQUANT_NON_INTRA_PDECL))
{
    int i;
    unsigned long checksum1, checksum2;
    int16_t srccpy[64], dstcpy[64];
    uint16_t *qmat;

    qmat = (uint16_t*)wsp->inter_q_mat;

    /* in case src == dst */
    memcpy(srccpy, src, 64*sizeof(int16_t));

    (*test)(IQUANT_NON_INTRA_ARGS);
    for (checksum1 = i = 0; i < 64; i++)
	checksum1 += dst[i];

    memcpy(dstcpy, dst, 64*sizeof(int16_t));

    memcpy(src, srccpy, 64*sizeof(int16_t));

    (*verify)(IQUANT_NON_INTRA_ARGS);
    for (checksum2 = i = 0; i < 64; i++)
	checksum2 += dst[i];

    if (checksum1 != checksum2) {
	mjpeg_debug("iquant_non_intra(" IQUANT_NON_INTRA_PFMT ")",
	    IQUANT_NON_INTRA_ARGS);
	mjpeg_debug("iquant_non_intra: checksums differ %d != %d",
		    checksum1, checksum2);
    }

    for (i = 0; i < 64; i++) {
	if (dstcpy[i] != dst[i]) {
	    mjpeg_debug("iquant_non_intra: src[%d]=%d, qmat=%d, "
			"dst %d != %d", i, srccpy[i], qmat[i]*mquant,
			dstcpy[i], dst[i]);
	}
    }
}
#endif /* }}} */

#if ALTIVEC_TEST_FUNCTION(iquant_non_intra_m1) /* {{{ */
#  ifdef ALTIVEC_VERIFY
void iquant_non_intra_m1_altivec_verify(IQUANT_NON_INTRA_PDECL)
{
    iquant_non_intra_altivec_verify(IQUANT_NON_INTRA_ARGS,
	iquant_non_intra_m1_altivec, ALTIVEC_TEST_WITH(iquant_non_intra_m1));
}
#  else
ALTIVEC_TEST(iquant_non_intra_m1, void, (IQUANT_NON_INTRA_PDECL),
    IQUANT_NON_INTRA_PFMT, IQUANT_NON_INTRA_ARGS);
#  endif
#endif /* }}} */

#if ALTIVEC_TEST_FUNCTION(iquant_non_intra_m2) /* {{{ */
#  ifdef ALTIVEC_VERIFY
void iquant_non_intra_m2_altivec_verify(IQUANT_NON_INTRA_PDECL)
{
    iquant_non_intra_altivec_verify(IQUANT_NON_INTRA_ARGS,
	iquant_non_intra_m2_altivec, ALTIVEC_TEST_WITH(iquant_non_intra_m2));
}
#  else
ALTIVEC_TEST(iquant_non_intra_m2, void, (IQUANT_NON_INTRA_PDECL),
    IQUANT_NON_INTRA_PFMT, IQUANT_NON_INTRA_ARGS);
#  endif
#endif /* }}} */

/* vim:set foldmethod=marker foldlevel=0: */
