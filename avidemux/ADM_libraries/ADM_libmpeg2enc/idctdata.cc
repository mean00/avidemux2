//
// MMX32 iDCT algorithm  (IEEE-1180 compliant) :: idct_mmx32()
//
// MPEG2AVI
// --------
//  v0.16B33 initial release
//
// This was one of the harder pieces of work to code.
// Intel's app-note focuses on the numerical issues of the algorithm, but
// assumes the programmer is familiar with IDCT mathematics, leaving the
// form of the complete function up to the programmer's imagination.

#include <ADM_default.h>
#include <mjpeg_types.h>

//  ALGORITHM OVERVIEW
//  ------------------
// I played around with the code for quite a few hours.  I came up
// with *A* working IDCT algorithm, however I'm not sure whether my routine
// is "the correct one."  But rest assured, my code passes all six IEEE 
// accuracy tests with plenty of margin.
//
//   My IDCT algorithm consists of 4 steps:
//
//   1) IDCT-row transformation (using the IDCT-row function) on all 8 rows
//      This yields an intermediate 8x8 matrix.
//
//   2) intermediate matrix transpose (mandatory)
//
//   3) IDCT-row transformation (2nd time) on all 8 rows of the intermediate
//      matrix.  The output is the final-result, in transposed form.
//
//   4) post-transformation matrix transpose 
//      (not necessary if the input-data is already transposed, this could
//       be done during the MPEG "zig-zag" scan, but since my algorithm
//       requires at least one transpose operation, why not re-use the
//       transpose-code.)
//
//   Although the (1st) and (3rd) steps use the SAME row-transform operation,
//   the (3rd) step uses different shift&round constants (explained later.)
//
//   Also note that the intermediate transpose (2) would not be neccessary,
//   if the subsequent operation were a iDCT-column transformation.  Since
//   we only have the iDCT-row transform, we transpose the intermediate
//   matrix and use the iDCT-row transform a 2nd time.
//
//   I had to change some constants/variables for my method to work :
//
//      As given by Intel, the #defines for SHIFT_INV_COL and RND_INV_COL are
//      wrong.  Not surprising since I'm not using a true column-transform 
//      operation, but the row-transform operation (as mentioned earlier.)
//      round_inv_col[], which is given as "4 int16_t" values, should have the
//      same dimensions as round_inv_row[].  The corrected variables are 
//      shown.
//
//      Intel's code defines a different table for each each row operation.
//      The tables given are 0/4, 1/7, 2/6, and 5/3.  My code only uses row#0.
//      Using the other rows messes up the overall transform.
//
//   IMPLEMENTATION DETAILs
//   ----------------------
// 
//   I divided the algorithm's work into two subroutines,
//    1) idct_mmx32_rows() - transforms 8 rows, then transpose
//    2) idct_mmx32_cols() - transforms 8 rows, then transpose
//       yields final result ("drop-in" direct replacement for INT32 IDCT)
//
//   The 2nd function is a clone of the 1st, with changes made only to the
//   shift&rounding instructions.
//
//      In the 1st function (rows), the shift & round instructions use 
//       SHIFT_INV_ROW & round_inv_row[] (renamed to r_inv_row[])
//
//      In the 2nd function (cols)-> r_inv_col[], and
//       SHIFT_INV_COL & round_inv_col[] (renamed to r_inv_col[])
//
//   Each function contains an integrated transpose-operator, which comes
//   AFTER the primary transformation operation.  In the future, I'll optimize
//   the code to do more of the transpose-work "in-place".  Right now, I've
//   left the code as two subroutines and a main calling function, so other
//   people can read the code more easily.
//
//   liaor@umcc.ais.org  http://members.tripod.com/~liaor
//  

//;=============================================================================
//;
//;  AP-922   http://developer.intel.com/vtune/cbts/strmsimd
//; These examples contain code fragments for first stage iDCT 8x8
//; (for rows) and first stage DCT 8x8 (for columns)
//;
//;=============================================================================

#define BITS_INV_ACC	4	//; 4 or 5 for IEEE
	// 5 yields higher accuracy, but lessens dynamic range on the input matrix
#define SHIFT_INV_ROW	(16 - BITS_INV_ACC)
#define SHIFT_INV_COL	(1 + BITS_INV_ACC +14 )  // changed from Intel's val)


#define RND_INV_ROW		(1 << (SHIFT_INV_ROW-1))
#define RND_INV_COL		(1 << (SHIFT_INV_COL-1)) 
#define RND_INV_CORR	(RND_INV_COL - 1)		//; correction -1.0 and round

/* TODO: This should *really* be aligned on 16-byte boundaries... */

  int idct_r_inv_row[2] __asm__ ("idct_r_inv_row") __attribute__ ((aligned (16))) = { RND_INV_ROW, RND_INV_ROW};
  int idct_r_inv_col[2] __asm__ ("idct_r_inv_col") __attribute__ ((aligned (16))) = {RND_INV_COL, RND_INV_COL};
  int idct_r_inv_corr[2]  __asm__ ("idct_r_inv_corr") __attribute__ ((aligned (16))) = {RND_INV_CORR, RND_INV_CORR };

/* Unused and thus redundant...
const long long dct_one_corr = 0x0001000100010001;
*/

/*
;=============================================================================
;
; The first stage iDCT 8x8 - inverse DCTs of rows
;
;-----------------------------------------------------------------------------
; The 8-point inverse DCT direct algorithm
;-----------------------------------------------------------------------------
;
; static const int16_t w[32] = {
; FIX(cos_4_16), FIX(cos_2_16), FIX(cos_4_16), FIX(cos_6_16),
; FIX(cos_4_16), FIX(cos_6_16), -FIX(cos_4_16), -FIX(cos_2_16),
; FIX(cos_4_16), -FIX(cos_6_16), -FIX(cos_4_16), FIX(cos_2_16),
; FIX(cos_4_16), -FIX(cos_2_16), FIX(cos_4_16), -FIX(cos_6_16),
; FIX(cos_1_16), FIX(cos_3_16), FIX(cos_5_16), FIX(cos_7_16),
; FIX(cos_3_16), -FIX(cos_7_16), -FIX(cos_1_16), -FIX(cos_5_16),
; FIX(cos_5_16), -FIX(cos_1_16), FIX(cos_7_16), FIX(cos_3_16),
; FIX(cos_7_16), -FIX(cos_5_16), FIX(cos_3_16), -FIX(cos_1_16) };
;
; #define DCT_8_INV_ROW(x, y)

;{
; int a0, a1, a2, a3, b0, b1, b2, b3;
;
; a0 =x[0]*w[0]+x[2]*w[1]+x[4]*w[2]+x[6]*w[3];
; a1 =x[0]*w[4]+x[2]*w[5]+x[4]*w[6]+x[6]*w[7];
; a2 = x[0] * w[ 8] + x[2] * w[ 9] + x[4] * w[10] + x[6] * w[11];
; a3 = x[0] * w[12] + x[2] * w[13] + x[4] * w[14] + x[6] * w[15];
; b0 = x[1] * w[16] + x[3] * w[17] + x[5] * w[18] + x[7] * w[19];
; b1 = x[1] * w[20] + x[3] * w[21] + x[5] * w[22] + x[7] * w[23];
; b2 = x[1] * w[24] + x[3] * w[25] + x[5] * w[26] + x[7] * w[27];
; b3 = x[1] * w[28] + x[3] * w[29] + x[5] * w[30] + x[7] * w[31];
;
; y[0] = SHIFT_ROUND ( a0 + b0 );
; y[1] = SHIFT_ROUND ( a1 + b1 );
; y[2] = SHIFT_ROUND ( a2 + b2 );
; y[3] = SHIFT_ROUND ( a3 + b3 );
; y[4] = SHIFT_ROUND ( a3 - b3 );
; y[5] = SHIFT_ROUND ( a2 - b2 );
; y[6] = SHIFT_ROUND ( a1 - b1 );
; y[7] = SHIFT_ROUND ( a0 - b0 );
;}
;
;-----------------------------------------------------------------------------
;
; In this implementation the outputs of the iDCT-1D are multiplied
; for rows 0,4 - by cos_4_16,
; for rows 1,7 - by cos_1_16,
; for rows 2,6 - by cos_2_16,
; for rows 3,5 - by cos_3_16
; and are shifted to the left for better accuracy
;
; For the constants used,
; FIX(float_const) = (int16_t) (float_const * (1<<15) + 0.5)
;
;=============================================================================
*/

/* CONCATENATED TABLE, rows 0,1,2,3,4,5,6,7 (in order )

   In our implementation, however, we only use row0 !
*/

  int16_t idct_tab_01234567[] __asm__ ("idct_tab_01234567") = {
	//row0, this row is required
	16384, 16384, 16384, -16384,	// ; movq-> w06 w04 w02 w00
	21407, 8867, 8867, -21407,		// w07 w05 w03 w01
	16384, -16384, 16384, 16384,	//; w14 w12 w10 w08
	-8867, 21407, -21407, -8867,	//; w15 w13 w11 w09
	22725, 12873, 19266, -22725,	//; w22 w20 w18 w16
	19266, 4520, -4520, -12873,		//; w23 w21 w19 w17
	12873, 4520, 4520, 19266,		//; w30 w28 w26 w24
	-22725, 19266, -12873, -22725,  //w31 w29 w27 w25

	// the rest of these rows (1-7), aren't used !

	//row1
	22725, 22725, 22725, -22725,	// ; movq-> w06 w04 w02 w00
	29692, 12299, 12299, -29692,	//	; w07 w05 w03 w01
	22725, -22725, 22725, 22725,	//; w14 w12 w10 w08
	-12299, 29692, -29692, -12299,	//; w15 w13 w11 w09
	31521, 17855, 26722, -31521,	//; w22 w20 w18 w16
	26722, 6270, -6270, -17855,		//; w23 w21 w19 w17
	17855, 6270, 6270, 26722,		//; w30 w28 w26 w24
	-31521, 26722, -17855, -31521,	// w31 w29 w27 w25

	//row2
	21407, 21407, 21407, -21407,	// ; movq-> w06 w04 w02 w00
	27969, 11585, 11585, -27969,	// ; w07 w05 w03 w01
	21407, -21407, 21407, 21407,	// ; w14 w12 w10 w08
	-11585, 27969, -27969, -11585,	//  ;w15 w13 w11 w09
	29692, 16819, 25172, -29692, 	// ;w22 w20 w18 w16
	25172, 5906, -5906, -16819, 	// ;w23 w21 w19 w17
	16819, 5906, 5906, 25172, 		// ;w30 w28 w26 w24
	-29692, 25172, -16819, -29692,	//  ;w31 w29 w27 w25

	//row3
	19266, 19266, 19266, -19266,	//; movq-> w06 w04 w02 w00
	25172, 10426, 10426, -25172,	//; w07 w05 w03 w01
	19266, -19266, 19266, 19266,	//; w14 w12 w10 w08
	-10426, 25172, -25172, -10426,	//; w15 w13 w11 w09
	26722, 15137, 22654, -26722,	//; w22 w20 w18 w16
	22654, 5315, -5315, -15137,		//; w23 w21 w19 w17
	15137, 5315, 5315, 22654,		//; w30 w28 w26 w24
	-26722, 22654, -15137, -26722,	//; w31 w29 w27 w25

	//row4
	16384, 16384, 16384, -16384,	// ; movq-> w06 w04 w02 w00
	21407, 8867, 8867, -21407,		// w07 w05 w03 w01
	16384, -16384, 16384, 16384,	//; w14 w12 w10 w08
	-8867, 21407, -21407, -8867,	//; w15 w13 w11 w09
	22725, 12873, 19266, -22725,	//; w22 w20 w18 w16
	19266, 4520, -4520, -12873,		//; w23 w21 w19 w17
	12873, 4520, 4520, 19266,		//; w30 w28 w26 w24
	-22725, 19266, -12873, -22725,  //w31 w29 w27 w25

	//row5
	19266, 19266, 19266, -19266,	//; movq-> w06 w04 w02 w00
	25172, 10426, 10426, -25172,	//; w07 w05 w03 w01
	19266, -19266, 19266, 19266,	//; w14 w12 w10 w08
	-10426, 25172, -25172, -10426,	//; w15 w13 w11 w09
	26722, 15137, 22654, -26722,	//; w22 w20 w18 w16
	22654, 5315, -5315, -15137,		//; w23 w21 w19 w17
	15137, 5315, 5315, 22654,		//; w30 w28 w26 w24
	-26722, 22654, -15137, -26722,	//; w31 w29 w27 w25

	//row6
	21407, 21407, 21407, -21407,	// ; movq-> w06 w04 w02 w00
	27969, 11585, 11585, -27969,	// ; w07 w05 w03 w01
	21407, -21407, 21407, 21407,	// ; w14 w12 w10 w08
	-11585, 27969, -27969, -11585,	//  ;w15 w13 w11 w09
	29692, 16819, 25172, -29692, 	// ;w22 w20 w18 w16
	25172, 5906, -5906, -16819, 	// ;w23 w21 w19 w17
	16819, 5906, 5906, 25172, 		// ;w30 w28 w26 w24
	-29692, 25172, -16819, -29692,	//  ;w31 w29 w27 w25

	//row7
	22725, 22725, 22725, -22725,	// ; movq-> w06 w04 w02 w00
	29692, 12299, 12299, -29692,	//	; w07 w05 w03 w01
	22725, -22725, 22725, 22725,	//; w14 w12 w10 w08
	-12299, 29692, -29692, -12299,	//; w15 w13 w11 w09
	31521, 17855, 26722, -31521,	//; w22 w20 w18 w16
	26722, 6270, -6270, -17855,		//; w23 w21 w19 w17
	17855, 6270, 6270, 26722,		//; w30 w28 w26 w24
	-31521, 26722, -17855, -31521};	// w31 w29 w27 w25
