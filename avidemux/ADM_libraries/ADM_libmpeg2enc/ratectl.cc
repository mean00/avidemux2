/* ratectl.c, bitrate control routines (linear quantization only currently) */

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

#include "config.h"
#include <math.h>
#include <limits.h>
#include "mjpeg_types.h"
#include "mjpeg_logging.h"
#include "global.h"
#include "simd.h"
#include "fastintfns.h"

#include "ratectl.hh"

/* private prototypes */

static void calc_actj ( Picture &picture, double *act_sum, double *var_sum);

//#ifdef JUNK
#if 0
/*Constant bit-rate rate control variables */
/* X's global complexity (Chi! not X!) measures.
   Actually: average quantisation * bits allocated in *previous* frame
   N.b. the choice of measure is *not* arbitrary.  The feedback bit rate
   control gets horribly messed up if it is *not* proportionate to bit demand
   i.e. bits used scaled for quantisation.
   d's  virtual reciever buffer fullness
   r - Rate control feedback gain (in bits/frame)  I.e if the "virtual
   buffer" is x% full and
*/

double Xi = 0, Xp = 0, Xb = 0;
int r;
int	d0i = 0, d0pb = 0, d0p = 0, d0b = 0;


/* R - Remaining bits available in the next one second period.
   T - Target bits for current frame q
   d - Current d for quantisation purposes updated using
   scaled difference of target bit usage and actual usage
   NOTE: We use double's for these as 48 bits will suffice and
   there's some tricky float arithmetic required/

*/
static int32_t R;
static int32_t per_pict_bits;
static int     fields_in_gop;
static double  field_rate;
static int     fields_per_pict;
static int32_t buffer_variation = 0;
static int d;
static int32_t T;
static int64_t bits_transported = 0LL;
static int64_t bits_used = 0LL;
static int32_t gop_buffer_correction;
static int32_t pict_base_bits;

static int32_t I_pict_base_bits;
static int32_t B_pict_base_bits;
static int32_t P_pict_base_bits;

/* bitcnt_EOP - Position in generated bit-stream for latest
				end-of-picture Comparing these values with the
				bit-stream position for when the picture is due to be
				displayed allows us to see what the vbv buffer is up
				to.

*/

static int64_t bitcnt_EOP = 0;
static int frame_overshoot_margin = 0;
static int undershoot_carry;
static double overshoot_gain;

/*
  actsum - Total activity (sum block variances) in frame
  actcovered - Activity macroblocks so far quantised (used to
               fine tune quantisation to avoid starving highly
			   active blocks appearing late in frame...) UNUSED
  avg_act - Current average activity...
*/
static double actsum;
static double actcovered;
static double sum_avg_act = 0.0;
static double avg_act;
static double avg_var;
static double sum_avg_var = 0.0;
static double sum_avg_quant = 0.0;

static int Ni, Np, Nb;
static int64_t S;

/* Note: eventually we may wish to tweak these to suit image content */
static double Ki;    	/* Down-scaling of I/B/P-frame complexity */
static double Kb;	    /* relative to others in bit-allocation   */
static double Kp;	    /* calculations.  We only need 2 but have all
							   3 for readability */


static int min_d,max_d;
static int min_q, max_q;

static double avg_KI = 5.0;	      /* TODO: These values guesstimates        */
static double avg_KB = 10.0*2.0;  /* for MPEG-1, may need tuning for MPEG-2	*/
static double avg_KP = 10.0*2.0;
#define K_AVG_WINDOW_I 4.0
#define K_AVG_WINDOW_P   10.0
#define K_AVG_WINDOW_B   20.0
static double bits_per_mb;

static double sum_vbuf_Q = 0.0;
static bool fast_tune = false;
static bool first_gop = true;
static bool first_B = false;
static bool first_P = false;
static bool first_I = false;

/* Scaling and clipping of quantisation factors
   One floating point used in predictive calculations.
   The second integerises the results of the first for use in the
   actual stream...
*/
#endif

static double scale_quantf( int q_scale_type, double quant )
{
	double quantf;

	if ( q_scale_type )
	{
		int iquantl, iquanth;
		double wl, wh;

		/* BUG TODO: This should interpolate the table... */
		wh = quant-floor(quant);
		wl = 1.0 - wh;
		iquantl = (int) floor(quant);
		iquanth = iquantl+1;
		/* clip to legal (linear) range */
		if (iquantl<1)
		{
			iquantl = 1;
			iquanth = 1;
		}

		if (iquantl>111)
		{
			iquantl = 112;
			iquanth = 112;
		}

		quantf = (double)
		  wl * (double)non_linear_mquant_table[map_non_linear_mquant[iquantl]]
			+
		  wh * (double)non_linear_mquant_table[map_non_linear_mquant[iquanth]]
			;
	}
	else
	{
		/* clip mquant to legal (linear) range */
		quantf = quant;
		if (quantf<2.0)
			quantf = 2;
		if (quantf>62.0)
			quantf = 62.0;
	}
	return quantf;
}


int scale_quant( int q_scale_type , double quant )
{
	int iquant;

	if ( q_scale_type  )
	{
		iquant = (int) floor(quant+0.5);

		/* clip mquant to legal (linear) range */
		if (iquant<1)
			iquant = 1;
		if (iquant>112)
			iquant = 112;

		iquant =
			non_linear_mquant_table[map_non_linear_mquant[iquant]];
	}
	else
	{
		/* clip mquant to legal (linear) range */
		iquant = (int)floor(quant+0.5);
		if (iquant<2)
			iquant = 2;
		if (iquant>62)
			iquant = 62;
		iquant = (iquant/2)*2; // Must be *even*
	}
	return iquant;
}

double inv_scale_quant( int q_scale_type, int raw_code )
{
	int i;
	if( q_scale_type )
	{
		i = 112;
		while( 1 < i && map_non_linear_mquant[i] != raw_code )
			--i;
		return ((double)i);
	}
	else
		return ((double)raw_code);
}

/*****************************
 *
 * On-the-fly rate controller.  The constructor sets up the initial
 * control and estimator parameter values to values that experience
 * suggest make sense.  All the important ones are dynamically
 * tuned anyway so these values are not too critical.
 *
 ****************************/
OnTheFlyRateCtl::OnTheFlyRateCtl()
{

	reset();

}
void OnTheFlyRateCtl::reset(void )
{
	buffer_variation = 0;
	bits_transported = 0;
	bits_used = 0;
	prev_bitcount = 0;
	bitcnt_EOP = 0;
	frame_overshoot_margin = 0;
	sum_avg_act = 0.0;
	sum_avg_var = 0.0;

	/* TODO: These values should are really MPEG-1/2 and material type
	   dependent.  The encoder should probably run over the first 100
	   frames or so look-ahead to tune theses dynamically before doing
	   real encoding... alternative a config file should  be written!
	*/

	avg_KI = 5.0;
	avg_KB = 10.0*2.0;
	avg_KP = 10.0*2.0;
	sum_avg_quant = 0.0;

}

/*********************
 *
 * Initialise rate control parameters
 * params:  reinit - Rate control is being re-initialised during the middle
 *                   of a run.  Don't reset adaptive parameters.
 ********************/

void OnTheFlyRateCtl::InitSeq(bool reinit)
{
	double init_quant;
	/* If its stills with a size we have to hit then make the
	   guesstimates of for initial quantisation pessimistic...
	*/
	bits_transported = bits_used = 0;
	field_rate = 2*ctl->decode_frame_rate;
	fields_per_pict = opt->fieldpic ? 1 : 2;
	if( opt->still_size > 0 )
	{
		avg_KI *= 1.5;
		per_pict_bits = opt->still_size * 8;
		R = opt->still_size * 8;
	}
	else
	{
		per_pict_bits =
			static_cast<int32_t>(opt->fieldpic
								 ? opt->bit_rate / field_rate
								 : opt->bit_rate / ctl->decode_frame_rate
				);
		R = static_cast<int32_t>(opt->bit_rate);
	}

	/* Everything else already set or adaptive */
	if( reinit )
		return;

	first_gop = true;

	/* Calculate reasonable margins for variation in the decoder
	   buffer.  We assume that having less than 5 frame intervals
	   worth buffered is cutting it fine for avoiding under-runs.

	   The gain values represent the fraction of the under/over shoot
	   to be recovered during one second.  Gain is decreased if the
	   buffer margin is large, gain is higher for avoiding overshoot.

	   Currently, for a 1-frame sized margin gain is set to recover
	   an undershoot in half a second
	*/
	if( opt->still_size > 0 )
	{
		undershoot_carry = 0;
		overshoot_gain = 1.0;
	}
	else
	{
		int buffer_safe = 3 * per_pict_bits ;
		undershoot_carry = (ctl->video_buffer_size - buffer_safe)/6;
		if( undershoot_carry < 0 )
                {
                        printf("Buffer %d buffer_size %d\n",buffer_safe,ctl->video_buffer_size);
                        printf("Avg bitrate : %d  framerate %d\n",opt->bit_rate,ctl->decode_frame_rate);
			mjpeg_error_exit1("Rate control can't cope with a video buffer smaller 4 frame intervals");
                }
		overshoot_gain =  opt->bit_rate / (ctl->video_buffer_size-buffer_safe);
	}
	bits_per_mb = (double)opt->bit_rate / (mb_per_pict);

	/*
	  Reaction paramer - i.e. quantisation feedback gain relative
	  to bit over/undershoot.
	  For normal frames it is fairly modest as we can compensate
	  over multiple frames and can average out variations in image
	  complexity.

	  For stills we set it a higher so corrections take place
	  more rapidly *within* a single frame.
	*/
	if( opt->still_size > 0 )
		r = (int)floor(2.0*opt->bit_rate/ctl->decode_frame_rate);
	else
		r = (int)floor(4.0*opt->bit_rate/ctl->decode_frame_rate);


	/* Set the virtual buffers for per-frame rate control feedback to
	   values corresponding to the quantisation floor (if specified)
	   or a "reasonable" quantisation (6.0) if not.
	*/

	init_quant = (ctl->quant_floor > 0.0 ? ctl->quant_floor : 6.0);
	d0p = d0b = d0pb = d0i = static_cast<int>(init_quant * r / 62.0);

	next_ip_delay = 0.0;
	decoding_time = 0.0;


#ifdef OUTPUT_STAT
	fprintf(statfile,"\nrate control: sequence initialization\n");
	fprintf(statfile,
			" initial global complexity measures (I,P,B): Xi=%.0f, Xp=%.0f, Xb=%.0f\n",
			Xi, Xp, Xb);
	fprintf(statfile," reaction parameter: r=%d\n", r);
	fprintf(statfile,
			" initial virtual buffer fullness (I,P,B): d0i=%d, d0pb=%d",
			d0i, d0pb);
#endif


}


void OnTheFlyRateCtl::InitGOP( int np, int nb)
{
	Np = opt->fieldpic ? 2*np+1 : 2*np;
	Nb = opt->fieldpic ? 2*nb : 2*nb;
	Ni = opt->fieldpic ? 1 : 2;
	fields_in_gop = Ni + Nb + Np;

	/*
	  At the start of a GOP before any frames have gone the
	  actual buffer state represents a long term average. Any
	  undershoot due to the I_frame of the previous GOP
	  should by now have been caught up.
	*/

	gop_buffer_correction = 0;

	/* Each still is encoded independently so we reset rate control
	   for each one.  They're all I-frames so each stills is a GOP too.
	*/

	if( first_gop || opt->still_size > 0)
	{
		mjpeg_debug( "FIRST GOP INIT");
		fast_tune = true;
		first_I = first_B = first_P = true;
		first_gop = false;
		I_pict_base_bits = per_pict_bits;
		B_pict_base_bits = per_pict_bits;
		P_pict_base_bits = per_pict_bits;
	}
	else
	{
		double recovery_fraction = field_rate/(overshoot_gain * fields_in_gop);
		double recovery_gain =
			recovery_fraction > 1.0 ? 1.0 : overshoot_gain * recovery_fraction;
		int available_bits =
			static_cast<int>( (opt->bit_rate+buffer_variation*recovery_gain)
							  * fields_in_gop/field_rate);
		double Xsum = Ni*Xi+Np*Xp+Nb*Xb;
		mjpeg_debug( "REST GOP INIT" );
		I_pict_base_bits = (int32_t)(fields_per_pict*available_bits*Xi/Xsum);
		P_pict_base_bits = (int32_t)(fields_per_pict*available_bits*Xp/Xsum);
		B_pict_base_bits = (int32_t)(fields_per_pict*available_bits*Xb/Xsum);
		fast_tune = 0;

	}

#ifdef OUTPUT_STAT
	fprintf(statfile,"\nrate control: new group of pictures (GOP)\n");
	fprintf(statfile," target number of bits for GOP: R=%.0f\n",R);
	fprintf(statfile," number of P pictures in GOP: Np=%d\n",Np);
	fprintf(statfile," number of B pictures in GOP: Nb=%d\n",Nb);
#endif
}


/* Step 1: compute target bits for current picture being coded */
void OnTheFlyRateCtl::InitPict(Picture &picture)
{
	double avg_K = 0.0;
	double target_Q;
	double current_Q;
	double Si, Sp, Sb;
	int available_bits;
	double Xsum,varsum;
	bool no_avg_K = false;

	/* TODO: A.Stevens  Nov 2000 - This modification needs testing visually.

	   Weird.  The original code used the average activity of the
	   *previous* frame as the basis for quantisation calculations for
	   rather than the activity in the *current* frame.  That *has* to
	   be a bad idea..., surely, here we try to be smarter by using the
	   current values and keeping track of how much of the frames
	   activitity has been covered as we go along.

	   We also guesstimate the relationship between  (sum
	   of DCT coefficients) and actual quantisation weighted activty.
	   We use this to try to predict the activity of each frame.
	*/

	calc_actj(picture, &actsum, &varsum );
	avg_act = actsum/(double)(mb_per_pict);
	avg_var = varsum/(double)(mb_per_pict);
	mjpeg_info("mb_per_pict: %d sum_avg : %f sum_avg_var : %f\n",
						mb_per_pict,avg_act,avg_var);
	sum_avg_act += avg_act;
	sum_avg_var += avg_var;
	actcovered = 0.0;
	sum_vbuf_Q = 0.0;


	/* Allocate target bits for frame based on frames numbers in GOP
	   weighted by:
	   - global complexity averages
	   - predicted activity measures
	   - fixed type based weightings

	   T = (Nx * Xx/Kx) / Sigma_j (Nj * Xj / Kj)

	   N.b. B frames are an exception as there is *no* predictive
	   element in their bit-allocations.  The reason this is done is
	   that highly active B frames are inevitably the result of
	   transients and/or scene changes.  Psycho-visual considerations
	   suggest there's no point rendering sudden transients
	   terribly well as they're not percieved accurately anyway.

	   In the case of scene changes similar considerations apply.  In
	   this case also we want to save bits for the next I or P frame
	   where they will help improve other frames too.
	   TODO: Experiment with *inverse* predictive correction for B-frames
	   and turning off active-block boosting for B-frames.

	   Note that we have to calulate per-frame bits by scaling the one-second
	   bit-pool a one-GOP bit-pool.
	*/


	if( opt->still_size > 0 )
		available_bits = per_pict_bits;
	else
	{
		int feedback_correction =
			static_cast<int>( fast_tune
							  ?	buffer_variation * overshoot_gain
							  : (buffer_variation+gop_buffer_correction)
							    * overshoot_gain
				);
		available_bits =
			static_cast<int>( (opt->bit_rate+feedback_correction)
							  * fields_in_gop/field_rate
							  );
	}

	min_q = min_d = INT_MAX;
	max_q = max_d = INT_MIN;
    Xsum = Ni*Xi+Np*Xp+Nb*Xb;
	switch (picture.pict_type)
	{
	case I_TYPE:

		/* There is little reason to rely on the *last* I-frame as
		   they're not closely related.  The slow correction of K
		   should be enough to fine-tune.
		*/

		d = d0i;
		avg_K = avg_KI;
		Si = avg_K*actsum;
		no_avg_K = first_I;
		if( first_I )
		{
			T = (int32_t)(fields_per_pict*available_bits/(Ni+(Np/1.7)+Nb/(2.0*1.7)));
		}
		else
		{
			T = (int32_t)(fields_per_pict*available_bits*Si/Xsum);
		}
		pict_base_bits = I_pict_base_bits;
		break;
	case P_TYPE:
		d = d0p;
		avg_K = avg_KP;
		Sp = (2*Xp + avg_K * actsum)/3.0;	 /* Damp P-frame response a little
											as really big jumps will probably
											make avg_K inaccurate */
		no_avg_K = first_P;
		if( first_P )
		{
			T = (int32_t)(fields_per_pict*available_bits/(Np+Nb/2.0));
		}
		else
		{
			T = (int32_t)(fields_per_pict*available_bits*Sp/Xsum);
		}
		pict_base_bits = P_pict_base_bits;
		break;
	case B_TYPE:
		d = d0b;
		avg_K = avg_KB;
		Sb = Xb;				/* We don't want B-frames to be too
								   responsive...*/
		no_avg_K = first_B;
		if( first_B )
		{
			T =  (int32_t)(fields_per_pict*available_bits/(Nb+Np*2.0));
		}
		else
		{
			T =  (int32_t)(fields_per_pict*available_bits*Sb/Xsum);
		}
		pict_base_bits = B_pict_base_bits;

		break;
	}

	/*
	   If we're fed a sequences of identical or near-identical images
	   we can get actually get allocations for frames that exceed
	   the video buffer size!  This of course won't work so we arbitrarily
	   limit any individual frame to 3/4's of the buffer.
	*/

	T = intmin( T, ctl->video_buffer_size*3/4 );

	mjpeg_debug( "I=%d P=%d B=%d", I_pict_base_bits, P_pict_base_bits, B_pict_base_bits );
	mjpeg_debug( "T=%05d A=%06d D=%06d (%06d) ", (int)T/8, (int)available_bits/8, (int)buffer_variation/8, (int)(buffer_variation + gop_buffer_correction)/8 );


	/*
	   To account for the wildly different sizes of frames
	   we compute a correction to the current instantaneous
	   buffer state that accounts for the fact that all other
	   thing being equal buffer to go down a lot after the I-frame
	   decode but fill up again through the B and P frames.

	   For this we use the base bit allocations of the picture's
	   "pict_base_bits" which will pretty accurately add up to a
	   GOP-length's of bits not the more dynamic predictive T target
	   bit-allocation (which *won't* add up very well).
	*/

	mjpeg_debug( "PBB=%d PPB=%d", pict_base_bits, per_pict_bits );
	gop_buffer_correction += (pict_base_bits-per_pict_bits);


	/* Undershot bits have been "returned" via R */
	if( d < 0 )
		d = 0;

	/* We don't let the target volume get absurdly low as it makes some
	   of the prediction maths ill-condtioned.  At these levels quantisation
	   is always minimum anyway
	*/
	T = intmax( T, 4000 );

	if( opt->still_size > 0 && opt->vbv_buffer_still_size )
	{
		/* If stills size must match then target low to ensure no
		   overshoot.
		*/
		mjpeg_info( "Setting VCD HR still overshoot margin to %d bytes", T/(16*8) );
		frame_overshoot_margin = T/16;
		T -= frame_overshoot_margin;
	}


	current_Q = scale_quant(picture.q_scale_type,62.0*d / r);
	if( no_avg_K )
		target_Q = current_Q;
	else
		target_Q = scale_quantf(picture.q_scale_type,
								avg_K * avg_act *(mb_per_pict) / T);

	picture.avg_act = avg_act;
	picture.sum_avg_act = sum_avg_act;

	/*  If guesstimated target quantisation requirement is much different
		than that suggested by the virtual buffer we reset the virtual
		buffer to set the guesstimate as initial quantisation.

		This effectively speeds up rate control response when sudden
		changes in content occur
	*/


	if ( 62.0*d / r  < target_Q / 2.0  )
	{
		d = (int) (target_Q  * r / (62.0));
	}

	if ( 62.0*d / r  > target_Q * 2.0  )
	{
		d = (int) (d+(target_Q  * r / 62.0))/2;
	}

	S = bitcount();


#ifdef OUTPUT_STAT
	fprintf(statfile,"\nrate control: start of picture\n");
	fprintf(statfile," target number of bits: T=%.0f\n",T/8);
#endif
}



static void calc_actj(Picture &picture, double *act_sum, double *var_sum)
{
	int i,j,k,l;
	double actj,sum;
	double varsum;
	uint16_t *i_q_mat;
	int blksum;
	sum = 0.0;
	varsum = 0.0;
	k = 0;

	for (j=0; j<opt->enc_height2; j+=16)
		for (i=0; i<opt->enc_width; i+=16)
		{
			/* A.Stevens Jul 2000 Luminance variance *has* to be a
			   rotten measure of how active a block in terms of bits
			   needed to code a lossless DCT.  E.g. a half-white
			   half-black block has a maximal variance but pretty
			   small DCT coefficients.

			   So.... instead of luminance variance as used in the
			   original we use the absolute sum of DCT coefficients as
			   our block activity measure.  */

			varsum += (double)picture.mbinfo[k].final_me.var;
			if( picture.mbinfo[k].final_me.mb_type  & MB_INTRA )
			{
				i_q_mat = i_intra_q;
				/* Compensate for the wholly disproprotionate weight
				 of the DC coefficients.  Shold produce more sensible
				 results...  yes... it *is* an mostly empirically derived
				 fudge factor ;-)
				*/
				blksum =  -80*COEFFSUM_SCALE;
			}
			else
			{
				i_q_mat = i_inter_q;
				blksum = 0;
			}
			/* It takes some bits to code even an entirely zero block...
			   It also makes a lot of calculations a lot better conditioned
			   if it can be guaranteed that activity is always distinctly
			   non-zero.
			 */


			for( l = 0; l < 6; ++l )
				blksum +=
					(*pquant_weight_coeff_sum)
					( picture.mbinfo[k].RawDCTblocks()[l], i_q_mat ) ;
			actj = (double)blksum / (double)COEFFSUM_SCALE;
			if( actj < 12.0 )
				actj = 12.0;

			picture.mbinfo[k].act = (double)actj;
			sum += (double)actj;
			++k;
		}
	*act_sum = sum;
	*var_sum = varsum;
}

/*
 * Update rate-controls statistics after pictures has ended..
 *
 */
void OnTheFlyRateCtl::UpdatePict(Picture &picture)
{
	double X;
	double K;
	double AQ;
	int32_t AP;		/* Actual (including padding) picture bit counts */
	int    i;
	int    Qsum;
	int frame_overshoot;

	AP = bitcount() - S;
	frame_overshoot = (int)AP-(int)T;

	/* For the virtual buffers for quantisation feedback it is the
	   actual under/overshoot *including* padding.  Otherwise the
	   buffers go zero.
	*/
	d += frame_overshoot;

	/* Warn if it looks like we've busted the safety margins in stills
	   size specification.  Adjust padding to account for safety
	   margin if we're padding to suit stills whose size has to be
	   specified in advance in vbv_buffer_size.
	*/
	picture.pad = 0;

	if( opt->still_size > 0 && opt->vbv_buffer_still_size)
	{
		int padding_bytes;
		if( frame_overshoot > frame_overshoot_margin )
		{
			mjpeg_warn( "Rate overshoot: VCD hi-res still %d bytes too large! ",
						((int)AP)/8-opt->still_size);
		}

		//
		// Aim for an actual size squarely in the middle of the 2048
		// byte granuality of the still_size coding.  This gives a
		// safety margin for headers etc.
		//
		frame_overshoot = frame_overshoot - frame_overshoot_margin;
		if( frame_overshoot < -2048*8 )
			frame_overshoot += 1024*8;

		padding_bytes = -frame_overshoot/8;
		if( padding_bytes > 0 )
		{
			mjpeg_debug( "Padding still to size: %d extra bytes", padding_bytes );
			picture.pad = 1;
			alignbits();
			// MEANX -> 2 pass
			if(!rateCtlDisablePadding)
			{
				for( i = 0; i < padding_bytes/2; ++i )
				{
					putbits(0, 16);
				}
			}
			// /MEANX
		}
		AP += padding_bytes*8 ;			/* total # of bits in picture */
		frame_overshoot += padding_bytes*8;
	}


	/*
	  Compute the estimate of the current decoder buffer state.  We
	  use this to feedback-correct the available bit-pool with a
	  fraction of the current buffer state estimate.  If we're ahead
	  of the game we allow a small increase in the pool.  If we
	  dropping towards a dangerously low buffer we decrease the pool
	  (rather more vigorously).

	  Note that since we cannot hold more than a buffer-full if we have
	  a positive buffer_variation in CBR we assume it was padded away
	  and in VBR we assume we only sent until the buffer was full.
	*/


	bits_used += (bitcount()-prev_bitcount);
	prev_bitcount = bitcount();
	bits_transported += per_pict_bits;
	mjpeg_debug( "TR=%llu USD=%llu ", bits_transported/8, bits_used/8);
	buffer_variation  = (int32_t)(bits_transported - bits_used);

	if( buffer_variation > 0 )
	{
		if( ctl->quant_floor > 0 )
		{
			bits_transported = bits_used;
			buffer_variation = 0;
		}
		else if( buffer_variation > undershoot_carry )
		{
			bits_used = bits_transported + undershoot_carry;
			buffer_variation = undershoot_carry;
		}
	}


	Qsum = 0;
	for( i = 0; i < mb_per_pict; ++i )
	{
		Qsum += picture.mbinfo[i].mquant;
	}


    /* AQ is the average Quantisation of the block.
	   Its only used for stats display as the integerisation
	   of the quantisation value makes it rather coarse for use in
	   estimating bit-demand */
	AQ = (double)Qsum/(double)mb_per_pict;
	sum_avg_quant += AQ;

	/* X (Chi - Complexity!) is an estimate of "bit-demand" for the
	frame.  I.e. how many bits it would need to be encoded without
	quantisation.  It is used in adaptively allocating bits to busy
	frames. It is simply calculated as bits actually used times
	average target (not rounded!) quantisation.

	K is a running estimate of how bit-demand relates to frame
	activity - bits demand per activity it is used to allow
	prediction of quantisation needed to hit a bit-allocation.
	*/

	X = AP  * AQ; //sum_vbuf_Q /  mb_per_pict;
	K = X / actsum;
	picture.AQ = AQ;
	picture.SQ = sum_avg_quant;

	mjpeg_debug( "D=%d R=%d GC=%d",
				 buffer_variation/8,
				 (int)R/8,
				 gop_buffer_correction/8  );

	/* Xi are used as a guesstimate of *typical* frame activities
	   based on the past.  Thus we don't want anomalous outliers due
	   to scene changes swinging things too much (this is handled by
	   the predictive complexity measure stuff) so we use moving
	   averages.  The weightings are intended so all 3 averages have
	   similar real-time decay periods based on an assumption of
	   20-30Hz frame rates.
	*/

	switch (picture.pict_type)
	{
	case I_TYPE:
		d0i = d;
		if( first_I )
		{
			Xi = X;
			avg_KI = K;
			first_I = 0;
		}
		else
		{
			avg_KI = (K + avg_KI * K_AVG_WINDOW_I) / (K_AVG_WINDOW_I+1.0) ;
			Xi = (X + K_AVG_WINDOW_I*Xi)/(K_AVG_WINDOW_I+1.0);

			/* To handle longer sequences with little picture content
		   where I, B and P frames are of unusually similar size we
		   insist I frames assumed to be at least of the same
		   complexity as two B and a P frame.  This ensures that
		   *after* a low picture content sequence there won't be a
		   grossly under-estimated I-frame size that hasn't been
		   allowed for in the buffer management.
			*/
			Xi = Xi < Xp+2.0*Xb ? Xp+2.0*Xb  : Xi;
		}
		break;
	case P_TYPE:
		d0p = d;
		if( first_P )
		{
			Xp = X;
			avg_KP = K;
			first_P = 0;
		}
		else
		{
			avg_KP = (K + avg_KP * K_AVG_WINDOW_P) / (K_AVG_WINDOW_P+1.0) ;
			if( fast_tune )
				Xp = (X+Xp*2.0)/3.0;
			else
				Xp = (X + Xp*K_AVG_WINDOW_P)/(K_AVG_WINDOW_P+1.0);
		}
		break;
	case B_TYPE:
		d0b = d;
		if( first_B )
		{
			Xb = X;
			avg_KB = K;
			first_B = 0;
		}
		else
		{
			avg_KB = (K + avg_KB * K_AVG_WINDOW_B) / (K_AVG_WINDOW_B+1.0) ;
			if( fast_tune )
			{
				Xb = (X + Xb * 3.0) / 4.0;
			}
			else
				Xb = (X + Xb*K_AVG_WINDOW_B)/(K_AVG_WINDOW_B+1.0);
		}
		break;
	}

	VbvEndOfPict(picture);

#ifdef OUTPUT_STAT
	fprintf(statfile,"\nrate control: end of picture\n");
	fprintf(statfile," actual number of bits: S=%lld\n",S);
	fprintf(statfile," average quantization parameter AQ=%.1f\n",
			(double)AQ);
	fprintf(statfile," remaining number of bits in GOP: R=%.0f\n",R);
	fprintf(statfile,
			" global complexity measures (I,P,B): Xi=%.0f, Xp=%.0f, Xb=%.0f\n",
			Xi, Xp, Xb);
	fprintf(statfile,
			" virtual buffer fullness (I,PB): d0i=%d, d0b=%d\n",
			d0i, d0b);
	fprintf(statfile," remaining number of P pictures in GOP: Np=%d\n",Np);
	fprintf(statfile," remaining number of B pictures in GOP: Nb=%d\n",Nb);
	fprintf(statfile," average activity: avg_act=%.1f \n", avg_act );
#endif

}

/* compute initial quantization stepsize (at the beginning of picture)
   ctl->quant_floor != 0 is the VBR case where we set a bitrate as a (high)
   maximum and then put a floor on quantisation to achieve a reasonable
   overall size.
 */
static int init_quant;

int OnTheFlyRateCtl::InitialMacroBlockQuant(Picture &picture)
{

	int mquant = scale_quant( picture.q_scale_type, d*62.0/r );
	init_quant = mquant = intmax(mquant, static_cast<int>(ctl->quant_floor));

/*
  fprintf(statfile,"rc_start_mb:\n");
  fprintf(statfile,"mquant=%d\n",mquant);
*/
	return mquant;
}


/*************
 *
 * SelectQuantization - select a quantisation for the current
 * macroblock based on the fullness of the virtual decoder buffer.
 *
 ************/

int OnTheFlyRateCtl::MacroBlockQuant( const MacroBlock &mb )
{
	int mquant;
	int lum_variance = mb.BaseLumVariance();
	double act  = mb.Activity();
	const Picture &picture = mb.ParentPicture();
	/* A.Stevens 2000 : we measure how much *information* (total activity)
	   has been covered and aim to release bits in proportion.

	   We keep track of a virtual buffer that catches the difference
	   between the bits allocated and the bits we actually used.  The
	   fullness of this buffer controls quantisation.

	*/

	/* Guesstimate a virtual buffer fullness based on
	   bits used vs. bits in proportion to activity encoded
	*/

	double dj = ((double)d) +
		((double)(bitcount()-S) - actcovered * ((double)T) / actsum);


	/* scale against dynamic range of mquant and the bits/picture
	   count.  ctl->quant_floor != 0.0 is the VBR case where we set a
	   bitrate as a (high) maximum and then put a floor on
	   quantisation to achieve a reasonable overall size.  Not that
	   this *is* baseline quantisation.  Not adjust for local
	   activity.  Otherwise we end up blurring active
	   macroblocks. Silly in a VBR context.
	*/

	double Qj = dj*62.0/r;
	Qj = (Qj > ctl->quant_floor) ? Qj : ctl->quant_floor;

	/*  Heuristic: We decrease quantisation for macroblocks
		with markedly low luminace variance.  This helps make
		gentle gradients (e.g. smooth backgrounds) look better at
		(hopefully) small additonal cost  in coding bits
	*/

	double act_boost;
#ifdef OLD_QUANTISATION_STEARING
	double N_act =  ( act < avg_act || picture.pict_type == B_TYPE ) ?
		1.0 :
		(ctl->act_boost*act +  avg_act)/(act + ctl->act_boost*avg_act);
	act_boost = 1.0/N_act;
#else
	if( lum_variance < ctl->boost_var_ceil )
	{
		if( lum_variance < ctl->boost_var_ceil/2)
			act_boost = ctl->act_boost;
		else
		{
			double max_boost_var = ctl->boost_var_ceil/2;
			double above_max_boost =
				(static_cast<double>(lum_variance)-max_boost_var)
				/ max_boost_var;
			act_boost = 1.0 + (ctl->act_boost-1.0) * (1.0-above_max_boost);
		}
	}
	else
		act_boost = 1.0;

#endif
	sum_vbuf_Q += scale_quantf(picture.q_scale_type,Qj/act_boost);
	mquant = scale_quant(picture.q_scale_type,Qj/act_boost) ;

	/* Update activity covered */

	actcovered += act;

	return mquant;
#ifdef OUTPUT_STAT
/*
  fprintf(statfile,"MQ(%d): ",j);
  fprintf(statfile,"dj=%.0f, Qj=%1.1f, actj=3.1%f, N_actj=1.1%f, mquant=%03d\n",
  dj,Qj,actj,N_actj,mquant);
*/
	//picture.mbinfo[j].N_act = N_actj;
#endif
}

/* VBV calculations
 *
 * generates warnings if underflow or overflow occurs
 */

/* vbv_end_of_picture
 *
 * - has to be called directly after writing picture_data()
 * - needed for accurate VBV buffer overflow calculation
 * - assumes there is no byte stuffing prior to the next start code
 *
 * Note correction for bytes that will be stuffed away in the eventual CBR
 * bit-stream.
 */

void OnTheFlyRateCtl::VbvEndOfPict(Picture &picture)
{
	bitcnt_EOP = (bitcount()) - BITCOUNT_OFFSET;

}

/* calc_vbv_delay
 *
 * has to be called directly after writing the picture start code, the
 * reference point for vbv_delay
 *
 * A.Stevens 2000:
 * Actually we call it just before the start code is written, but anyone
 * who thinks 32 bits +/- in all these other approximations matters is fooling
 * themselves.
 */

void OnTheFlyRateCtl::CalcVbvDelay(Picture &picture)
{

	/* number of 1/90000 s ticks until next picture is to be decoded */
	if (picture.pict_type == B_TYPE)
	{
		if (opt->prog_seq)
		{
			if (!picture.repeatfirst)
				picture_delay = 90000.0/opt->frame_rate; /* 1 frame */
			else
			{
				if (!picture.topfirst)
					picture_delay = 90000.0*2.0/opt->frame_rate; /* 2 frames */
				else
					picture_delay = 90000.0*3.0/opt->frame_rate; /* 3 frames */
			}
		}
		else
		{
			/* interlaced */
			if (opt->fieldpic)
				picture_delay = 90000.0/(2.0*opt->frame_rate); /* 1 field */
			else
			{
				if (!picture.repeatfirst)
					picture_delay = 90000.0*2.0/(2.0*opt->frame_rate); /* 2 flds */
				else
					picture_delay = 90000.0*3.0/(2.0*opt->frame_rate); /* 3 flds */
			}
		}
	}
	else
	{
		/* I or P picture */
		if (opt->fieldpic)
		{
			if(picture.topfirst && (picture.pict_struct==TOP_FIELD))
			{
				/* first field */
				picture_delay = 90000.0/(2.0*opt->frame_rate);
			}
			else
			{
				/* second field */
				/* take frame reordering delay into account */
				picture_delay = next_ip_delay - 90000.0/(2.0*opt->frame_rate);
			}
		}
		else
		{
			/* frame picture */
			/* take frame reordering delay into account*/
			picture_delay = next_ip_delay;
		}

		if (!opt->fieldpic ||
			picture.topfirst!=(picture.pict_struct==TOP_FIELD))
		{
			/* frame picture or second field */
			if (opt->prog_seq)
			{
				if (!picture.repeatfirst)
					next_ip_delay = 90000.0/opt->frame_rate;
				else
				{
					if (!picture.topfirst)
						next_ip_delay = 90000.0*2.0/opt->frame_rate;
					else
						next_ip_delay = 90000.0*3.0/opt->frame_rate;
				}
			}
			else
			{
				if (opt->fieldpic)
					next_ip_delay = 90000.0/(2.0*opt->frame_rate);
				else
				{
					if (!picture.repeatfirst)
						next_ip_delay = 90000.0*2.0/(2.0*opt->frame_rate);
					else
						next_ip_delay = 90000.0*3.0/(2.0*opt->frame_rate);
				}
			}
		}
	}

	if (decoding_time==0.0)
	{
		/* first call of calc_vbv_delay */
		/* we start with a 7/8 filled VBV buffer (12.5% back-off) */
		picture_delay = ((opt->vbv_buffer_size*7)/8)*90000.0/opt->bit_rate;
		if (opt->fieldpic)
			next_ip_delay = (int)(90000.0/opt->frame_rate+0.5);
	}

	/* VBV checks */

	/*
	   TODO: This is currently disabled because it is hopeless wrong
	   most of the time. It generates 20 warnings for frames with small
	   predecessors (small bitcnt_EOP) that in reality would be padded
	   away by the multiplexer for every realistic warning for an
	   oversize packet.
	*/

#ifdef CRIES_WOLF

	/* check for underflow (previous picture).
	*/
	if (!opt->low_delay && (decoding_time < (double)bitcnt_EOP*90000.0/opt->bit_rate))
	{
		/* picture not completely in buffer at intended decoding time */
		mjpeg_warn("vbv_delay underflow frame %d (target=%.1f, actual=%.1f)",
				   frame_num-1, decoding_time, bitcnt_EOP*90000.0/opt->bit_rate);
	}


	/* when to decode current frame */
	decoding_time += picture_delay;


	/* check for overflow (current picture).  Unless verbose warn
	   only if overflow must be at least in part due to an oversize
	   frame (rather than undersize predecessor).
	   	*/

	picture.vbv_delay = (int)(decoding_time - ((double)bitcnt_EOP)*90000.0/bit_rate);

	if ( decoding_time * ((double)bit_rate  / 90000.0) - ((double)bitcnt_EOP)
		> vbv_buffer_size )
	{
		double oversize = opt->vbv_buffer_size -
			(decoding_time / 90000.0 * bit_rate - (double)(bitcnt_EOP+frame_undershoot));
		if(!quiet || oversize > 0.0  )
			mjpeg_warn("vbv_delay overflow frame %d - %f.0 bytes!",
					   frame_num,
					   oversize / 8.0
				);
	}


#ifdef OUTPUT_STAT
	fprintf(statfile,
			"\nvbv_delay=%d (bitcount=%lld, decoding_time=%.2f, bitcnt_EOP=%lld)\n",
			picture.vbv_delay,bitcount(),decoding_time,bitcnt_EOP);
#endif

	if (picture.vbv_delay<0)
	{
		mjpeg_warn("vbv_delay underflow: %d",picture.vbv_delay);
		picture.vbv_delay = 0;
	}



	if (picture.vbv_delay>65535)
	{
		mjpeg_warn("vbv_delay frame %d exceeds permissible range: %d",
				   frame_num, picture.vbv_delay);
		picture.vbv_delay = 65535;
	}
#else
	if( !opt->mpeg1 || ctl->quant_floor != 0 || opt->still_size > 0)
		picture.vbv_delay =  0xffff;
	else if( opt->still_size > 0 )
		picture.vbv_delay =  static_cast<int>(90000.0/opt->frame_rate/4);
#endif

}
