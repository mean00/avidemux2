/*
 *  TwoLAME: an optimized MPEG Audio Layer Two encoder
 *
 *  Copyright (C) 2001-2004 Michael Cheng
 *  Copyright (C) 2004-2005 The TwoLAME Project
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 */

#ifndef	_BITBUFFER_H_
#define	_BITBUFFER_H_


/* bit stream structure */
typedef struct bit_stream_struc {
    unsigned char *buf;       /* bit stream buffer */
    int buf_size;         /* size of buffer (in number of bytes) */
    long totbit;          /* bit counter of bit stream */
    int buf_byte_idx;     /* pointer to top byte in buffer */
    int buf_bit_idx;      /* pointer to top bit of top byte in buffer */
    int eob;          /* end of buffer index */
    int eobs;         /* end of bit stream flag */
} bit_stream;


void buffer_init( unsigned char *buffer, int buffer_size, bit_stream* bs );
//void buffer_deinit( bit_stream ** bs );

void buffer_put1bit (bit_stream *, int);
inline void buffer_putbits (bit_stream *, unsigned int, int);
unsigned long buffer_sstell (bit_stream *);

#endif

