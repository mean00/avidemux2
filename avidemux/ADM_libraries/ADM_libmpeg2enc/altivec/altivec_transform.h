/* altivec_transform.h, this file is part of the
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

#include <stdint.h>

#include "altivec_conf.h"


#define ALTIVEC_TEST_TRANSFORM /* {{{ */                                     \
    ( ( defined(ALTIVEC_BENCHMARK) || defined(ALTIVEC_VERIFY) ) &&           \
      ( ALTIVEC_TEST_FUNCTION(fdct) ||                                       \
        ALTIVEC_TEST_FUNCTION(idct) ||                                       \
        ALTIVEC_TEST_FUNCTION(add_pred) ||                                   \
        ALTIVEC_TEST_FUNCTION(sub_pred) ||                                   \
        ALTIVEC_TEST_FUNCTION(field_dct_best) ) )                            \
    /* }}} */


#ifdef __cplusplus
extern "C" {
#endif

ALTIVEC_FUNCTION(fdct, void, (int16_t *blk));

ALTIVEC_FUNCTION(idct, void, (int16_t *blk));

ALTIVEC_FUNCTION(sub_pred, void,
    (uint8_t *pred, uint8_t *cur, int lx, int16_t *blk));

ALTIVEC_FUNCTION(add_pred, void,
    (uint8_t *pred, uint8_t *cur, int lx, int16_t *blk));

ALTIVEC_FUNCTION(field_dct_best, int,
    (uint8_t *cur_lum_mb, uint8_t *pred_lum_mb, int stride));

#ifdef __cplusplus
}
#endif
