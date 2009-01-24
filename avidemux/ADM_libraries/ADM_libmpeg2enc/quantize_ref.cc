/* quantize.c, Low-level quantization / inverse quantization
 * routines */

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

/* Modifications and enhancements (C) 2000-2003 Andrew Stevens */

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
#include <math.h>
#include <stdlib.h>
#ifdef HAVE_FENV_H
#include <fenv.h>
#endif
#include "mjpeg_logging.h"
#include "syntaxparams.h"
#include "fastintfns.h"

#include "cpu_accel.h"
#include "simd.h"
#include "ADM_cpuCap.h"

#ifdef HAVE_ALTIVEC
void enable_altivec_quantization(int op);//t->mpeg1);
#endif


int (*pquant_non_intra)( int16_t *src, int16_t *dst,
                                int q_scale_type, 
                                int *nonsat_mquant);
int (*pquant_weight_coeff_sum)(int16_t *blk, uint16_t*i_quant_mat );

void (*piquant_non_intra)(int16_t *src, int16_t *dst, int mquant );

/* non-linear quantization coefficient table */
uint8_t non_linear_mquant_table[32] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	8,10,12,14,16,18,20,22,
	24,28,32,36,40,44,48,52,
	56,64,72,80,88,96,104,112
};

/* non-linear mquant table for mapping from scale to code
 * since reconstruction levels are not bijective with the index map,
 * it is up to the designer to determine most of the quantization levels
 */

uint8_t map_non_linear_mquant[113] =
{
	0,1,2,3,4,5,6,7,8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,16,16,
	16,17,17,17,18,18,18,18,19,19,19,19,20,20,20,20,21,21,21,21,22,22,
	22,22,23,23,23,23,24,24,24,24,24,24,24,25,25,25,25,25,25,25,26,26,
	26,26,26,26,26,26,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,29,
	29,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,31,31,31,31,31
};

/* Table driven intra / non-intra quantization matrices */
uint16_t intra_q_tbl[113][64], inter_q_tbl[113][64];
uint16_t i_intra_q_tbl[113][64], i_inter_q_tbl[113][64];
float intra_q_tblf[113][64], inter_q_tblf[113][64];
float i_intra_q_tblf[113][64], i_inter_q_tblf[113][64];

/*
 * Return the code for a quantisation level
 */

int quant_code(  int q_scale_type, int mquant )
{
	return q_scale_type ? map_non_linear_mquant[ mquant] : mquant>>1;
}

/*
 *
 * Computes the next quantisation up.  Used to avoid saturation
 * in macroblock coefficients - common in MPEG-1 - which causes
 * nasty artefacts.
 *
 * NOTE: Does no range checking...
 *
 */
 

int next_larger_quant( int q_scale_type, int quant )
{
	if( q_scale_type )
	{
		if( map_non_linear_mquant[quant]+1 > 31 )
			return quant;
		else
			return non_linear_mquant_table[map_non_linear_mquant[quant]+1];
	}
	else 
	{
		return ( quant+2 > 31 ) ? quant : quant+2;
	}
}

/* 
 * Quantisation for intra blocks using Test Model 5 quantization
 *
 * this quantizer has a bias of 1/8 stepsize towards zero
 * (except for the DC coefficient)
 *
	PRECONDITION: src dst point to *disinct* memory buffers...
		          of block_count *adjact* int16_t[64] arrays... 
 *
 * RETURN: 1 If non-zero coefficients left after quantisaiont 0 otherwise
 */

void mp2_quant_intra( int16_t *src, 
				  int16_t *dst,
				  int q_scale_type, int dc_prec,
				  int *nonsat_mquant
	)
{
  int16_t *psrc,*pbuf;
  int i,comp;
  int x, y, d;
  int clipping;
  int mquant = *nonsat_mquant;
  int clipvalue  = opt->dctsatlim;
  uint16_t *quant_mat = intra_q_tbl[mquant] /* intra_q */;


  /* Inspired by suggestion by Juan.  Quantize a little harder if we clip...
   */

  do
	{
	  clipping = 0;
	  pbuf = dst;
	  psrc = src;
	  for( comp = 0; comp<block_count && !clipping; ++comp )
	  {
		x = psrc[0];
		d = 8>>dc_prec; /* intra_dc_mult */
		pbuf[0] = (x>=0) ? (x+(d>>1))/d : -((-x+(d>>1))/d); /* round(x/d) */


		for (i=1; i<64 ; i++)
		  {
			x = psrc[i];
			d = quant_mat[i];
#ifdef ORIGINAL_CODE
			y = (32*(x >= 0 ? x : -x) + (d>>1))/d; /* round(32*x/quant_mat) */
			d = (3*mquant+2)>>2;
			y = (y+d)/(2*mquant); /* (y+0.75*mquant) / (2*mquant) */

			/* clip to syntax limits */
			if (y > 255)
			  {
				if (mpeg1)
				  y = 255;
				else if (y > 2047)
				  y = 2047;
			  }
#else
			/* RJ: save one divide operation */
			y = ((abs(x)<<5)+ ((3*quant_mat[i])>>2))/(quant_mat[i]<<1)
				/*(32*abs(x) + (d>>1) + d*((3*mquant+2)>>2))/(quant_mat[i]*2*mquant) */
				;
			if ( y > clipvalue )
			  {
				clipping = 1;
				mquant = next_larger_quant(q_scale_type, mquant );
				quant_mat = intra_q_tbl[mquant];
				break;
			  }
#endif
		  
		  	pbuf[i] = intsamesign(x,y);
		  }
		pbuf += 64;
		psrc += 64;
	  }
			
	} while( clipping );
  *nonsat_mquant = mquant;
}


/*
 * Quantisation matrix weighted Coefficient sum fixed-point
 * integer with low 16 bits fractional...
 * To be used for rate control as a measure of dct block
 * complexity...
 *
 */

int quant_weight_coeff_sum( int16_t *blk, uint16_t * i_quant_mat )
{
  int i;
  int sum = 0;
   for( i = 0; i < 64; i+=2 )
	{
		sum += abs((int)blk[i]) * (i_quant_mat[i]) + abs((int)blk[i+1]) * (i_quant_mat[i+1]);
	}
    return sum;
	/* In case you're wondering typical average coeff_sum's for a rather
	 noisy video are around 20.0.  */
}


							     
/* 
 * Quantisation for non-intra blocks using Test Model 5 quantization
 *
 * this quantizer has a bias of 1/8 stepsize towards zero
 * (except for the DC coefficient)
 *
 * A.Stevens 2000: The above comment is nonsense.  Only the intra quantiser does
 * this.  This one just truncates with a modest bias of 1/(4*quant_matrix_scale)
 * to 1.
 *
 *	PRECONDITION: src dst point to *disinct* memory buffers...
 *	              of block_count *adjacent* int16_t[64] arrays...
 *
 * RETURN: A bit-mask of block_count bits indicating non-zero blocks (a 1).
 *
 */
																							     											     
int quant_non_intra( int16_t *src, int16_t *dst,
					 int q_scale_type,
					 int *nonsat_mquant)
{
	int i;
	int x, y, d;
	int nzflag;
	int coeff_count;
	int clipvalue  = opt->dctsatlim;
	int flags = 0;
	int saturated = 0;
    int mquant = *nonsat_mquant;
	uint16_t *quant_mat = inter_q_tbl[mquant]; /* inter_q */
	
	coeff_count = 64*block_count;
	flags = 0;
	nzflag = 0;
	for (i=0; i<coeff_count; ++i)
	{
restart:
		if( (i%64) == 0 )
		{
			nzflag = (nzflag<<1) | !!flags;
			flags = 0;
			  
		}
		/* RJ: save one divide operation */

		x = abs( ((int)src[i]) ) /*(src[i] >= 0 ? src[i] : -src[i])*/ ;
		d = (int)quant_mat[(i&63)]; 
		/* A.Stevens 2000: Given the math of non-intra frame
		   quantisation / inverse quantisation I always though the
		   funny little foudning factor was bogus.  It seems to be
		   the encoder needs less bits if you simply divide!
		*/

		y = (x<<4) /  (d) /* (32*x + (d>>1))/(d*2*mquant)*/ ;
		if ( y > clipvalue )
		{
			if( saturated )
			{
				y = clipvalue;
			}
			else
			{
				int new_mquant = next_larger_quant( q_scale_type, mquant );
				if( new_mquant != mquant )
				{
					mquant = new_mquant;
					quant_mat = inter_q_tbl[mquant];
				}
				else
				{
					saturated = 1;
				}
				i=0;
				nzflag =0;
				goto restart;
			}
		}
		dst[i] = intsamesign(src[i], y) /* (src[i] >= 0 ? y : -y) */;
		flags |= dst[i];
	}
	nzflag = (nzflag<<1) | !!flags;

    *nonsat_mquant = mquant;
    return nzflag;
}

/* MPEG-1 inverse quantization */
static void iquant1_intra(int16_t *src, int16_t *dst, int dc_prec, int mquant)
{
  int i, val;
  uint16_t *quant_mat = opt->intra_q;

  dst[0] = src[0] << (3-dc_prec);
  for (i=1; i<64; i++)
  {
    val = (int)(src[i]*quant_mat[i]*mquant)/16;

    /* mismatch control */
    if ((val&1)==0 && val!=0)
      val+= (val>0) ? -1 : 1;

    /* saturation */
    dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
  }
}


/* MPEG-2 inverse quantization */
void iquant_intra(int16_t *src, int16_t *dst, int dc_prec, int mquant)
{
  int i, val, sum;

  if ( opt->mpeg1  )
    iquant1_intra(src,dst,dc_prec, mquant);
  else
  {
    sum = dst[0] = src[0] << (3-dc_prec);
    for (i=1; i<64; i++)
    {
      val = (int)(src[i]*opt->intra_q[i]*mquant)/16;
      sum+= dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
    }

    /* mismatch control */
    if ((sum&1)==0)
      dst[63]^= 1;
  }
}


void iquant_non_intra_m1(int16_t *src, int16_t *dst,  uint16_t *quant_mat)
{
  int i, val;

#ifndef ORIGINAL_CODE

  for (i=0; i<64; i++)
  {
    val = src[i];
    if (val!=0)
    {
      val = (int)((2*val+(val>0 ? 1 : -1))*quant_mat[i])/32;

      /* mismatch control */
      if ((val&1)==0 && val!=0)
        val+= (val>0) ? -1 : 1;
    }

    /* saturation */
     dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
 }
#else
  
  for (i=0; i<64; i++)
  {
    val = abs(src[i]);
    if (val!=0)
    {
		val = ((val+val+1)*quant_mat[i]) >> 5;
		/* mismatch control */
		val -= (~(val&1))&(val!=0);
		val = fastmin(val, 2047); /* Saturation */
    }
	dst[i] = intsamesign(src[i],val);
	
  }
  
#endif
}




void iquant_non_intra(int16_t *src, int16_t *dst, int mquant )
{
  int i, val, sum;
  uint16_t *quant_mat;

  if ( opt->mpeg1 )
	  iquant_non_intra_m1(src,dst,inter_q_tbl[mquant]);
  else
  {
	  sum = 0;
#ifdef ORIGINAL_CODE

	  for (i=0; i<64; i++)
	  {
		  val = src[i];
		  if (val!=0)
			  
			  val = (int)((2*val+(val>0 ? 1 : -1))*opt->inter_q[i]*mquant)/32;
		  sum+= dst[i] = (val>2047) ? 2047 : ((val<-2048) ? -2048 : val);
	  }
#else
	  quant_mat = inter_q_tbl[mquant];
	  for (i=0; i<64; i++)
	  {
		  val = src[i];
		  if( val != 0 )
		  {
			  val = abs(val);
			  val = (int)((val+val+1)*quant_mat[i])>>5;
			  val = intmin( val, 2047);
			  sum += val;
		  }
		  dst[i] = intsamesign(src[i],val);
	  }
#endif

    /* mismatch control */
    if ((sum&1)==0)
      dst[63]^= 1;
  }
}

/*
  Initialise quantization routines.  Currently just setting up MMX
  routines if available.  

  TODO: The initialisation of the quantisation tables should move
  here...
*/

void init_quantizer(void)
{
  pquant_weight_coeff_sum = quant_weight_coeff_sum;
  piquant_non_intra = iquant_non_intra;
#ifdef HAVE_X86CPU
        if(CpuCaps::hasMMX())
        {
                printf("[Mpeg2enc]Using MMX quant non intra\n");
                pquant_non_intra = quant_non_intra_mmx;
                
        }
        else
#endif      
              printf("[Mpeg2enc]Using C quant non intra\n");   
}


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
