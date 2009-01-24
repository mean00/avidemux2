#ifndef _RATECTL_HH
#define _RATECTL_HH

/*  (C) 2003 Andrew Stevens */

/*  This is free software; you can redistribute it
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
#define K_AVG_WINDOW_I   4.0
#define K_AVG_WINDOW_P   10.0
#define	K_AVG_WINDOW_B   20.0

#include <config.h>
class MacroBlock;
// MEANX : UGLY
extern int rateCtlDisablePadding;
class RateCtl
{
public:
	virtual void InitSeq( bool reinit ) = 0;
	virtual void InitGOP( int nb, int np ) = 0;
	virtual void InitPict (Picture &picture)= 0;
	virtual void UpdatePict (Picture &picture) = 0;
	virtual int  MacroBlockQuant( const MacroBlock &mb) = 0;
	virtual int  InitialMacroBlockQuant(Picture &picture) = 0;
	virtual void VbvEndOfPict (Picture &picture) = 0;
	virtual void CalcVbvDelay (Picture &picture) = 0;
};

class OnTheFlyRateCtl : public RateCtl
{
public:
	OnTheFlyRateCtl();
	virtual void InitSeq( bool reinit );
	virtual void InitGOP( int nb, int np );
	virtual void InitPict (Picture &picture);
	virtual void UpdatePict (Picture &picture);
	virtual int  MacroBlockQuant( const MacroBlock &mb );
	virtual int  InitialMacroBlockQuant(Picture &picture);
	virtual void VbvEndOfPict (Picture &picture);
	virtual void CalcVbvDelay (Picture &picture);
	void reset(void);
private:

   /* X's measure global complexity (Chi! not X!) of frame types.
	* Actually: X = average quantisation * bits allocated in *previous* frame
	* N.b. the choice of measure is *not* arbitrary.  The feedback bit
	* rate control gets horribly messed up if it is *not* proportionate
	* to bit demand i.e. bits used scaled for quantisation.  
	* d's are virtual reciever buffer fullness 
	* r is Rate control feedback gain (in* bits/frame) 
	*/

	double Xi, Xp, Xb;
	int32_t	d0i, d0pb, d0p, d0b;
	int32_t r;
	
	/* R - Remaining bits available in the next one second period.
	   T - Target bits for current frame 
	   d - Current virtual reciever buffer fullness for quantisation
	   purposes updated using scaled difference of target bit usage
	   and actual usage
	*/
	int32_t R;
	int32_t T;
	int32_t d;

	int32_t per_pict_bits;
	int     fields_in_gop;
	double  field_rate;
	int     fields_per_pict;

	int32_t buffer_variation;
	int64_t bits_transported;
	int64_t bits_used;
	int32_t gop_buffer_correction;
	int32_t pict_base_bits;
	
	int32_t I_pict_base_bits;
	int32_t B_pict_base_bits;
	int32_t P_pict_base_bits;

    /* bitcnt_EOP - Position in generated bit-stream for latest
	   end-of-picture Comparing these values with the
	   bit-stream position for when the picture is due to be
	   displayed allows us to see what the vbv buffer is up
	   to.
	*/

	int64_t bitcnt_EOP;
	int64_t prev_bitcount;
	int frame_overshoot_margin;
	int undershoot_carry;
	double overshoot_gain;

    /*
	  actsum - Total activity (sum block variances) in frame
	  actcovered - Activity macroblocks so far quantised (used to
	  fine tune quantisation to avoid starving highly
	  active blocks appearing late in frame...) UNUSED
	  avg_act - Current average activity...
	*/
	double actsum;
	double actcovered;
	double sum_avg_act;
	double avg_act;
	double avg_var;
	double sum_avg_var;
	double sum_avg_quant;
	double sum_vbuf_Q;
	
	int Ni, Np, Nb;
	int64_t S;


	int min_d, max_d;
	int min_q, max_q;
	
	/* K's average scale factor between frame activity (which we can
	 * calculate quickly in advance and actual frame complexity X (a
	 * measure of 'bit demand').  These is estimated to allow a
	 * reasonable prediction to be made of the bit demand for
	 * calculating initial quantisation.  The constants are the window
	 * sizes used for for the sliding averages.
	 */

	double avg_KI;
	double avg_KB;
	double avg_KP;

	double bits_per_mb;
	bool fast_tune;
	bool first_gop;
	bool first_B;
	bool first_P;
	bool first_I;

	// VBV calculation data
	double picture_delay;
	double next_ip_delay; /* due to frame reordering delay */
	double decoding_time;


};


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
#endif
