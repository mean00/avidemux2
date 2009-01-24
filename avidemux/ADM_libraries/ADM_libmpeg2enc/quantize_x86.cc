/* quantize_x86.c Quantization / inverse quantization    
   In compiler (gcc) embdeed assmbley language...
*/

/* Copyright (C) 2000 Andrew Stevens */

/* This program is free software; you can redistribute it
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




#include <config.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifdef HAVE_X86CPU

#include "syntaxparams.h"
#include "mjpeg_logging.h"
/*#include "fastintfns.h"
#include "cpu_accel.h"*/
// #include "simd.h"
#include "mmx.h"

#include "tables.h"
//#include "quantize_precomp.h"
#include "quantize_ref.h"
					
int quant_weight_coeff_sum_mmx (int16_t *blk, uint16_t *i_quant_mat );

void iquantize_non_intra_m1_mmx(int16_t *src, int16_t *dst, uint16_t *qmat);
void iquantize_non_intra_m2_mmx(int16_t *src, int16_t *dst, uint16_t *qmat);

/* 
 * Quantisation for non-intra blocks 
 *
 * Various versions for various SIMD instruction sets.  Not all of them
 * bother to implement the test model 5 quantisation of the reference source
 * (this has a bias of 1/8 stepsize towards zero - except for the DC coefficient).
 *
 * Actually, as far as I can tell even the reference source doesn't quite do it
 * for non-intra (though it *does* for intra).
 * 
 * Careful analysis of the code also suggests what it actually does is truncate
 * with a modest bias towards 1 (the d>>2 factor)
 *
 *	PRECONDITION: src dst point to *disinct* memory buffers...
 *	              of block_count *adjacent* int16_t[64] arrays...
 *
 *RETURN: A bit-mask of block_count bits indicating non-zero blocks (a 1).
 */
	
#if 0

/*
 * SSE version: simply truncates to zero, however, the tables have a 2% bias
 * upwards which partly compensates.
 */
 
static int quant_non_intra_sse( struct QuantizerWorkSpace *wsp,
				int16_t *src, int16_t *dst,
				int q_scale_type,
				int satlim,
				int *nonsat_mquant)
{
  __m64 saturated;
  float *i_quant_matf; 
  int mquant = *nonsat_mquant;
  const int coeff_count = 64*BLOCK_COUNT;
  uint32_t nzflag;
  uint64_t flags;
  __m64 *psrc, *pdst;
  __m128 *piqf;
  int i;
  uint32_t tmp;
  // Set truncation as rounding mode...
  uint32_t oldcsr = _mmgetcsr();
  
  _mm_setcsr( (oldcsr & ~_MM_ROUND_MASK        0x6000) | _MM_ROUND_TOWARD_ZERO );
  __m64 zero = _mm_setzero_si64();
  __m64 satlim_0123 = _mm_set1_pi16( (int16_t)satlim);
  __m64 neg_satlim_0123 = _mm_set1_pi16( (int16_t)-satlim);

restart:
  i_quant_matf = (__m128 *)wsp->i_inter_q_tblf[mquant];
  flags = 0;
  piqf = i_quant_matf;
  saturated = _mm_setzero_si64();
  nzflag = 0;
  psrc = (__m64 *)src;
  pdst = (__m64 *)dst;
  for (i=0; i < coeff_count/(2*4) ; i += 2)
    {
      int j = i&63;
      // Sign extend psrc[i] to double words
      __m128 psrc_i_0_01234 = _mm_cvtpi16_ps( psrc[i] );
      
      __m128 psrc_i_4_01234 = _mm_cvtpi16_ps( psrc[i+4] );
      
      // "Divide" by multiplying by inverse quantisation
      psrc_i_0_01234 = _mm_mul_ps( psrc_i_0_01234, piqf[j] );
      psrc_i_4_01234 = _mm_mul_ps( psrc_i_4_01234, piqf[j] );
      
      // Accumulate saturation
      __m64 rawquant_0_0123 = _mm_cvtps_pi16(psrc_i_0_01234);
      __m64 rawquant_4_0123 = _mm_cvtps_pi16(psrc_i_4_01234);
      saturated = _mm_or_si64( saturated, _mm_cmpgt_pi16( rawquant_0_0123, satlim_0123 ) );
      saturated = _mm_or_si64( saturated, _mm_cmpgt_pi16( rawquant_4_0123, satlim_0123 ) );
      saturated = _mm_or_si64( saturated, _mm_cmpgt_pi16( rawquant_0_0123, neg_satlim_0123 ));
      saturated = _mm_or_si64( saturated, _mm_cmpgt_pi16( rawquant_4_0123, neg_satlim_0123 ));
      pdst[i] = rawquant_0_0123;
      pdst[i+4] = rawquant_4_0123;
      
      // Accumulate zero flags...
      flags = (uint64_t)_mm_or_si64( _mm_set_pi64x(flags), _mm_or_si64( rawquant_0_0123, rawquant_4_0123 ));
      
      // Check if we saturated every block (every 16 grou
      if( (i & (64/8-1)) == (64/8-1) )
	{
	  if( (uint64_t)saturated != (uint64_t)0 )
	    {
	      int new_mquant = next_larger_quant( q_scale_type, mquant );
	      if( new_mquant != mquant )
		{
		  mquant = new_mquant;
		  goto restart;
		}
	      else
		{
		  return quant_non_intra(wsp, src, dst, 
					 q_scale_type,
					 satlim,
					 nonsat_mquant);
		}
	    }
	  nzflag =  (nzflag<<1) | ((uint64_t)flags != (uint64_t)0 );
	  flags = 0;
	}
      _mm_empty();
      return nzflag;
}
#endif

/* MMX version, that is 100% accurate
   It works by multiplying by the inverse of the quant factor.  However
   in certain circumstances this results in a value too low.  To compensate,
   it then multiplies by the quant factor, and compares this to the original
   value.  If this new value is less than or equal to the original value
   minus the quant factor, then the intermediate result is incremented.
   This adjust-by-1 is all that is needed to correct the result.

   8 words are handled per iteration; first four words uses mm0-mm3,
   last four words use mm4-mm7.
*/
int quant_non_intra_mmx( int16_t *src, int16_t *dst,       int q_scale_type,               int *nonsat_mquant)
{
    int saturated,satlim;
    int mquant = *nonsat_mquant;
#define BLOCK_COUNT block_count
    int   coeff_count = 64*BLOCK_COUNT;
    uint32_t nzflag, flags;
    int16_t *psrc, *pdst, *pdiv, *pmul, *pdivbase, *pmulbase;
    int i;
    uint32_t tmp;
    uint64_t negone_q,satlim_q;

    /* Load -1 into negone_q */
    pxor_r2r( mm6, mm6 );
    pcmpeqw_r2r( mm6, mm6 );
    movq_r2m( mm6, negone_q );
    satlim=opt->dctsatlim;
    /* Load satlim into satlim_q */
    movd_g2r( satlim, mm7 );
    punpcklwd_r2r( mm7, mm7 );
    punpckldq_r2r( mm7, mm7 );
    movq_r2m( mm7, satlim_q );

    pdivbase = (int16_t *)i_inter_q_tbl[mquant];
    pmulbase = (int16_t *)inter_q_tbl[mquant];

 restart:
    flags = 0;
    pdiv = pdivbase;
    pmul = pmulbase;
    saturated = 0;
    nzflag = 0;
    psrc = src;
    pdst = dst;
    for (i=0; i < coeff_count ; i+=8)
    {
        movq_m2r( psrc[0], mm3 ); // load values
        movq_m2r( psrc[4], mm7 ); // load values
        psllw_i2r( 4, mm3 );   // multiply by 16
        psllw_i2r( 4, mm7 );   // multiply by 16
        movq_r2r( mm3, mm0 );  // keep sign in mm3, make mm0=abs(mm3)
        movq_r2r( mm7, mm4 );  // keep sign in mm3, make mm0=abs(mm3)
        psraw_i2r( 15, mm3 );
        psraw_i2r( 15, mm7 );
        pxor_r2r( mm3, mm0 );
        pxor_r2r( mm7, mm4 );
        psubw_r2r( mm3, mm0 );
        psubw_r2r( mm7, mm4 );
        movq_m2r( pdiv[0], mm1 ); // "divide" by quant
        movq_m2r( pdiv[4], mm5 ); // "divide" by quant
        pmulhw_r2r( mm0, mm1 );
        pmulhw_r2r( mm4, mm5 );
        movq_m2r( pmul[0], mm2 ); // check that x*q > X-q; i.e. make sure x is not off by too much
        movq_m2r( pmul[4], mm6 ); // check that x*q > X-q; i.e. make sure x is not off by too much
        psubw_r2r( mm2, mm0 );
        psubw_r2r( mm6, mm4 );
        pmullw_r2r( mm1, mm2 );
        pmullw_r2r( mm5, mm6 );
        pcmpgtw_r2r( mm0, mm2 );
        pcmpgtw_r2r( mm4, mm6 );
        pxor_m2r( negone_q, mm2 );
        pxor_m2r( negone_q, mm6 );
        psubw_r2r( mm2, mm1 ); // if x*q <= X-q, increment x by 1
        psubw_r2r( mm6, mm5 ); // if x*q <= X-q, increment x by 1
        movq_r2r( mm1, mm0 ); // stash abs of result away
        movq_r2r( mm5, mm4 ); // stash abs of result away
        pxor_r2r( mm3, mm1 ); // make result have same sign as orig value
        pxor_r2r( mm7, mm5 ); // make result have same sign as orig value
        psubw_r2r( mm3, mm1 );        
        psubw_r2r( mm7, mm5 );        
        movq_r2m( mm1, pdst[0] ); // store result
        movq_r2m( mm5, pdst[4] ); // store result
        
        por_r2r( mm5, mm1 );
        movq_r2r( mm1, mm2 ); // set flags for non null responses
        psrlq_i2r( 32, mm2 );
        por_r2r( mm1, mm2 );
        movd_r2g( mm2, tmp );
        flags |= tmp;

        pcmpgtw_m2r( satlim_q, mm0 ); // did result exceed satlim?
        pcmpgtw_m2r( satlim_q, mm4 );

        por_r2r( mm4, mm0 );
        movq_r2r( mm0, mm1 );
        psrlq_i2r( 32, mm1 );
        por_r2r( mm1, mm0 );
        movd_r2g( mm0, tmp );
        saturated |= tmp;

        pdiv += 8;
        pmul += 8;
        pdst += 8;
        psrc += 8;

        if( (i & 63) == (63/8)*8 )
        {
            if( saturated )
            {
                int new_mquant = next_larger_quant( q_scale_type, mquant );
                if( new_mquant != mquant )
                {
                    mquant = new_mquant;
                    goto restart;
                }
                else
                {
		    emms();
                    return quant_non_intra( src, dst, 
                                           q_scale_type,
                                           nonsat_mquant);
                }
            }

            nzflag = (nzflag<<1) | !!flags;
            flags = 0;
            pdiv = pdivbase;
            pmul = pmulbase;
        }
			
    }
    emms();

    //nzflag = (nzflag<<1) | (!!flags);
    return nzflag;
}
#if 0

static void iquant_non_intra_m1_mmx(struct QuantizerWorkSpace *wsp,
							 int16_t *src, int16_t *dst, int mquant )
{
	iquantize_non_intra_m1_mmx(src,dst,wsp->inter_q_tbl[mquant]);
}

static void iquant_non_intra_m2_mmx(struct QuantizerWorkSpace *wsp,
							 int16_t *src, int16_t *dst, int mquant )
{
	iquantize_non_intra_m2_mmx(src,dst,wsp->inter_q_tbl[mquant]);
}

static int quant_weight_coeff_x86_intra( struct QuantizerWorkSpace *wsp,
								  int16_t *blk )
{
	return quant_weight_coeff_sum_mmx( blk, wsp->i_intra_q_mat );
}

static int quant_weight_coeff_x86_inter( struct QuantizerWorkSpace *wsp,
								  int16_t *blk )
{
	return quant_weight_coeff_sum_mmx( blk, wsp->i_inter_q_mat );
}



extern void iquant_non_intra_m2(struct QuantizerWorkSpace *wsp,
                                int16_t *src, int16_t *dst, int mquant );
                                
static void iquant_non_intra_m2_mmxtest( struct QuantizerWorkSpace *wsp,
                                            int16_t *src, int16_t *dst, 
                                            int mquant)
{
    int i;
    int16_t ref[64];
    iquant_non_intra_m2(wsp, src,ref,mquant);
    iquant_non_intra_m2_mmx(wsp, src,dst,mquant);

    
    for( i = 0; i < 64; ++i )
    {
        if( dst[i] != ref[i] )
        {
            printf( "OUCH!!! Bad iquant coeff %d: %d ref=%d\n", i, dst[i], ref[i] );
            abort();
        } 
    }
}
static int quant_non_intra_test(struct QuantizerWorkSpace *wsp,
                                int16_t *src, int16_t *dst,
                                int q_scale_type,
                                int satlim,
                                int *nonsat_mquant)
{
    int16_t d2[64*BLOCK_COUNT];
    int rv1,rv2,mq1,mq2;

    mq1=*nonsat_mquant;
    rv1=quant_non_intra(wsp,src,d2,q_scale_type,satlim,&mq1);
    rv2=quant_non_intra_mmx(wsp,src,dst,q_scale_type,satlim,nonsat_mquant);
    mq2=*nonsat_mquant;

    if( rv1!=rv2 )
        mjpeg_warn("quant_non_intra disparity: ret=%d vs %d",rv1,rv2);
    if( mq1!=mq2 )
        mjpeg_warn("quant_non_intra disparity: mq=%d vs %d",mq1,mq2);
    if( memcmp(d2,dst,64*BLOCK_COUNT*sizeof(int16_t)) ) {
        int i;

        mjpeg_warn("quant_non_intra disparity: dst differs");
        for( i=0; i<64*BLOCK_COUNT; i++ ) {
            if( d2[i]!=dst[i] )
                mjpeg_warn("\t[%d]: %4d vs %4d",i,dst[i],d2[i]);
        }
    }

    return rv2;
}
#endif
#if 0
static int quant_non_intra_can_use_mmx(struct QuantizerWorkSpace *wsp)
{
    int i;

    for( i=0; i<64; i++ ) {
        int q=wsp->inter_q_mat[i];

        // if q<0, then bad things happen (mismatched sign)
        // if q==0, then divide by 0 happens
        // if q==1 or q==2, then 65536/q >= 32768 (negative sign when using signed multiply)
        // if q>292, then q*112 (which is the max quant scale) >=32768 (again, negative sign when using signed multiply)
        if( q<3 || q>292 )
            return 0;
    }
    return 1;
}

void init_x86_quantization( struct QuantizerCalls *qcalls,
                            struct QuantizerWorkSpace *wsp,
                            int mpeg1)
{
    int flags = cpu_accel();
    int d_quant_nonintra, d_weight_intra, d_weight_nonintra, d_iquant_intra;
    int d_iquant_nonintra;
    const char *opt_type1 = "", *opt_type2 = "";

    if  ((flags & ACCEL_X86_MMX) != 0 ) /* MMX CPU */
    {
	d_quant_nonintra = disable_simd("quant_nonintra");
	d_weight_intra = disable_simd("quant_weight_intra");
	d_weight_nonintra = disable_simd("quant_weight_nonintra");
	d_iquant_intra = disable_simd("iquant_intra");
	d_iquant_nonintra = disable_simd("iquant_nonintra");

        if  (d_quant_nonintra == 0)
        {
            if( quant_non_intra_can_use_mmx(wsp) )
            {
	        opt_type1 = "MMX and";
	        qcalls->pquant_non_intra = quant_non_intra_mmx;
            } else {
                mjpeg_warn("Non-intra quantization table out of range; disabling MMX");
            }
        }

        opt_type2 = "MMX";
        if (d_weight_intra == 0)
            qcalls->pquant_weight_coeff_intra = quant_weight_coeff_x86_intra;
        if (d_weight_nonintra == 0)
            qcalls->pquant_weight_coeff_inter = quant_weight_coeff_x86_inter;
        
        if (mpeg1)
        {
            if (d_iquant_nonintra == 0)
                qcalls->piquant_non_intra = iquant_non_intra_m1_mmx;
        }
        else
        {
            if (d_iquant_nonintra == 0)
                qcalls->piquant_non_intra = iquant_non_intra_m2_mmx;
        }
        
        if  (d_quant_nonintra)
            mjpeg_info(" Disabling quant_non_intra");
        if  (d_iquant_intra)
            mjpeg_info(" Disabling iquant_intra");
        if  (d_iquant_nonintra)
            mjpeg_info(" Disabling iquant_nonintra");
        if  (d_weight_intra)
            mjpeg_info(" Disabling quant_weight_intra");
        if (d_weight_nonintra)
            mjpeg_info(" Disabling quant_weight_nonintra");

	mjpeg_info( "SETTING %s %s for QUANTIZER!", opt_type1, opt_type2);
    }
}
#endif
#endif
