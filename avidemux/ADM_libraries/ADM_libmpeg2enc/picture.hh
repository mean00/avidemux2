
/*  (C) 2000/2001/2002 Andrew Stevens */

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


#ifndef _PICTURE_HH
#define _PICTURE_HH
/* picture.hh picture class... */

#include <vector>
using namespace std;

/* Transformed per-picture data  */

typedef int MotionVecPred[2][2][2];
typedef int DC_DctPred[3];

class CodingPredictors
{
public:

    void Reset_DC_DCT_Pred()
        {
            int cc;
            for (cc=0; cc<3; cc++)
                dc_dct_pred[cc] = 0;
                
        }
    void Reset_MV_Pred()
        {
            int *base=&PMV[0][0][0];
            int v;
            for( v = 0; v < 2*2*2; ++v)
                base[v]=0;
        }

    DC_DctPred dc_dct_pred;
    MotionVecPred PMV;
    MacroBlock *prev_mb;
    int mquant_pred;
    
};

class pict_data_s
{
public:
	int decode;					/* Number of frame in stream */
	int present;				/* Number of frame in playback order */
	/* multiple-reader/single-writer channels Synchronisation  
	   sync only: no data is "read"/"written"
	 */
/*	sync_guard_t *ref_frame_completion;
	sync_guard_t *prev_frame_completion;
	sync_guard_t completion;
*/
	/* picture encoding source data  */
	uint8_t **oldorg, **neworg;	/* Images for Old and new reference picts */
	uint8_t **oldref, **newref;	/* original and reconstructed */
	uint8_t **curorg, **curref;	/* Images for current pict orginal and*/
								/* reconstructed */
	uint8_t **pred;			/* Prediction based on MC (if any) */
	int sxf, syf, sxb, syb;		/* MC search limits. */
	bool secondfield;			/* Second field of field frame */
	bool ipflag;				/* P pict in IP frame (FIELD pics only)*/

	/* picture structure (header) data */

	int temp_ref; /* temporal reference */
	int pict_type; /* picture coding type (I, P or B) */
	int vbv_delay; /* video buffering verifier delay (1/90000 seconds) */
	int forw_hor_f_code, forw_vert_f_code;
	int back_hor_f_code, back_vert_f_code; /* motion vector ranges */
	int dc_prec;				/* DC coefficient prec for intra blocks */
	int pict_struct;			/* picture structure (frame, top / bottom) */
	bool topfirst;				/* display top field first */
	bool frame_pred_dct;			/* Use only frame prediction... */
	int intravlc;				/* Intra VLC format */
	int q_scale_type;			/* Quantiser scale... */
	int altscan;				/* Alternate scan  */
	bool repeatfirst;			/* repeat first field after second field */
	bool prog_frame;				/* progressive frame */

	/* 8*8 block data, raw (unquantised) and quantised, and (eventually but
	   not yet inverse quantised */
	DCTblock *blocks;
	DCTblock *qblocks;

	/* Macroblocks of picture */
	vector<MacroBlock> mbinfo;

	/* Information for GOP start frames */
	bool gop_start;             /* GOP Start frame */
    bool closed_gop;            /* GOP is closed   */
	int nb;						/* B frames in GOP */
	int np;						/* P frames in GOP */
	bool new_seq;				/* GOP starts new sequence */

	/* Statistics... */
	int pad;
	int split;
	double AQ;
	double SQ;
	double avg_act;
	double sum_avg_act;
	/* MEANX : Average quant factor */
	int	averageQuant;

};

class RateCtl;

class Picture : public pict_data_s, public CodingPredictors
{
public:
    void PutHeadersAndEncoding( RateCtl &ratecontrol );
    void QuantiseAndPutEncoding(RateCtl &ratecontrol);
    void PutHeader(); 
private:
    void PutSliceHdr( int slice_mb_y );
    void PutMVs( MotionEst &me, bool back );
    void PutCodingExt();
};


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
#endif
