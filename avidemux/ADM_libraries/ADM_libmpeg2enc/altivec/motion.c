/* motion.c, this file is part of the
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

#include "../mjpeg_logging.h"


void enable_altivec_motion()
{
#if ALTIVEC_TEST_MOTION
#  if defined(ALTIVEC_BENCHMARK)
    mjpeg_info("SETTING AltiVec BENCHMARK for MOTION!");
#  elif defined(ALTIVEC_VERIFY)
    mjpeg_info("SETTING AltiVec VERIFY for MOTION!");
#  endif
#else
    mjpeg_info("SETTING AltiVec for MOTION!");
#endif

    /* psad_sub22 = sad_sub22; */
    /* psad_sub44 = sad_sub44; */

    /* disable sad_00_altivec when benchmarking find_best_one_pel */
#if !(defined(ALTIVEC_BENCHMARK) && ALTIVEC_TEST_FUNCTION(find_best_one_pel))
#if ALTIVEC_TEST_FUNCTION(sad_00)
    psad_00 = ALTIVEC_TEST_SUFFIX(sad_00);
#else
    psad_00 = ALTIVEC_SUFFIX(sad_00);
#endif
#endif

#if ALTIVEC_TEST_FUNCTION(sad_01)
    psad_01 = ALTIVEC_TEST_SUFFIX(sad_01);
#else
    psad_01 = ALTIVEC_SUFFIX(sad_01);
#endif

#if ALTIVEC_TEST_FUNCTION(sad_10)
    psad_10 = ALTIVEC_TEST_SUFFIX(sad_10);
#else
    psad_10 = ALTIVEC_SUFFIX(sad_10);
#endif

#if ALTIVEC_TEST_FUNCTION(sad_11)
    psad_11 = ALTIVEC_TEST_SUFFIX(sad_11);
#else
    psad_11 = ALTIVEC_SUFFIX(sad_11);
#endif

#if ALTIVEC_TEST_FUNCTION(bsad)
    pbsad = ALTIVEC_TEST_SUFFIX(bsad);
#else
    pbsad = ALTIVEC_SUFFIX(bsad);
#endif

#if ALTIVEC_TEST_FUNCTION(sumsq)
    psumsq = ALTIVEC_TEST_SUFFIX(sumsq);
#else
    psumsq = ALTIVEC_SUFFIX(sumsq);
#endif

#if ALTIVEC_TEST_FUNCTION(sumsq_sub22)
    psumsq_sub22 = ALTIVEC_TEST_SUFFIX(sumsq_sub22);
#else
    psumsq_sub22 = ALTIVEC_SUFFIX(sumsq_sub22);
#endif

#if ALTIVEC_TEST_FUNCTION(bsumsq)
    pbsumsq = ALTIVEC_TEST_SUFFIX(bsumsq);
#else
    pbsumsq = ALTIVEC_SUFFIX(bsumsq);
#endif


#if ALTIVEC_TEST_FUNCTION(bsumsq_sub22)
    pbsumsq_sub22 = ALTIVEC_TEST_SUFFIX(bsumsq_sub22);
#else
    pbsumsq_sub22 = ALTIVEC_SUFFIX(bsumsq_sub22);
#endif

#if ALTIVEC_TEST_FUNCTION(find_best_one_pel)
    pfind_best_one_pel = ALTIVEC_TEST_SUFFIX(find_best_one_pel);
#else
    pfind_best_one_pel = ALTIVEC_SUFFIX(find_best_one_pel);
#endif

#if ALTIVEC_TEST_FUNCTION(build_sub22_mests)
    pbuild_sub22_mests = ALTIVEC_TEST_SUFFIX(build_sub22_mests);
#else
    pbuild_sub22_mests = ALTIVEC_SUFFIX(build_sub22_mests);
#endif

#if ALTIVEC_TEST_FUNCTION(build_sub44_mests)
    pbuild_sub44_mests = ALTIVEC_TEST_SUFFIX(build_sub44_mests);
#else
    pbuild_sub44_mests = ALTIVEC_SUFFIX(build_sub44_mests);
#endif

#if ALTIVEC_TEST_FUNCTION(variance)
    pvariance = ALTIVEC_TEST_SUFFIX(variance);
#else
    pvariance = ALTIVEC_SUFFIX(variance);
#endif

#if ALTIVEC_TEST_FUNCTION(subsample_image)
    psubsample_image = ALTIVEC_TEST_SUFFIX(subsample_image);
#else
    psubsample_image = ALTIVEC_SUFFIX(subsample_image);
#endif
}
