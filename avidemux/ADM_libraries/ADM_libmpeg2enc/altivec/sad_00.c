/* sad_00.c, this file is part of the
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
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
#include "amber.h"

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif


/*
 * sum absolute difference 
 * 
 * Input requirements:
 *   b) blk2 is always vector aligned
 *   c) rowstride is a multiple of 16
 *   d) h is either 8 or 16
 *
 */

#define SAD_00_PDECL                                                         \
  uint8_t *blk1,                                                             \
  uint8_t *blk2,                                                             \
  int rowstride,                                                             \
  int h,                                                                     \
  int dlim                                                                   \

#define SAD_00_ARGS blk1, blk2, rowstride, h, dlim

int sad_00_altivec(SAD_00_PDECL)
{
    int i;
    unsigned char *pblk1, *pblk2;
    vector unsigned char blk1A, blk2A, blk1B, blk2B;
    vector unsigned char blk1A0, blk1B0;
    vector unsigned char blk1A1, blk1B1;
    vector unsigned char minA, minB;
    vector unsigned char maxA, maxB;
    vector unsigned char difA, difB;
    vector unsigned int sum;
    vector signed int zero;
    vector unsigned char perm;
    union {
	vector signed int v;
	struct {
	    signed int pad[3];
	    signed int sum;
	} s;
    } vo;

#ifdef ALTIVEC_VERIFY
  if (NOT_VECTOR_ALIGNED(blk2))
    mjpeg_error_exit1("sad_00: blk2 %% 16 != 0, (%d)\n", blk2);

  if (NOT_VECTOR_ALIGNED(rowstride))
    mjpeg_error_exit1("sad_00: rowstride %% 16 != 0, (%d)\n", rowstride);

  if (h != 8 && h != 16)
    mjpeg_error_exit1("sad_00: h != [8|16], (%d)\n", h);
#endif

    AMBER_START;

    pblk1 = blk1;
    pblk2 = blk2;
    i = (h >> 1) - 1;

    zero = vec_splat_s32(0);
    sum = vec_splat_u32(0);

    if (VECTOR_ALIGNED(pblk1)) {

	blk1A = vec_ld(0, pblk1);
	pblk1 += rowstride;
	blk1B = vec_ld(0, pblk1);

	blk2A = vec_ld(0, pblk2);
	pblk2 += rowstride;
	blk2B = vec_ld(0, pblk2);

	do {
	    maxA = vec_max(blk1A, blk2A);
	    minA = vec_min(blk1A, blk2A);

	    pblk1 += rowstride;
	    blk1A = vec_ld(0, pblk1);
	    pblk2 += rowstride;
	    blk2A = vec_ld(0, pblk2);

	    difA = vec_sub(maxA, minA);
	    sum = vec_sum4s(difA, sum);

	    maxB = vec_max(blk1B, blk2B);
	    minB = vec_min(blk1B, blk2B);

	    pblk1 += rowstride;
	    blk1B = vec_ld(0, pblk1);
	    pblk2 += rowstride;
	    blk2B = vec_ld(0, pblk2);

	    difB = vec_sub(maxB, minB);
	    sum = vec_sum4s(difB, sum);
	} while (--i);

    } else {

	perm = vec_lvsl(0, pblk1);

	blk1A0 = vec_ld(0, pblk1);
	blk1A1 = vec_ld(16, pblk1);
	pblk1 += rowstride;
	blk1B0 = vec_ld(0, pblk1);
	blk1B1 = vec_ld(16, pblk1);

	blk2A = vec_ld(0, pblk2);
	pblk2 += rowstride;
	blk2B = vec_ld(0, pblk2);

	do {
	    blk1A = vec_perm(blk1A0, blk1A1, perm);

	    pblk1 += rowstride;
	    blk1A0 = vec_ld(0, pblk1);
	    blk1A1 = vec_ld(16, pblk1);

	    maxA = vec_max(blk1A, blk2A);
	    minA = vec_min(blk1A, blk2A);

	    pblk2 += rowstride;
	    blk2A = vec_ld(0, pblk2);

	    difA = vec_sub(maxA, minA);
	    sum = vec_sum4s(difA, sum);


	    blk1B = vec_perm(blk1B0, blk1B1, perm);
	    pblk1 += rowstride;
	    blk1B0 = vec_ld(0, pblk1);
	    blk1B1 = vec_ld(16, pblk1);

	    maxB = vec_max(blk1B, blk2B);
	    minB = vec_min(blk1B, blk2B);

	    pblk2 += rowstride;
	    blk2B = vec_ld(0, pblk2);

	    difB = vec_sub(maxB, minB);
	    sum = vec_sum4s(difB, sum);
	} while (--i);

	blk1A = vec_perm(blk1A0, blk1A1, perm);
	blk1B = vec_perm(blk1B0, blk1B1, perm);
    }

    maxA = vec_max(blk1A, blk2A);
    minA = vec_min(blk1A, blk2A);
    difA = vec_sub(maxA, minA);
    sum = vec_sum4s(difA, sum);

    maxB = vec_max(blk1B, blk2B);
    minB = vec_min(blk1B, blk2B);
    difB = vec_sub(maxB, minB);
    sum = vec_sum4s(difB, sum);

    vo.v = vec_sums(vs32(sum), zero);

    AMBER_STOP;

    return vo.s.sum;
}


#if ALTIVEC_TEST_FUNCTION(sad_00) /* {{{ */
#ifdef ALTIVEC_VERIFY
int sad_00_altivec_verify(SAD_00_PDECL)
{
  int value, test;

  value = ALTIVEC_TEST_WITH(sad_00)(SAD_00_ARGS);
  test  = sad_00_altivec(SAD_00_ARGS);

  /* special case, test <= dlim */
  if (test != value && test <= dlim)
    mjpeg_debug("%d != %d"
      "=sad_00(blk1=0x%X, blk2=0x%X, rowstride=%d, h=%d, dlim=%d)",
      test, value, SAD_00_ARGS);

  return value;
}
#else
ALTIVEC_TEST(sad_00, int, (SAD_00_PDECL),
  "blk1=0x%X, blk2=0%X, rowstride=%d, h=%d, dlim=%d",
  SAD_00_ARGS);
#endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
