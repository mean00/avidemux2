/* subsample_image.c, this file is part of the
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

#if defined(ALTIVEC_VERIFY) && ALTIVEC_TEST_FUNCTION(subsample_image)
#include <stdlib.h>
#endif

#include "vectorize.h"
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


#define SUBSAMPLE_IMAGE_PDECL /* {{{ */                                      \
	uint8_t *image, int rowstride,                                       \
	uint8_t *sub22_image,                                                \
	uint8_t *sub44_image                                                 \
	/* }}} */
#define SUBSAMPLE_IMAGE_ARGS image, rowstride, sub22_image, sub44_image
#define SUBSAMPLE_IMAGE_PFMT /* {{{ */                                       \
	"image=0x%X, rowstride=%d, sub22_image=0x%X, sub44_image=0x%X"       \
	/* }}} */

void subsample_image_altivec(SUBSAMPLE_IMAGE_PDECL)
{
    int i, ii, j, stride1, stride2, stride3, stride4, halfstride;
    unsigned char *pB, *pB2, *pB4;
    vector unsigned char l0, l1, l2, l3;
    vector unsigned short s0, s1, s2, s3;
    vector unsigned short s22_0, s22_1, s22_2, s22_3;
    vector unsigned short s44, s44_0, s44_1;
    vector unsigned short zero, two;
#ifdef ALTIVEC_DST
    DataStreamControl dsc;
#endif

#ifdef ALTIVEC_VERIFY /* {{{ */
    if (NOT_VECTOR_ALIGNED(image))
	mjpeg_error_exit1("subsample_image: %s %% %d != 0, (%d)",
	    "image", 16, image);

    if ((rowstride & 63) != 0)
	mjpeg_error_exit1("subsample_image: %s %% %d != 0, (%d)",
	    "rowstride", 64, rowstride);

    if (NOT_VECTOR_ALIGNED(sub22_image))
	mjpeg_error_exit1("subsample_image: %s %% %d != 0, (%d)",
	    "sub22_image", 16, sub22_image);

    if (NOT_VECTOR_ALIGNED(sub44_image))
	mjpeg_error_exit1("subsample_image: %s %% %d != 0, (%d)",
	    "sub44_image", 16, sub44_image);
#endif /* }}} */

    AMBER_START;

    pB = image;

#ifdef ALTIVEC_DST
    dsc.control = DATA_STREAM_CONTROL(6,4,0);
    dsc.block.stride = rowstride;

    vec_dst(pB, dsc.control, 0);
#endif

    pB2 = sub22_image;
    pB4 = sub44_image;

    j = ((unsigned long)(pB2 - pB) / rowstride) >> 2; /* height/4 */

    stride1 = rowstride;
    stride2 = stride1 + stride1;
    stride3 = stride2 + stride1;
    stride4 = stride2 + stride2;
    halfstride = stride1 >> 1; /* /2 */

    ii = rowstride >> 6; /* rowstride/16/4 */

    zero = vec_splat_u16(0);
    two = vec_splat_u16(2);

    do {
	i = ii;
	do {
	    l0 = vec_ld(0, pB);
	    l1 = vec_ld(stride1, pB);
	    l2 = vec_ld(stride2, pB);
	    l3 = vec_ld(stride3, pB);
	    pB += 16;
#ifdef ALTIVEC_DST
	    vec_dst(pB + (16 * 3), dsc.control, 0);
#endif

	    /* l0 = 0x[00,01,02,03,04,05,06,07,08,09,0A,0B,0C,0D,0E,0F] */
	    /* l1 = 0x[10,11,12,13,14,15,16,17,18,19,1A,1B,1C,1D,1E,1F] */
	    /* l2 = 0x[20,21,22,23,24,25,26,27,28,29,2A,2B,2C,2D,2E,2F] */
	    /* l3 = 0x[30,31,32,33,34,35,36,37,38,39,3A,3B,3C,3D,3E,3F] */

	    /* s0 = 0x[00,01,      02,03,      04,05,      06,07,     ] */
	    /*        [      10,11,      12,13,      14,15,      16,17] */
	    s0 = vu16(vec_mergeh(vu16(l0), vu16(l1)));
	    /* s0 = 0x[00+01+10+11,02+03+12+13,04+05+14+15,06+07+16+17] */
	    s0 = vu16(vec_sum4s(vu8(s0), vu32(zero)));

	    /* s1 = 0x[08,09,      0A,0B,      0C,0D,      0E,0F,     ] */
	    /*        [      18,19,      1A,1B,      1C,1D,      1E,1F] */
	    s1 = vu16(vec_mergel(vu16(l0), vu16(l1)));
	    /* s1 = 0x[08+09+18+19,0A+0B+1A+1B,0C+0D+1C+1D,0E+0F+1E+1F] */
	    s1 = vu16(vec_sum4s(vu8(s1), vu32(zero)));

	    /* s2 = 0x[20,21,      22,23,      24,25,      26,27,     ] */
	    /*        [      30,31,      32,33,      34,35,      36,37] */
	    s2 = vu16(vec_mergeh(vu16(l2), vu16(l3)));
	    /* s2 = 0x[20+21+30+31,22+23+32+33,24+25+34+35,26+27+36+37] */
	    s2 = vu16(vec_sum4s(vu8(s2), vu32(zero)));

	    /* s3 = 0x[28,29,      2A,2B,      2C,2D,      2E,2F,     ] */
	    /*        [      38,39,      3A,3B,      3C,3D,      3E,3F] */
	    s3 = vu16(vec_mergel(vu16(l2), vu16(l3)));
	    /* s3 = 0x[28+29+38+39,2A+2B+3A+3B,2C+2D+3C+3D,2E+2F+3E+3F] */
	    s3 = vu16(vec_sum4s(vu8(s3), vu32(zero)));

	    /* start loading next block */
	    l0 = vec_ld(0, pB);
	    l1 = vec_ld(stride1, pB);
	    l2 = vec_ld(stride2, pB);
	    l3 = vec_ld(stride3, pB);
	    pB += 16;

	    /* s0 = 0x[00+01+10+11, 02+03+12+13, 04+05+14+15, 06+07+16+17] */
	    /* s1 = 0x[08+09+18+19, 0A+0B+1A+1B, 0C+0D+1C+1D, 0E+0F+1E+1F] */
	    /* s2 = 0x[20+21+30+31, 22+23+32+33, 24+25+34+35, 26+27+36+37] */
	    /* s3 = 0x[28+29+38+39, 2A+2B+3A+3B, 2C+2D+3C+3D, 2E+2F+3E+3F] */

	    /* s22_0 = 0x[   00,   02,   04,   06,   08,   0A,   0C,   0E] */
	    s22_0 = vec_packsu(vu32(s0), vu32(s1));
	    /* s22_1 = 0x[   20,   22,   24,   26,   28,   2A,   2C,   2E] */
	    s22_1 = vec_packsu(vu32(s2), vu32(s3));

	    /* (pB[i]+pB[i+1]+pN[i]+pN[i+1]) + 2 */
	    s22_0 = vec_add(s22_0, two);
	    /* (pNN[i]+pNN[i+1]+pNNN[i]+pNNN[i+1]) + 2 */
	    s22_1 = vec_add(s22_1, two);

	    /* (pB[i]+pB[i+1]+pN[i]+pN[i+1]+2) >> 2 */
	    s22_0 = vec_sra(s22_0, two);
	    /* (pNN[i]+pNN[i+1]+pNNN[i]+pNNN[i+1]+2) >> 2 */
	    s22_1 = vec_sra(s22_1, two);

	    /* s22_0 = 0x[   00,   02,   04,   06,   08,   0A,   0C,   0E] */
	    /* s22_1 = 0x[   20,   22,   24,   26,   28,   2A,   2C,   2E] */
	    /* s44_0 = 0x[00+20,02+22,04+24,06+26,08+28,0A+2A,0C+2C,0E+2E] */
	    s44_0 = vec_add(s22_0, s22_1);

	    /* s44_0 = 0x[00+20+02+22, 04+24+06+26, 08+28+0A+2A, 0C+2C+0E+2E] */
	    s44_0 = vu16(vec_sum4s(vs16(s44_0), vs32(zero)));

	    /* - - - - - - - - - - - - - - - - - - - */
	    s0 = vu16(vec_mergeh(vu16(l0), vu16(l1)));
	    s0 = vu16(vec_sum4s(vu8(s0), vu32(zero)));
	    s1 = vu16(vec_mergel(vu16(l0), vu16(l1)));
	    s1 = vu16(vec_sum4s(vu8(s1), vu32(zero)));
	    s2 = vu16(vec_mergeh(vu16(l2), vu16(l3)));
	    s2 = vu16(vec_sum4s(vu8(s2), vu32(zero)));
	    s3 = vu16(vec_mergel(vu16(l2), vu16(l3)));
	    s3 = vu16(vec_sum4s(vu8(s3), vu32(zero)));

	    /* start loading next l[0-3] */
	    l0 = vec_ld(0, pB);
	    l1 = vec_ld(stride1, pB);
	    l2 = vec_ld(stride2, pB);
	    l3 = vec_ld(stride3, pB);
	    pB += 16;


	    s22_2 = vec_packsu(vu32(s0), vu32(s1));
	    s22_3 = vec_packsu(vu32(s2), vu32(s3));

	    s22_2 = vec_add(s22_2, two);
	    s22_3 = vec_add(s22_3, two);

	    s22_2 = vec_sra(s22_2, two);
	    s22_3 = vec_sra(s22_3, two);


	    s44_1 = vec_add(s22_2, s22_3);
	    s44_1 = vu16(vec_sum4s(vs16(s44_1), vs32(zero)));

	    /* store s22 block */
	    s22_0 = vu16(vec_packsu(s22_0, s22_2));
	    s22_1 = vu16(vec_packsu(s22_1, s22_3));
	    vec_st(vu8(s22_0), 0, pB2);
	    vec_st(vu8(s22_1), halfstride, pB2);
	    pB2 += 16;

	    /* - - - - - - - - - - - - - - - - - - - */
	    s0 = vu16(vec_mergeh(vu16(l0), vu16(l1)));
	    s0 = vu16(vec_sum4s(vu8(s0), vu32(zero)));
	    s1 = vu16(vec_mergel(vu16(l0), vu16(l1)));
	    s1 = vu16(vec_sum4s(vu8(s1), vu32(zero)));
	    s2 = vu16(vec_mergeh(vu16(l2), vu16(l3)));
	    s2 = vu16(vec_sum4s(vu8(s2), vu32(zero)));
	    s3 = vu16(vec_mergel(vu16(l2), vu16(l3)));
	    s3 = vu16(vec_sum4s(vu8(s3), vu32(zero)));

	    /* starting loading next l[0-3] */
	    l0 = vec_ld(0, pB);
	    l1 = vec_ld(stride1, pB);
	    l2 = vec_ld(stride2, pB);
	    l3 = vec_ld(stride3, pB);
	    pB += 16;


	    s22_0 = vec_packsu(vu32(s0), vu32(s1));
	    s22_1 = vec_packsu(vu32(s2), vu32(s3));

	    s22_0 = vec_add(s22_0, two);
	    s22_1 = vec_add(s22_1, two);

	    s22_0 = vec_sra(s22_0, two);
	    s22_1 = vec_sra(s22_1, two);


	    s44 = vec_packsu(vu32(s44_0), vu32(s44_1));
	    s44 = vec_add(s44, two);
	    s44 = vec_sra(s44, two);

	    s44_0 = vec_add(s22_0, s22_1);
	    s44_0 = vu16(vec_sum4s(vs16(s44_0), vs32(zero)));

	    /* - - - - - - - - - - - - - - - - - - - */
	    s0 = vu16(vec_mergeh(vu16(l0), vu16(l1)));
	    s0 = vu16(vec_sum4s(vu8(s0), vu32(zero)));
	    s1 = vu16(vec_mergel(vu16(l0), vu16(l1)));
	    s1 = vu16(vec_sum4s(vu8(s1), vu32(zero)));
	    s2 = vu16(vec_mergeh(vu16(l2), vu16(l3)));
	    s2 = vu16(vec_sum4s(vu8(s2), vu32(zero)));
	    s3 = vu16(vec_mergel(vu16(l2), vu16(l3)));
	    s3 = vu16(vec_sum4s(vu8(s3), vu32(zero)));

	    s22_2 = vec_packsu(vu32(s0), vu32(s1));
	    s22_3 = vec_packsu(vu32(s2), vu32(s3));

	    s22_2 = vec_add(s22_2, two);
	    s22_3 = vec_add(s22_3, two);

	    s22_2 = vec_sra(s22_2, two);
	    s22_3 = vec_sra(s22_3, two);

	    s44_1 = vec_add(s22_2, s22_3);
	    s44_1 = vu16(vec_sum4s(vs16(s44_1), vs32(zero)));

	    /* store s22 block */
	    s22_0 = vu16(vec_packsu(s22_0, s22_2));
	    s22_1 = vu16(vec_packsu(s22_1, s22_3));
	    vec_st(vu8(s22_0), 0, pB2);
	    vec_st(vu8(s22_1), halfstride, pB2);
	    pB2 += 16;

	    /* pack all four s44 chunks */
	    s44_0 = vec_packsu(vu32(s44_0), vu32(s44_1));
	    s44_0 = vec_add(s44_0, two);
	    s44_0 = vec_sra(s44_0, two);
	    s44 = vu16(vec_packsu(s44, s44_0));

	    vec_st(vu8(s44), 0, pB4);
	    pB4 += 16;

	} while (--i);

	pB += stride3;
	pB2 += halfstride;

    } while (--j);

#ifdef ALTIVEC_DST
    vec_dss(0);
#endif

    AMBER_STOP;
}

#if ALTIVEC_TEST_FUNCTION(subsample_image) /* {{{ */
#  ifdef ALTIVEC_VERIFY

static void imgcpy(uint8_t *d, uint8_t *s, int width, int height, int stride)
{
    int i, j;

    for (j = 0; j < height; j++) {
	for (i = 0; i < width; i++)
	    d[i] = s[i];
	d += width;
	s += stride;
    }
}

static unsigned long checksum(uint8_t *p, int width, int height, int stride)
{
    int i, j;
    unsigned long checksum;

    for (checksum = j = 0; j < height; j++) {
	for (i = 0; i < width; i++)
	    checksum += p[i];
	p += stride;
    }

    return checksum;
}

static void imgcmp(const char *ss, uint8_t *a, uint8_t *b,
    int width, int height, int stride)
{
    int i, j;

    for (j = 0; j < height; j++) {
	for (i = 0; i < width; i++)
	    if (a[i] != b[i])
		mjpeg_debug("subsample_image: %s[%d][%d] %d != %d",
		    ss, j, i, a[i], b[i]);

	a += width;
	b += stride;
    }
}

void subsample_image_altivec_verify(SUBSAMPLE_IMAGE_PDECL)
{
    int width, height;
    unsigned long checksum44_1, checksum44_2;
    unsigned long checksum22_1, checksum22_2;
    unsigned char *cpy22, *cpy44;

    width = rowstride;
    height = (unsigned long)(sub22_image - image) / rowstride;

    cpy22 = (unsigned char*)malloc((width/2) * (height/2));
    cpy44 = (unsigned char*)malloc((width/4) * (height/4));
    if (cpy22 == NULL || cpy44 == NULL)
	mjpeg_error_exit1("subsample_image: malloc failed");

    subsample_image_altivec(SUBSAMPLE_IMAGE_ARGS);
    checksum22_1 = checksum(sub22_image, width/2, height/2, rowstride/2);
    checksum44_1 = checksum(sub44_image, width/4, height/4, rowstride/4);

    /* copy data for imgcmp */
    imgcpy(cpy22, sub22_image, width/2, height/2, rowstride/2);
    imgcpy(cpy44, sub44_image, width/4, height/4, rowstride/4);

    ALTIVEC_TEST_WITH(subsample_image)(SUBSAMPLE_IMAGE_ARGS);
    checksum22_2 = checksum(sub22_image, width/2, height/2, rowstride/2);
    checksum44_2 = checksum(sub44_image, width/4, height/4, rowstride/4);

    if (checksum22_1 != checksum22_2 || checksum44_1 != checksum44_2) {
	mjpeg_debug("subsample_image(" SUBSAMPLE_IMAGE_PFMT ")",
	    SUBSAMPLE_IMAGE_ARGS);
	if (checksum22_1 != checksum22_2)
	    mjpeg_debug("subsample_image: %s checksums differ %d != %d",
		"2*2", checksum22_1, checksum22_2);
	if (checksum44_1 != checksum44_2)
	    mjpeg_debug("subsample_image: %s checksums differ %d != %d",
		"4*4", checksum44_1, checksum44_2);

	imgcmp("2*2", cpy22, sub22_image, width/2, height/2, rowstride/2);
	imgcmp("4*4", cpy44, sub44_image, width/4, height/4, rowstride/4);
    }

    free(cpy22);
    free(cpy44);
}

#  else

#undef BENCHMARK_ITERATIONS
#define BENCHMARK_ITERATIONS 1000
#undef BENCHMARK_FREQUENCY  1
#define BENCHMARK_FREQUENCY  1

ALTIVEC_TEST(subsample_image, void, (SUBSAMPLE_IMAGE_PDECL),
    SUBSAMPLE_IMAGE_PFMT, SUBSAMPLE_IMAGE_ARGS);
#  endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
