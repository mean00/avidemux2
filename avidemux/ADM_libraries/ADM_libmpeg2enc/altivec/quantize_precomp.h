/*
 * quantize_precomp.h, Precomputed quantisation tables for various fast
 * implementations of quantization/ inverse quantization routines
 *
 */

/*  (C) 2003 Andrew Stevens */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#ifndef _QUANTIZE_PRECOMP_H
#define _QUANTIZE_PRECOMP_H

struct QuantizerWorkSpace
{
	uint16_t intra_q_mat[64], i_intra_q_mat[64];
	uint16_t inter_q_mat[64], i_inter_q_mat[64];
	uint16_t intra_q_tbl[113][64], inter_q_tbl[113][64];
	uint16_t i_intra_q_tbl[113][64], i_inter_q_tbl[113][64];
	uint16_t r_intra_q_tbl[113][64], r_inter_q_tbl[113][64];

	float intra_q_tblf[113][64], inter_q_tblf[113][64];
	float i_intra_q_tblf[113][64], i_inter_q_tblf[113][64];
};


#endif
