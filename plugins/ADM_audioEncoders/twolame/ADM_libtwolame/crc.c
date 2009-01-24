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
#include <string.h>

#include "twolame.h"
#include "common.h"
#include "bitbuffer.h"
#include "encode.h"
#include "crc.h"


/*****************************************************************************
*
*  CRC error protection package
*
*****************************************************************************/

void crc_calc (twolame_options *glopts, unsigned int bit_alloc[2][SBLIMIT],
	       unsigned int scfsi[2][SBLIMIT], unsigned int *crc)
{
  int i, k;
  frame_info *frame = &glopts->frame;
  frame_header *header = &glopts->header;
  int nch = frame->nch;
  int sblimit = frame->sblimit;
  int jsbound = frame->jsbound;

  *crc = 0xffff;		/* changed from '0' 92-08-11 shn */
  crc_update (header->bitrate_index, 4, crc);
  crc_update (header->samplerate_idx, 2, crc);
  crc_update (header->padding, 1, crc);
  crc_update (header->private_bit, 1, crc);
  crc_update (header->mode, 2, crc);
  crc_update (header->mode_ext, 2, crc);
  crc_update (header->copyright, 1, crc);
  crc_update (header->original, 1, crc);
  crc_update (header->emphasis, 2, crc);

  for (i = 0; i < sblimit; i++)
    for (k = 0; k < ((i < jsbound) ? nch : 1); k++)
      crc_update ( bit_alloc[k][i],
                   get_alloc_table_bits(glopts->tablenum, i, bit_alloc[k][i]),
                   crc );

  for (i = 0; i < sblimit; i++)
    for (k = 0; k < nch; k++)
      if (bit_alloc[k][i])
	crc_update (scfsi[k][i], 2, crc);
}

void crc_update (unsigned int data, unsigned int length, unsigned int *crc)
{
  unsigned int masking, carry;

  masking = 1 << length;

  while ((masking >>= 1)) {
    carry = *crc & 0x8000;
    *crc <<= 1;
    if (!carry ^ !(data & masking))
      *crc ^= CRC16_POLYNOMIAL;
  }
  *crc &= 0xffff;
}

