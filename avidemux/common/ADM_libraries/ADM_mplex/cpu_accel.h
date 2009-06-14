/*
 * cpu_accel.h
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file was part of mpeg2dec, a free MPEG-2 video stream decoder.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef _CPU_ACCEL_HH
#define _CPU_ACCEL_HH

#include <stdlib.h>
#include "mjpeg_types.h"


// x86 accelerations
#define ACCEL_X86_MMX	0x80000000
#define ACCEL_X86_3DNOW	0x40000000
#define ACCEL_X86_MMXEXT 0x20000000
#define ACCEL_X86_SSE   0x10000000

#ifdef __cplusplus
extern "C" {
#endif

	int32_t cpu_accel (void);
	void *bufalloc( size_t size );
#if	!defined(HAVE_POSIX_MEMALIGN)
	int posix_memalign(void **, size_t, size_t);
#endif
#if	!defined(HAVE_MEMALIGN)
	void *memalign(size_t, size_t);
#endif
#if	!defined(HAVE_FMAX)
	double fmax(double, double);
#endif

extern const char *disable_simd_flags[];
extern int disable_simd(char *);

#ifdef __cplusplus
}
#endif


#endif
