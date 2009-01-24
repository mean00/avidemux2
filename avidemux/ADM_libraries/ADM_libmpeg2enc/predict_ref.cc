/* predict_ref.c, Reference implementations of motion compensated
 * prediction routines */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */
/* Modifications and enhancements (C) 2000/2001 Andrew Stevens */

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

#include <stdio.h>
#include "config.h"
#include "mjpeg_types.h"
#include "mjpeg_logging.h"
#include "syntaxparams.h"
#include "mpeg2enc.h"
#include "predict_ref.h"
#include "cpu_accel.h"
#include "simd.h"

#include <stdio.h>


#ifdef HAVE_ALTIVEC
#include "altivec/altivec_predict.h"
#endif

void (*ppred_comp)( uint8_t *src, uint8_t *dst,
					int lx, int w, int h, int x, int y, int dx, int dy,
					int addflag);



/* low level prediction routine (Reference implementation)
 *
 * src:     prediction source
 * dst:     prediction destination
 * lx:      line width (for both src and dst)
 * x,y:     destination coordinates
 * dx,dy:   half pel motion vector
 * w,h:     size of prediction block
 * addflag: store or add prediction
 *
 * There are also SIMD versions of this routine...
 */

void pred_comp(
	uint8_t *src,
	uint8_t *dst,
	int lx,
	int w, int h,
	int x, int y,
	int dx, int dy,
	int addflag)
{
	int xint, xh, yint, yh;
	int i, j;
	uint8_t *s, *d;

	/* half pel scaling */
	xint = dx>>1; /* integer part */
	xh = dx & 1;  /* half pel flag */
	yint = dy>>1;
	yh = dy & 1;

	/* origins */
	s = src + lx*(y+yint) + (x+xint); /* motion vector */
	d = dst + lx*y + x;

	if (!xh && !yh)
		if (addflag)
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = (unsigned int)(d[i]+s[i]+1)>>1;
				s+= lx;
				d+= lx;
			}
		else
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = s[i];
				s+= lx;
				d+= lx;
			}
	else if (!xh && yh)
		if (addflag)
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = (d[i] + ((unsigned int)(s[i]+s[i+lx]+1)>>1)+1)>>1;
				s+= lx;
				d+= lx;
			}
		else
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = (unsigned int)(s[i]+s[i+lx]+1)>>1;
				s+= lx;
				d+= lx;
			}
	else if (xh && !yh)
		if (addflag)
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = (d[i] + ((unsigned int)(s[i]+s[i+1]+1)>>1)+1)>>1;
				s+= lx;
				d+= lx;
			}
		else
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = (unsigned int)(s[i]+s[i+1]+1)>>1;
				s+= lx;
				d+= lx;
			}
	else /* if (xh && yh) */
		if (addflag)
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = (d[i] + ((unsigned int)(s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2)+1)>>1;
				s+= lx;
				d+= lx;
			}
		else
			for (j=0; j<h; j++)
			{
				for (i=0; i<w; i++)
					d[i] = (unsigned int)(s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2;
				s+= lx;
				d+= lx;
			}
}


/* calculate derived motion vectors (DMV) for dual prime prediction
 * dmvector[2]: differential motion vectors (-1,0,+1)
 * mvx,mvy: motion vector (for same parity)
 *
 * DMV[2][2]: derived motion vectors (for opposite parity)
 *
 * uses global variables pict_struct and topfirst
 *
 * Notes:
 *  - all vectors are in field coordinates (even for frame pictures)
 *
 */

void calc_DMV( int pict_struct,  bool topfirst,
			   int DMV[][2], int *dmvector, int mvx, int mvy
)
{
  if (pict_struct==FRAME_PICTURE)
  {
    if (topfirst)
    {
      /* vector for prediction of top field from bottom field */
      DMV[0][0] = ((mvx  +(mvx>0))>>1) + dmvector[0];
      DMV[0][1] = ((mvy  +(mvy>0))>>1) + dmvector[1] - 1;

      /* vector for prediction of bottom field from top field */
      DMV[1][0] = ((3*mvx+(mvx>0))>>1) + dmvector[0];
      DMV[1][1] = ((3*mvy+(mvy>0))>>1) + dmvector[1] + 1;
    }
    else
    {
      /* vector for prediction of top field from bottom field */
      DMV[0][0] = ((3*mvx+(mvx>0))>>1) + dmvector[0];
      DMV[0][1] = ((3*mvy+(mvy>0))>>1) + dmvector[1] - 1;

      /* vector for prediction of bottom field from top field */
      DMV[1][0] = ((mvx  +(mvx>0))>>1) + dmvector[0];
      DMV[1][1] = ((mvy  +(mvy>0))>>1) + dmvector[1] + 1;
    }
  }
  else
  {
    /* vector for prediction from field of opposite 'parity' */
    DMV[0][0] = ((mvx+(mvx>0))>>1) + dmvector[0];
    DMV[0][1] = ((mvy+(mvy>0))>>1) + dmvector[1];

    /* correct for vertical field shift */
    if (pict_struct==TOP_FIELD)
      DMV[0][1]--;
    else
      DMV[0][1]++;
  }
}

void clearblock( int pict_struct,
				 uint8_t *cur[], int i0, int j0
	)
{
  int i, j, w, h;
  uint8_t *p;

  p = cur[0] 
	  + ((pict_struct==BOTTOM_FIELD) ? opt->phy_width : 0) 
	  + i0 + opt->phy_width2*j0;

  for (j=0; j<16; j++)
  {
    for (i=0; i<16; i++)
      p[i] = 128;
    p+= opt->phy_width2;
  }

  w = h = 16;

  if (opt->chroma_format!=CHROMA444)
  {
    i0>>=1; w>>=1;
  }

  if (opt->chroma_format==CHROMA420)
  {
    j0>>=1; h>>=1;
  }

  p = cur[1] 
	  + ((pict_struct==BOTTOM_FIELD) ? opt->phy_chrom_width : 0) 
	  + i0 + opt->phy_chrom_width2*j0;

  for (j=0; j<h; j++)
  {
    for (i=0; i<w; i++)
      p[i] = 128;
    p+= opt->phy_chrom_width2;
  }

  p = cur[2] 
	  + ((pict_struct==BOTTOM_FIELD) ? opt->phy_chrom_width : 0) 
	  + i0 + opt->phy_chrom_width2*j0;

  for (j=0; j<h; j++)
  {
    for (i=0; i<w; i++)
      p[i] = 128;
    p+= opt->phy_chrom_width2;
  }
}


/*
  Initialise prediction - currently purely selection of which
  versions of the various low level computation routines to use
  
  */

void init_predict(void)
{
	int cpucap = cpu_accel();

	ppred_comp = pred_comp; /* Default safe value */
	if( cpucap  == 0 )	/* No MMX/SSE etc support available */
	{
		ppred_comp = pred_comp;
                printf("[Mpeg2enc] C predict (NO ACCEL)\n");
	}

#ifdef HAVE_X86CPU
	else if(cpucap & ACCEL_X86_MMXEXT ) /* AMD MMX or SSE... */
	{
                printf("[Mpeg2enc] MMXE predict\n");
		ppred_comp = pred_comp_mmxe;
	}
    else if(cpucap & ACCEL_X86_MMX ) /* Original MMX... */
	{
                printf("[Mpeg2enc] MMX predict\n");
		ppred_comp = pred_comp_mmx;
	}
#endif
#ifdef HAVE_ALTIVEC
    else
	{
#  if ALTIVEC_TEST_PREDICT
#    if defined(ALTIVEC_BENCHMARK)
	    mjpeg_info("SETTING AltiVec BENCHMARK for PREDICTION!");
#    elif defined(ALTIVEC_VERIFY)
	    mjpeg_info("SETTING AltiVec VERIFY for PREDICTION!");
#    endif
#  else
	    mjpeg_info("SETTING AltiVec for PREDICTION!");
#  endif

#  if ALTIVEC_TEST_FUNCTION(pred_comp)
	    ppred_comp = ALTIVEC_TEST_SUFFIX(pred_comp);
#  else
	    ppred_comp = ALTIVEC_SUFFIX(pred_comp);
#  endif
	}
#endif /* HAVE_ALTIVEC */
}



/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
