#ifndef _MACROBLOCK_HH
#define _MACROBLOCK_HH 
/* macroblock.hh macroblock class... */

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

/*  (C) 2000/2001 Andrew Stevens */

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
#include <vector>
#include "mjpeg_types.h"

using namespace std;

class Picture;


typedef int16_t DCTblock[64];

class MotionEst
{
public:
	int mb_type; /* intra/forward/backward/interpolated */
	int motion_type; /* frame/field/16x8/dual_prime */
	int MV[2][2][2]; /* motion vectors */
	int field_sel[2][2]; /* motion vertical field select */
	int dualprimeMV[2]; /* dual prime vectors */
	int var; 	/* luminance variance after motion compensation 
                   (measure of activity) */
};


/* macroblock information */
class MacroBlock
{
public:
    MacroBlock(Picture &_picture,
               const unsigned int _i,
               const unsigned int _j,
               DCTblock *_dctblocks,
               DCTblock *_qdctblocks
               ) :
        picture(&_picture),
        i(_i),
        j(_j),
        dctblocks(_dctblocks),
        qdctblocks(_qdctblocks)
        {
        }
    
    void MotionEstimate();
    void FrameME();            // In motionest.cc
    void FrameMEs();            // In motionest.cc
    void FieldME();
    void Predict();            // In predict.cc
    void Quantize();            // In quantize.cc
    void IQuantize();
    void Transform();          // In transfrm.cc
    void ITransform();
    void PutBlocks();           // In putpic.cc
    void SkippedCoding( bool slice_begin_end );

    inline const Picture &ParentPicture() const { return *picture; }
    inline int BaseLumVariance() const { return lum_variance; }
    inline double Activity() const { return act; }
    inline const int TopleftX() const { return i; }
    inline const int TopleftY() const { return j; }
    inline DCTblock *RawDCTblocks() const { return dctblocks; }
    inline DCTblock *QuantDCTblocks() const { return qdctblocks; }


private:
    Picture *picture;   
    unsigned int i,j;       // Co-ordinates top-left in picture

    DCTblock *dctblocks;
    DCTblock *qdctblocks;

    unsigned int lum_mean;
    unsigned int lum_variance;

    /* Old public struct information...
       TODO: This will gradually disappear as C++-ification continues
    */

public:
	bool field_dct;             // Field DCT encoded rather than frame DCT
	int mquant; /* quantization parameter */
	int cbp; /* coded block pattern */
	bool skipped; /* skipped macroblock */
	double act; /* activity measure */
	int i_act;  /* Activity measure if intra coded (I/P-frame) */
	int p_act;  /* Activity measure for *forward* prediction (P-frame) */
	int b_act;	/* Activity measure if bi-directionally coded (B-frame) */
    vector<MotionEst> plausible_me; // Suggestions from motion estimator
    MotionEst final_me;      // Motion estimate found to work best
#ifdef OUTPUT_STAT
  double N_act;
#endif

};

 

/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
#endif
