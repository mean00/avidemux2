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


#include <stdio.h>
#include <stdlib.h>

#include "twolame.h"
#include "common.h"
#include "bitbuffer.h"
#include "mem.h"


/*create bit buffer*/
bit_stream* buffer_init( unsigned char *buffer, int buffer_size )
{
	bit_stream* bs = (bit_stream *)twolame_malloc(sizeof(bit_stream),"bit_stream");
	
	bs->buf = buffer;
	bs->buf_size = buffer_size;
	bs->buf_byte_idx = 0;
	bs->buf_bit_idx = 8;
	bs->totbit = 0;
	bs->eob = FALSE;
	bs->eobs = FALSE;
	
	return bs;
}

/* Dellocate bit buffer */
void buffer_deinit( bit_stream ** bs )
{
	twolame_free( (void**)bs );
}


/*write 1 bit from the bit stream */
void buffer_put1bit (bit_stream * bs, int bit)
{
	bs->totbit++;
	
	bs->buf[bs->buf_byte_idx] |= (bit & 0x1) << (bs->buf_bit_idx - 1);
	bs->buf_bit_idx--;
	if (!bs->buf_bit_idx) {
		bs->buf_bit_idx = 8;
		bs->buf_byte_idx++;
		if (bs->buf_byte_idx >= bs->buf_size) {
			//empty_buffer (bs, minimum);
			fprintf(stdout,"buffer_put1bit: error. bit_stream buffer needs to be bigger\n");
			exit(99);
		}
		bs->buf[bs->buf_byte_idx] = 0;
	}
}

/*write N bits into the bit stream */
inline void buffer_putbits (bit_stream * bs, unsigned int val, int N)
{
	static const int putmask[9] = { 0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff };
	register int j = N;
	register int k, tmp;
	
	bs->totbit += N;
	while (j > 0) {
		k = MIN (j, bs->buf_bit_idx);
		tmp = val >> (j - k);
		bs->buf[bs->buf_byte_idx] |= (tmp & putmask[k]) << (bs->buf_bit_idx - k);
		bs->buf_bit_idx -= k;
		if (!bs->buf_bit_idx) {
			bs->buf_bit_idx = 8;
			bs->buf_byte_idx++;
			if (bs->buf_byte_idx >= bs->buf_size) {
				//empty_buffer (bs, minimum);
				fprintf(stdout,"buffer_putbits: error. bit_stream buffer needs to be bigger\n");
				exit(99);	
			}
			bs->buf[bs->buf_byte_idx] = 0;
		}
		j -= k;
	}
}

/*return the current bit stream length (in bits)*/
unsigned long buffer_sstell (bit_stream * bs)
{
	return (bs->totbit);
}

