/*
 * DSP utils
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard.
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file dsputil.h
 * DSP utils.
 * note, many functions in here may use MMX which trashes the FPU state, it is
 * absolutely necessary to call emms_c() between dsp & float/double code
 */

#ifndef DSPUTIL_CPU_H
#define DSPUTIL_CPU_H

int mm_support(void);

#define MM_MMX    0x0001 /* standard MMX */
#define MM_3DNOW  0x0004 /* AMD 3DNOW */
#define MM_MMXEXT 0x0002 /* SSE integer functions or AMD MMX ext */
#define MM_SSE    0x0008 /* SSE functions */
#define MM_SSE2   0x0010 /* PIV SSE2 functions */
#define MM_3DNOWEXT  0x0020 /* AMD 3DNowExt */
#define MM_SSE3   0x0040 /* Prescott SSE3 functions */
#define MM_SSSE3  0x0080 /* Conroe SSSE3 functions */

extern int mm_flags;

#endif
