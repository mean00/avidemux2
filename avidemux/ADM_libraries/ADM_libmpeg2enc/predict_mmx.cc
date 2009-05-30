/* 
 *   predict.s:  mmX optimized block summing differencing routines
 * 
 *   Believed to be original Copyright (C) 2000 Brent Byeler
 * 
 *   This program is free software; you can reaxstribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either version 2
 *   of the License, or (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.   
 */

#include <ADM_default.h>
#include "mjpeg_types.h"
#ifdef HAVE_X86CPU
#include "mmx.h"
  
void sub_pred_mmx(unsigned char *pred, unsigned char *cur, int lx, int16_t *blk)
{
	int i;

	pxor_r2r(mm7, mm7);
	for(i=0; i<8; i++) 
	{
		movq_m2r(*cur, mm0);
		cur+=lx;	
		movq_m2r(*pred, mm2);
		pred+=lx;
		movq_r2r(mm0, mm1);
 		punpcklbw_r2r(mm7, mm0);
 		punpckhbw_r2r(mm7, mm1);
		movq_r2r(mm2, mm3);
 		punpcklbw_r2r(mm7, mm2);
 		punpckhbw_r2r(mm7, mm3);

        	psubw_r2r(mm2, mm0);
        	psubw_r2r(mm3, mm1);
	
		movq_r2m(mm0, *blk);
		movq_r2m(mm1, *(blk+4));
		blk+=8;
	}
	emms();
}

/* add prediction and prediction error, saturate to 0...255 */
void add_pred_mmx(unsigned char *pred, unsigned char *cur, int lx, int16_t *blk)
{
	int i;

	pxor_r2r(mm7, mm7);
	for(i=0; i<8; i++) 
	{
		movq_m2r(*blk, mm0);
		movq_m2r(*(blk+4), mm1);
		blk+=8;
		movq_m2r(*pred, mm2);
		pred+=lx;
		movq_r2r(mm2, mm3);
 		punpcklbw_r2r(mm7, mm2);
 		punpckhbw_r2r(mm7, mm3);

		paddw_r2r(mm2, mm0);
		paddw_r2r(mm3, mm1);
		packuswb_r2r(mm1, mm0);
		movq_r2m(mm0, *cur);
		cur+=lx;		
	}
	emms();
}
#endif

