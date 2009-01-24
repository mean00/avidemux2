/* predict_ref.h, Low-level Architecture neutral prediction
 * (motion compensated reconstruction) routines */

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


#include "config.h"
#include "mjpeg_types.h"
#include "simd.h"
#include "predict_ref.h"
#ifdef HAVE_X86CPU
void pred_comp_mmxe(
	uint8_t *src,
	uint8_t *dst,
	int lx,
	int w, int h,
	int x, int y,
	int dx, int dy,
	int addflag)
{
	int xint, xh, yint, yh;
	uint8_t *s, *d;
	
	/* half pel scaling */
	xint = dx>>1; /* integer part */
	xh = dx & 1;  /* half pel flag */
	yint = dy>>1;
	yh = dy & 1;

	/* origins */
	s = src + lx*(y+yint) + (x+xint); /* motion vector */
	d = dst + lx*y + x;

	if( xh )
	{
		if( yh ) 
			predcomp_11_mmxe((char *)s,(char *)d,lx,w,h,addflag);
		else /* !yh */
			predcomp_10_mmxe((char *)s,(char *)d,lx,w,h,addflag);
	}
	else /* !xh */
	{
		if( yh ) 
			predcomp_01_mmxe((char *)s,(char *)d,lx,w,h,addflag);
		else /* !yh */
			predcomp_00_mmxe((char *)s,(char *)d,lx,w,h,addflag);
	}
		
}

void pred_comp_mmx(
	uint8_t *src,
	uint8_t *dst,
	int lx,
	int w, int h,
	int x, int y,
	int dx, int dy,
	int addflag)
{
	int xint, xh, yint, yh;
	uint8_t *s, *d;
	
	/* half pel scaling */
	xint = dx>>1; /* integer part */
	xh = dx & 1;  /* half pel flag */
	yint = dy>>1;
	yh = dy & 1;

	/* origins */
	s = src + lx*(y+yint) + (x+xint); /* motion vector */
	d = dst + lx*y + x;

	if( xh )
	{
		if( yh ) 
			predcomp_11_mmx((char *)s,(char *)d,lx,w,h,(int)addflag);
		else /* !yh */
			predcomp_10_mmx((char *)s,(char *)d,lx,w,h,(int)addflag);
	}
	else /* !xh */
	{
		if( yh ) 
			predcomp_01_mmx((char *)s,(char *)d,lx,w,h,(int)addflag);
		else /* !yh */
			predcomp_00_mmx((char *)s,(char *)d,lx,w,h,(int)addflag);
	}
		
}
#endif
