/* detect.c, this file is part of the
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

#ifdef HAVE_ALTIVEC_H
/* include last to ensure AltiVec type semantics, especially for bool. */
#include <altivec.h>
#endif

/*
 * Functions containing AltiVec code have additional VRSAVE instructions
 * at the beginning and end of the function. To avoid executing any AltiVec
 * instructions before our signal handler is setup the AltiVec code is
 * encapsulated in a function.
 *
 * The automatic VRSAVE instructions can be disabled with
 * #pragma altivec_vrsave off
 *
 * Storing vector registers to memory shouldn't alter the state of the vectors
 * or the vector unit. The following function contains a single vector stvx
 * instruction.
 */
#pragma altivec_vrsave off
int altivec_copy_v0()
{
    register vector unsigned int v0 asm ("v0");
    union {
	vector unsigned int align16;
	unsigned int v0[4];
    } copy;

    vec_st(v0, 0, copy.v0);
    return copy.v0[0];
}

/*
 * detect_altivec() moved to ../cpu_accel.c
 */
