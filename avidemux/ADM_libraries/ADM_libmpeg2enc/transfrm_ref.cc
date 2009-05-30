/* transfrm.c,   Low-level Architecture neutral DCT/iDCT and prediction
   difference / addition routines
   
*/

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
#include "ADM_default.h"
#include <stdio.h>
#include <math.h>
#include "mjpeg_types.h"
#include "mjpeg_logging.h"
#include "syntaxparams.h"
#include "transfrm_ref.h"
#include "cpu_accel.h"
#include "simd.h"

#ifdef HAVE_ALTIVEC
//#include "../utils/altivec/altivec_transform.h"
#endif

#ifdef HAVE_X86CPU

	void add_pred_mmx (uint8_t *pred, uint8_t *cur,
						  int lx, int16_t *blk) ;
	void sub_pred_mmx (uint8_t *pred, uint8_t *cur,
						  int lx, int16_t *blk) ;
 	void mp2_fdct_mmx(short int *block);;

	int field_dct_best_mmx( uint8_t *cur_lum_mb, uint8_t *pred_lum_mb);
	void mp2_idct_mmx( int16_t * blk ) ;
 #endif
void fdct( int16_t *blk );
void init_fdct (void);

void idct( int16_t *blk );
void init_idct (void);

/*
  Pointers to version of transform and prediction manipulation
  routines to be used..
 */

void (*pfdct)( int16_t * blk );
void (*pidct)( int16_t * blk );
void (*padd_pred) (uint8_t *pred, uint8_t *cur,
				   int lx, int16_t *blk);
void (*psub_pred) (uint8_t *pred, uint8_t *cur,
				   int lx, int16_t *blk);
int (*pfield_dct_best)( uint8_t *cur_lum_mb, uint8_t *pred_lum_mb);


int field_dct_best( uint8_t *cur_lum_mb, uint8_t *pred_lum_mb)
{
	/*
	 * calculate prediction error (cur-pred) for top (blk0)
	 * and bottom field (blk1)
	 */
	double r,d;
	int rowoffs = 0;
	int sumtop, sumbot, sumsqtop, sumsqbot, sumbottop;
	int j,i;
	int topvar, botvar;
	sumtop = sumsqtop = sumbot = sumsqbot = sumbottop = 0;
	for (j=0; j<8; j++)
	{
		for (i=0; i<16; i++)
		{
			register int toppix = 
				cur_lum_mb[rowoffs+i] - pred_lum_mb[rowoffs+i];
			register int botpix = 
				cur_lum_mb[rowoffs+opt->phy_width+i] 
				- pred_lum_mb[rowoffs+opt->phy_width+i];
			sumtop += toppix;
			sumsqtop += toppix*toppix;
			sumbot += botpix;
			sumsqbot += botpix*botpix;
			sumbottop += toppix*botpix;
		}
		rowoffs += (opt->phy_width<<1);
	}

	/* Calculate Variances top and bottom.  If they're of similar
	 sign estimate correlation if its good use frame DCT otherwise
	 use field.
	*/
	r = 0.0;
	topvar = sumsqtop-sumtop*sumtop/128;
	botvar = sumsqbot-sumbot*sumbot/128;
	if (!((topvar>0) ^ (botvar>0)))
	{
		d = ((double) topvar) * ((double)botvar);
		r = (sumbottop-(sumtop*sumbot)/128);
		if (r>0.5*sqrt(d))
			return 0; /* frame DCT */
		else
			return 1; /* field DCT */
	}
        return 1; /* field DCT */
}



/* add prediction and prediction error, saturate to 0...255 */

void add_pred(uint8_t *pred, uint8_t *cur,
			  int lx,
			  int16_t *blk)
{
	int i, j;

	for (j=0; j<8; j++)
	{
		for (i=0; i<8; i++)
		{
			int16_t rawsum = blk[i] + pred[i];
			cur[i] = (rawsum<0) ? 0 : ((rawsum>255) ? 255 : rawsum);
		}
		blk+= 8;
		cur+= lx;
		pred+= lx;
	}
}


/* subtract prediction from block data */
/* static */
void sub_pred(uint8_t *pred, uint8_t *cur, int lx, 	int16_t *blk)
{
	int i, j;

	for (j=0; j<8; j++)
	{
		for (i=0; i<8; i++)
			blk[i] = cur[i] - pred[i];
		blk+= 8;
		cur+= lx;
		pred+= lx;
	}
}


/*
  Initialise DCT transformation routines.  Selects the appropriate
  architecture dependent SIMD routines and initialises pre-computed tables
 */

void init_transform(void)
{

	// By default use C implementation
#ifdef HAVE_X86CPU
                if(CpuCaps::hasSSE())
                {
		  pfdct = mp2_fdct_sse;
		  pidct = mp2_idct_sse;
		  padd_pred = add_pred_mmx;
		  psub_pred = sub_pred_mmx;
		  pfield_dct_best = field_dct_best_mmx;
                  init_mp2_fdct_sse();
                  printf("[Mpeg2enc] SSE idct/fdct\n");
		  
                } else
                if(CpuCaps::hasMMX())
                {
		  pfdct = mp2_fdct_mmx;
		  pidct = mp2_idct_mmx;
		  padd_pred = add_pred_mmx;
		  psub_pred = sub_pred_mmx;
		  pfield_dct_best = field_dct_best_mmx;
                  printf("[Mpeg2enc] MMX idct/fdct\n");
		  
                }
                else
                
#endif
                {
		pfdct = fdct;
		pidct = idct;
		padd_pred = add_pred;
		psub_pred = sub_pred;
		pfield_dct_best = field_dct_best;
                printf("[Mpeg2enc] C idct/fdct\n");
#ifndef HAVE_X86CPU
		printf("Because not X86 Arch\n");
#endif
                }
		init_fdct();
		init_idct();
                
}                
	// MEANX
#if 0
#if defined(HAVE_ASM_MMX) && defined(HAVE_ASM_NASM) 
	if( (flags & ACCEL_X86_MMX) ) /* MMX CPU */
	{
		mjpeg_info( "SETTING MMX for TRANSFORM!");
		pfdct = fdct_mmx;
		pidct = idct_mmx;
		padd_pred = add_pred_mmx;
		psub_pred = sub_pred_mmx;
		pfield_dct_best = field_dct_best_mmx;
	}
	else
#endif
	{
		pfdct = fdct;
		pidct = idct;
		padd_pred = add_pred;
		psub_pred = sub_pred;
		pfield_dct_best = field_dct_best;
	}

#ifdef HAVE_ALTIVEC
	if (flags > 0)
	{
#if ALTIVEC_TEST_TRANSFORM
#  if defined(ALTIVEC_BENCHMARK)
	    mjpeg_info("SETTING AltiVec BENCHMARK for TRANSFORM!");
#  elif defined(ALTIVEC_VERIFY)
	    mjpeg_info("SETTING AltiVec VERIFY for TRANSFORM!");
#  endif
#else
	    mjpeg_info("SETTING AltiVec for TRANSFORM!");
#endif

#if ALTIVEC_TEST_FUNCTION(fdct)
	    pfdct = ALTIVEC_TEST_SUFFIX(fdct);
#else
	    pfdct = ALTIVEC_SUFFIX(fdct);
#endif

#if ALTIVEC_TEST_FUNCTION(idct)
	    pidct = ALTIVEC_TEST_SUFFIX(idct);
#else
	    pidct = ALTIVEC_SUFFIX(idct);
#endif

#if ALTIVEC_TEST_FUNCTION(add_pred)
	    padd_pred = ALTIVEC_TEST_SUFFIX(add_pred);
#else
	    padd_pred = ALTIVEC_SUFFIX(add_pred);
#endif

#if ALTIVEC_TEST_FUNCTION(sub_pred)
	    psub_pred = ALTIVEC_TEST_SUFFIX(sub_pred);
#else
	    psub_pred = ALTIVEC_SUFFIX(sub_pred);
#endif
	}
#endif /* HAVE_ALTIVEC */
	init_fdct();
	init_idct();
#endif

