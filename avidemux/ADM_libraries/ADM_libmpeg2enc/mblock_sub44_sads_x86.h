/* (C) 2000/2002 Andrew Stevens */

/*  This software is free software; you can redistribute it
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

#ifndef __MBLOCK_SUB44_SADS_X86_H__
#define __MBLOCK_SUB44_SADS_X86__

#include <config.h>
#include "mjpeg_types.h"
int mblocks_sub44_mests_mmxe( uint8_t *blk,  uint8_t *ref,
							 int ilow, int jlow,
							 int ihigh, int jhigh, 
							 int h, int rowstride, 
							 int threshold,
							 me_result_s *resvec);
int mblocks_sub44_mests_mmx( uint8_t *blk,  uint8_t *ref,
							int ilow, int jlow,
							int ihigh, int jhigh, 
							int threshold,
							int h, int rowstride, 
							me_result_s *resvec);
#endif
