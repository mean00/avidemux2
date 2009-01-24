/* iquant_intra.c, this file is part of the
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

#if defined(ALTIVEC_VERIFY) && ALTIVEC_TEST_FUNCTION(iquant_intra_m1)
#  include <stdlib.h>
#  include <string.h>
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


#define IQUANT_INTRA_PDECL struct QuantizerWorkSpace *wsp, int16_t *src, int16_t *dst, int dc_prec, int mquant
#define IQUANT_INTRA_ARGS wsp, src, dst, dc_prec, mquant
#define IQUANT_INTRA_PFMT "wsp=0x%X, src=0x%X, dst=0x%X, dc_prec=%d, mquant=%d"


void iquant_intra_m1_altivec(IQUANT_INTRA_PDECL)
{
    int i;
    vector signed short vsrc;
    uint16_t *qmat;
    vector unsigned short vqmat;
    vector unsigned short vmquant;
    vector bool short eqzero, ltzero;
    vector signed short val, t0;
    vector signed short zero, one;
    vector unsigned int four;
    vector signed short min, max;
    int offset, offset2;
    int16_t dst0;
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
    if (NOT_VECTOR_ALIGNED(wsp->intra_q_mat))
	mjpeg_error_exit1("iquant_intra_m1: wsp->intra_q_mat %% 16 != 0, (%d)",
	    wsp->intra_q_mat);

    if (NOT_VECTOR_ALIGNED(src))
	mjpeg_error_exit1("iquant_intra_m1: src %% 16 != 0, (%d)", src);

    if (NOT_VECTOR_ALIGNED(dst))
	mjpeg_error_exit1("iquant_intra_m1: dst %% 16 != 0, (%d)", dst);

    for (i = 0; i < 64; i++)
	if (src[i] < -256 || src[i] > 255)
	    mjpeg_error_exit1("iquant_intra_m2: -256 > src[%i] > 255, (%d)",
		i, src[i]);
#endif /* }}} */

    AMBER_START;

    dst0 = src[0] << (3 - dc_prec);

    qmat = (uint16_t*)wsp->intra_q_mat;

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(64/8,1,0);
    vec_dst(src, dsc.control, 0);
    vec_dst(qmat, dsc.control, 1);
#endif

    /* vmquant = (vector unsigned short)(mquant); */
    vu.mquant = (unsigned short)mquant;
    vmquant = vec_splat(vu.vu16, 0);

    zero = vec_splat_s16(0);
    one = vec_splat_s16(1);
    four = vec_splat_u32(4);
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
	/* intra_q[i] * mquant */
	vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

	/* save sign */
	ltzero = vec_cmplt(vsrc, zero);
	eqzero = vec_cmpeq(vsrc, zero);

	/* val = abs(src) */
	t0 = vec_sub(zero, vsrc);
	val = vec_max(t0, vsrc);

	/* val = (src * quant) >> 4 */
	t0 = vs16(vec_mule(val, vs16(vqmat)));
	val = vs16(vec_mulo(val, vs16(vqmat)));
	t0 = vs16(vec_sra(vs32(t0), four));
	t0 = vs16(vec_pack(vs32(t0), vs32(t0)));
	val = vs16(vec_sra(vs32(val), four));
	val = vs16(vec_pack(vs32(val), vs32(val)));
	val = vec_mergeh(vs16(t0), vs16(val));

	offset2 = offset;
	offset += 8*sizeof(int16_t);
	vsrc = vec_ld(offset, (signed short*)src);
	vqmat = vec_ld(offset, (unsigned short*)qmat);

	/* val = val - 1&~(val|val==0) */
	t0 = vec_or(val, eqzero);
	t0 = vec_andc(one, t0);
	val = vec_sub(val, t0);

	/* restore sign */
	t0 = vec_sub(zero, val);
	val = vec_sel(val, t0, ltzero);

	/* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
	val = vec_min(val, max);
	val = vec_max(val, min);

	vec_st(val, offset2, dst);
    } while (--i);
    /* intra_q[i] * mquant */
    vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

    /* save sign */
    ltzero = vec_cmplt(vsrc, zero);
    eqzero = vec_cmpeq(vsrc, zero);

    /* val = abs(src) */
    t0 = vec_sub(zero, vsrc);
    val = vec_max(t0, vsrc);

    /* val = (src * quant) >> 4 */
    t0 = vs16(vec_mule(val, vs16(vqmat)));
    val = vs16(vec_mulo(val, vs16(vqmat)));
    t0 = vs16(vec_sra(vs32(t0), four));
    t0 = vs16(vec_pack(vs32(t0), vs32(t0)));
    val = vs16(vec_sra(vs32(val), four));
    val = vs16(vec_pack(vs32(val), vs32(val)));
    val = vec_mergeh(vs16(t0), vs16(val));

    /* val = val - 1&~(val|val==0) */
    t0 = vec_or(val, eqzero);
    t0 = vec_andc(one, t0);
    val = vec_sub(val, t0);

    /* restore sign */
    t0 = vec_sub(zero, val);
    val = vec_sel(val, t0, ltzero);

    /* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
    val = vec_min(val, max);
    val = vec_max(val, min);

    vec_st(val, offset, dst);
#else
    /* {{{ */
    i = (64/8);
    do {
	vsrc = vec_ld(offset, (signed short*)src);
	vqmat = vec_ld(offset, (unsigned short*)qmat);

	/* intra_q[i] * mquant */
	vu16(vqmat) = vec_mulo(vu8(vqmat), vu8(vmquant));

	/* save sign */
	ltzero = vec_cmplt(vsrc, zero);
	eqzero = vec_cmpeq(vsrc, zero);

	/* val = abs(src) */
	t0 = vec_sub(zero, vsrc);
	val = vec_max(t0, vsrc);

	/* val = (src * quant) >> 4 */
	vs32(t0) = vec_mule(val, vs16(vqmat));
	vs32(val) = vec_mulo(val, vs16(vqmat));
	vs32(t0) = vec_sra(vs32(t0), four);
	vs16(t0) = vec_pack(vs32(t0), vs32(t0));
	vs32(val) = vec_sra(vs32(val), four);
	vs16(val) = vec_pack(vs32(val), vs32(val));
	val = vec_mergeh(vs16(t0), vs16(val));

	/* val = val - 1&~(val|val==0) */
	t0 = vec_or(val, eqzero);
	t0 = vec_andc(one, t0);
	val = vec_sub(val, t0);

	/* restore sign */
	t0 = vec_sub(zero, val);
	val = vec_sel(val, t0, ltzero);

	/* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
	val = vec_min(val, max);
	val = vec_max(val, min);

	vec_st(val, offset, dst);

	offset += 8*sizeof(int16_t);
    } while (--i);
    /* }}} */
#endif

    dst[0] = dst0;

    AMBER_STOP;
}



void iquant_intra_m2_altivec(IQUANT_INTRA_PDECL)
{
    int i;
    vector signed short vsrc;
    uint16_t *qmat;
    vector unsigned short vqmat;
    vector unsigned short vmquant;
    vector bool short ltzero;
    vector signed short val, t0;
    vector signed short zero;
    vector unsigned int four;
    vector signed short min, max;
    vector signed int vsum;
    int sum;
    int offset, offset2;
    int16_t dst0;
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
    if (NOT_VECTOR_ALIGNED(wsp->intra_q_mat))
	mjpeg_error_exit1("iquant_intra_m2: wsp->intra_q_mat %% 16 != 0, (%d)",
	    wsp->intra_q_mat);

    if (NOT_VECTOR_ALIGNED(src))
	mjpeg_error_exit1("iquant_intra_m2: src %% 16 != 0, (%d)", src);

    if (NOT_VECTOR_ALIGNED(dst))
	mjpeg_error_exit1("iquant_intra_m2: dst %% 16 != 0, (%d)", dst);

    for (i = 0; i < 64; i++)
	if (src[i] < -256 || src[i] > 255)
	    mjpeg_error_exit1("iquant_intra_m2: -256 > src[%i] > 255, (%d)",
		i, src[i]);
#endif /* }}} */

    AMBER_START;

    dst0 = src[0] << (3 - dc_prec);

    qmat = (uint16_t*)wsp->intra_q_mat;

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(64/8,1,0);
    vec_dst(src, dsc.control, 0);
    vec_dst(qmat, dsc.control, 1);
#endif

    /* vmquant = (vector unsigned short)(mquant); */
    vu.mquant = (unsigned short)mquant;
    vmquant = vec_splat(vu.vu16, 0);

    vsum = vec_splat_s32(0);
    zero = vec_splat_s16(0);
    four = vec_splat_u32(4);
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
	/* intra_q[i] * mquant */
	vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

	/* save sign */
	ltzero = vec_cmplt(vsrc, zero);

	/* val = abs(src) */
	t0 = vec_sub(zero, vsrc);
	val = vec_max(t0, vsrc);

	/* val = (src * quant) >> 4 */
	t0 = vs16(vec_mule(val, vs16(vqmat)));
	val = vs16(vec_mulo(val, vs16(vqmat)));
	t0 = vs16(vec_sra(vs32(t0), four));
	t0 = vs16(vec_pack(vs32(t0), vs32(t0)));
	val = vs16(vec_sra(vs32(val), four));
	val = vs16(vec_pack(vs32(val), vs32(val)));
	val = vec_mergeh(vs16(t0), vs16(val));

	offset2 = offset;
	offset += 8*sizeof(int16_t);
	vsrc = vec_ld(offset, (signed short*)src);
	vqmat = vec_ld(offset, (unsigned short*)qmat);

	/* restore sign */
	t0 = vec_sub(zero, val);
	val = vec_sel(val, t0, ltzero);

	/* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
	val = vec_min(val, max);
	val = vec_max(val, min);

	vsum = vec_sum4s(val, vsum);

	vec_st(val, offset2, dst);
    } while (--i);
    /* intra_q[i] * mquant */
    vqmat = vu16(vec_mulo(vu8(vqmat), vu8(vmquant)));

    /* save sign */
    ltzero = vec_cmplt(vsrc, zero);

    /* val = abs(src) */
    t0 = vec_sub(zero, vsrc);
    val = vec_max(t0, vsrc);

    /* val = (src * quant) >> 4 */
    t0 = vs16(vec_mule(val, vs16(vqmat)));
    val = vs16(vec_mulo(val, vs16(vqmat)));
    t0 = vs16(vec_sra(vs32(t0), four));
    t0 = vs16(vec_pack(vs32(t0), vs32(t0)));
    val = vs16(vec_sra(vs32(val), four));
    val = vs16(vec_pack(vs32(val), vs32(val)));
    val = vec_mergeh(vs16(t0), vs16(val));

    /* restore sign */
    t0 = vec_sub(zero, val);
    val = vec_sel(val, t0, ltzero);

    /* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
    val = vec_min(val, max);
    val = vec_max(val, min);

    vsum = vec_sum4s(val, vsum);

    vec_st(val, offset, dst);
#else
    /* {{{ */
    i = (64/8);
    do {
	vsrc = vec_ld(offset, (signed short*)src);
	vqmat = vec_ld(offset, (unsigned short*)qmat);

	/* intra_q[i] * mquant */
	vu16(vqmat) = vec_mulo(vu8(vqmat), vu8(vmquant));

	/* save sign */
	ltzero = vec_cmplt(vsrc, zero);

	/* val = abs(src) */
	t0 = vec_sub(zero, vsrc);
	val = vec_max(t0, vsrc);

	/* val = (src * quant) >> 4 */
	vs32(t0) = vec_mule(val, vs16(vqmat));
	vs32(val) = vec_mulo(val, vs16(vqmat));
	vs32(t0) = vec_sra(vs32(t0), four);
	vs16(t0) = vec_pack(vs32(t0), vs32(t0));
	vs32(val) = vec_sra(vs32(val), four);
	vs16(val) = vec_pack(vs32(val), vs32(val));
	val = vec_mergeh(vs16(t0), vs16(val));

	/* restore sign */
	t0 = vec_sub(zero, val);
	val = vec_sel(val, t0, ltzero);

	/* val = (val > 2047) ? ((val < -2048) ? -2048 : val); */
	val = vec_min(val, max);
	val = vec_max(val, min);

	vsum = vec_sum4s(val, vsum);

	vec_st(val, offset, dst);

	offset += 8*sizeof(int16_t);
    } while (--i);
    /* }}} */
#endif

    vu.vs32 = vec_sums(vsum, vs32(zero));
    sum = vu.s.sum;
    sum -= dst[0];
    sum += dst0;
    dst[0] = dst0;

    /* mismatch control */
#if 1
    dst[63] ^= !(sum & 1);
#else
    if ((sum & 1) == 0)
	dst[63] ^= 1;
#endif

    AMBER_STOP;
}


/* iquant_intra_altivec_verify {{{ */
#if ALTIVEC_TEST_FUNCTION(iquant_intra_m1) || \
    ALTIVEC_TEST_FUNCTION(iquant_intra_m2)

static void iquant_intra_altivec_verify(IQUANT_INTRA_PDECL,
    void (*test)(IQUANT_INTRA_PDECL),
    void (*verify)(IQUANT_INTRA_PDECL))
{
    int i;
    unsigned long checksum1, checksum2;
    int16_t srccpy[64], dstcpy[64];
    uint16_t *qmat;

    qmat = (uint16_t*) wsp->intra_q_mat;

    /* in case src == dst */
    memcpy(srccpy, src, 64*sizeof(int16_t));

    (*test)(IQUANT_INTRA_ARGS);
    for (checksum1 = i = 0; i < 64; i++)
	checksum1 += dst[i];

    memcpy(dstcpy, dst, 64*sizeof(int16_t));

    memcpy(src, srccpy, 64*sizeof(int16_t));

    (*verify)(IQUANT_INTRA_ARGS);
    for (checksum2 = i = 0; i < 64; i++)
	checksum2 += dst[i];

    if (checksum1 != checksum2) {
	mjpeg_debug("iquant_intra(" IQUANT_INTRA_PFMT ")",
	    IQUANT_INTRA_ARGS);
	mjpeg_debug("iquant_intra: checksums differ %d != %d",
		    checksum1, checksum2);
    }

    for (i = 0; i < 64; i++) {
	if (dstcpy[i] != dst[i]) {
	    mjpeg_debug("iquant_intra: src[%d]=%d, qmat=%d, "
			"dst[%d]=%d != %d", i, srccpy[i], qmat[i]*mquant,
			i, dstcpy[i], dst[i]);
	}
    }
}
#endif /* }}} */

#if ALTIVEC_TEST_FUNCTION(iquant_intra_m1) /* {{{ */
#  ifdef ALTIVEC_VERIFY
void iquant_intra_m1_altivec_verify(IQUANT_INTRA_PDECL)
{
    iquant_intra_altivec_verify(IQUANT_INTRA_ARGS,
	iquant_intra_m1_altivec, ALTIVEC_TEST_WITH(iquant_intra_m1));
}
#  else
ALTIVEC_TEST(iquant_intra_m1, void, (IQUANT_INTRA_PDECL),
    IQUANT_INTRA_PFMT, IQUANT_INTRA_ARGS);
#  endif
#endif /* }}} */

#if ALTIVEC_TEST_FUNCTION(iquant_intra_m2) /* {{{ */
#  ifdef ALTIVEC_VERIFY
void iquant_intra_m2_altivec_verify(IQUANT_INTRA_PDECL)
{
    iquant_intra_altivec_verify(IQUANT_INTRA_ARGS,
	iquant_intra_m2_altivec, ALTIVEC_TEST_WITH(iquant_intra_m2));
}
#  else
ALTIVEC_TEST(iquant_intra_m2, void, (IQUANT_INTRA_PDECL),
    IQUANT_INTRA_PFMT, IQUANT_INTRA_ARGS);
#  endif
#endif /* }}} */

/* vim:set foldmethod=marker foldlevel=0: */
