/* sub_mean_reduction.c, this file is part of the
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

#if defined(ALTIVEC_VERIFY) && ALTIVEC_TEST_FUNCTION(sub_mean_reduction)
#include <stdlib.h> /* malloc */
#include <string.h> /* memcpy */
#endif

#include "vectorize.h"
#include <math.h>
#include "../mjpeg_logging.h"

/* #define AMBER_ENABLE */
/* #define AMBER_MAX_TRACES 100 */
#include "amber.h"


#define SUB_MEAN_REDUCTION_PDECL /* {{{ */                                   \
    me_result_set *matchset,                                                 \
    int rtimes,                                                              \
    int *minweight_res                                                       \
    /* }}} */
#define SUB_MEAN_REDUCTION_ARGS  matchset, rtimes, minweight_res

void sub_mean_reduction_altivec(SUB_MEAN_REDUCTION_PDECL)
{
    me_result_s *matches, *match, *tail;
    me_result_s m0, m1;
    int w0, w1;
    int i;
    int len = matchset->len;
    int weight_sum;
    int mean_weight;

    AMBER_START;

    if (len <= 1) {
	*minweight_res = (len == 0 ? 100000 : matchset->mests[0].weight);
    } else {
	matches = matchset->mests;

	weight_sum = 0;
	match = matches - 1;

	if (len > 1) {
	    i = len >> 1;
	    do {
		w0 = (*(++match)).weight;
		weight_sum += w0;
		w1 = (*(++match)).weight;
		weight_sum += w1;
	    } while (--i);
	}
	if (len & 1)
	    weight_sum += (*(++match)).weight;

	mean_weight = weight_sum / len;

	while (rtimes--) {
	    weight_sum = 0;

	    match = matches - 1;
	    tail = match;

	    if (len > 1) {
		i = len >> 1;
		do {
		    m0 = *(++match);
		    w0 = m0.weight;
		    m1 = *(++match);
		    w1 = m1.weight;
		    if (w0 <= mean_weight) {
			*(++tail) = m0;
			weight_sum += w0;
		    }
		    if (w1 <= mean_weight) {
			*(++tail) = m1;
			weight_sum += w1;
		    }
		} while (--i);
	    }
	    if (len & 1) {
		m0 = *(++match);
		w0 = m0.weight;
		if (w0 <= mean_weight) {
		    *(++tail) = m0;
		    weight_sum += w0;
		}
	    }

	    tail++; /* adjust for -1 */
	    len = tail - matches;
	    mean_weight = weight_sum / len;
	}

	matchset->len = len;
	*minweight_res = mean_weight;
    }

    AMBER_STOP;
}

#if ALTIVEC_TEST_FUNCTION(sub_mean_reduction) /* {{{ */

#define SUB_MEAN_REDUCTION_PFMT	/* {{{ */                                    \
    "matchset=0x%X, rtimes=%d, minweight_res=0x%X"                           \
    /* }}} */

#  ifdef ALTIVEC_VERIFY

void sub_mean_reduction_altivec_verify(SUB_MEAN_REDUCTION_PDECL)
{
  int i, len, len1, len2, mwr, mwr1, mwr2;
  me_result_s *mestscpy;
  unsigned long checksum1, checksum2;

  /* save len */
  len = matchset->len;

  mestscpy = (me_result_s*)malloc(len*sizeof(me_result_s));
  if (mestscpy == NULL)
    mjpeg_error_exit1("sub_mean_reduction: malloc failed");

  /* save mests */
  memcpy(mestscpy, matchset->mests, len*sizeof(me_result_s));

  /* save minweight_res */
  mwr = *minweight_res;

  sub_mean_reduction_altivec(SUB_MEAN_REDUCTION_ARGS);
  len1 = matchset->len;
  mwr1 = *minweight_res;
  for (checksum1 = i = 0; i < len1; i++) {
    checksum1 += matchset->mests[i].weight;
    checksum1 += abs(matchset->mests[i].x);
    checksum1 += abs(matchset->mests[i].y);
  }

  /* restore len */
  matchset->len = len;

  /* restore mests */
  memcpy(matchset->mests, mestscpy, len*sizeof(me_result_s));

  /* restore minweight_res */
  *minweight_res = mwr;

  ALTIVEC_TEST_WITH(sub_mean_reduction)(SUB_MEAN_REDUCTION_ARGS);
  len2 = matchset->len;
  mwr2 = *minweight_res;
  for (checksum2 = i = 0; i < len2; i++) {
    checksum2 += matchset->mests[i].weight;
    checksum2 += abs(matchset->mests[i].x);
    checksum2 += abs(matchset->mests[i].y);
  }

  if (len1 != len2 || checksum1 != checksum2 || mwr1 != mwr2) {
    mjpeg_debug("sub_mean_reduction(" SUB_MEAN_REDUCTION_PFMT ")",
	SUB_MEAN_REDUCTION_ARGS);
    mjpeg_debug("sub_mean_reduction: matchset->len=%d", len);
    mjpeg_debug("sub_mean_reduction: minweight_res %d, %d", mwr1, mwr2);
    mjpeg_debug("sub_mean_reduction: checksums %d[%d], %d[%d]",
	checksum1, len1, checksum2, len2);
  }

  free(mestscpy);
}

#  else

#undef BENCHMARK_PROLOG
#define BENCHMARK_PROLOG /* {{{ */                                           \
    mjpeg_info("sub_mean_reduction: in matchset->len=%d",                    \
	 matchset->len);                                                     \
    /* }}} */

#undef BENCHMARK_EPILOG
#define BENCHMARK_EPILOG /* {{{ */                                           \
    mjpeg_info("sub_mean_reduction: out matchset->len=%d, *minweight_res=%d",\
	 matchset->len, *minweight_res);                                     \
    /* }}} */

ALTIVEC_TEST(sub_mean_reduction, void, (SUB_MEAN_REDUCTION_PDECL),
  SUB_MEAN_REDUCTION_PFMT, SUB_MEAN_REDUCTION_ARGS);
#  endif
#endif /* }}} */
/* vim:set foldmethod=marker foldlevel=0: */
