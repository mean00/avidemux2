/* vectorize.h, this file is part of the
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

/*
 * Useful AltiVec macros
 *
 */

#define VECTOR_ALIGNED(p) (((unsigned long)(p) & 15) == 0)
#define NOT_VECTOR_ALIGNED(p) (((unsigned long)(p) & 15) != 0)

#define VECTOR_OFFSET(p) ((((unsigned long)(p))+15) & (~15))

#define vs8(v)  ((vector signed char)(v))
#define vs16(v) ((vector signed short)(v))
#define vs32(v) ((vector signed int)(v))
#define vu8(v)  ((vector unsigned char)(v))
#define vu16(v) ((vector unsigned short)(v))
#define vu32(v) ((vector unsigned int)(v))
#define vb8(v)  ((vector bool char)(v))
#define vb16(v) ((vector bool short)(v))
#define vb32(v) ((vector bool int)(v))
#define vpx(v)  ((vector pixel)(v))
#define vfp(v)  ((vector float)(v))


typedef union {
    unsigned int control;
    struct {
        unsigned char size;  /* 0 to 31, (size & 0x1F) */
        unsigned char count; /* 0 to 255 */
        signed short stride; /* -32768 to +32768 */
    } block;
} DataStreamControl;

#define DATA_STREAM_CONTROL(size,count,stride) \
    ((((size)&0x1F)<<24)|(((count)&0xFF)<<16)|((stride)&0xFFFF))

#ifdef HAVE_ALTIVEC_H
/* GNU GCC3 style vector constants */
#define VCONST(v...) {v}
#else
/* Motorola style vector constants */
#define VCONST(v...) (v)
#endif
