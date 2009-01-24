/* altivec_predict.h, this file is part of the
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


#define ALTIVEC_TEST_PREDICT /* {{{ */                                       \
    ( ( defined(ALTIVEC_BENCHMARK) || defined(ALTIVEC_VERIFY) ) &&           \
    ALTIVEC_TEST_FUNCTION(pred_comp) )                                       \
    /* }}} */


#ifdef __cplusplus
extern "C" {
#endif

ALTIVEC_FUNCTION(pred_comp, void,
	(uint8_t *src, uint8_t *dst, int lx,
	 int w, int h, int x, int y, int dx, int dy, int addflag));

#ifdef __cplusplus
}
#endif
