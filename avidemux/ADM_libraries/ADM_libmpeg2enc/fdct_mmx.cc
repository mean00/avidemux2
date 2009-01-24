//////////////////////////////////////////////////////////////////////////////
//
//  fdctam32.c - AP922 MMX(3D-Now) forward-DCT
//  ----------
//  Intel Application Note AP-922 - fast, precise implementation of DCT
//        http://developer.intel.com/vtune/cbts/appnotes.htm
//  ----------
//  
//       This routine can use a 3D-Now/MMX enhancement to increase the
//  accuracy of the fdct_col_4 macro.  The dct_col function uses 3D-Now's
//  PMHULHRW instead of MMX's PMHULHW(and POR).  The substitution improves
//  accuracy very slightly with performance penalty.  If the target CPU
//  does not support 3D-Now, then this function cannot be executed.
//  
//  For a fast, precise MMX implementation of inverse-DCT 
//              visit http://www.elecard.com/peter
//
//  v1.0 07/22/2000 (initial release)
//     
//  liaor@iname.com  http://members.tripod.com/~liaor  
//////////////////////////////////////////////////////////////////////////////

/*
 *  A.Stevens Jul 2000:	 ported to nasm syntax and disentangled from
 *  from Win**** compiler specific stuff.
 *  All the real work was done above though.
 *  See above for how to optimise quality on 3DNow! CPU's
 *  Nov 2003 changed to PIC for use in shared libraries
 *
 *  G.Vervoort Jan 2005: ported to inline asm.
 */

#include <config.h>
#ifdef HAVE_X86CPU
#include "mjpeg_types.h"
#include "attributes.h"
#include "mmx.h"


//////////////////////////////////////////////////////////////////////
//
// constants for the forward DCT
// -----------------------------
//
// Be sure to check that your compiler is aligning all constants to QWORD
// (8-byte) memory boundaries!  Otherwise the unaligned memory access will
// severely stall MMX execution.
//
//////////////////////////////////////////////////////////////////////

#define BITS_FRW_ACC	3 //; 2 or 3 for accuracy
#define SHIFT_FRW_COL	BITS_FRW_ACC
#define SHIFT_FRW_ROW	(BITS_FRW_ACC + 17)
//#define RND_FRW_ROW		(262144 * (BITS_FRW_ACC - 1)) //; 1 << (SHIFT_FRW_ROW-1)
#define RND_FRW_ROW		(1 << (SHIFT_FRW_ROW-1))
//#define RND_FRW_COL		(2 * (BITS_FRW_ACC - 1)) //; 1 << (SHIFT_FRW_COL-1)
#define RND_FRW_COL		(1 << (SHIFT_FRW_COL-1))

//concatenated table, for forward DCT transformation
static const int16_t fdct_tg_all_16[] ATTR_ALIGN(8) = {
    13036, 13036, 13036, 13036,         // tg * (2<<16) + 0.5
    27146, 27146, 27146, 27146,         // tg * (2<<16) + 0.5
    -21746, -21746, -21746, -21746,     // tg * (2<<16) + 0.5
};

static const int16_t ocos_4_16[4] ATTR_ALIGN(8) = {
    23170, 23170, 23170, 23170, //cos * (2<<15) + 0.5
};
//MEANX
static volatile const int64_t fdct_one_corr ATTR_ALIGN(8) = 0x0001000100010001LL;
static const int32_t fdct_r_row[2] ATTR_ALIGN(8) = {RND_FRW_ROW, RND_FRW_ROW };

static const int16_t tab_frw_01234567[] ATTR_ALIGN(8) = {  // forward_dct coeff table
    //row0
    16384, 16384, 21407, -8867,     //    w09 w01 w08 w00
    16384, 16384, 8867, -21407,     //    w13 w05 w12 w04
    16384, -16384, 8867, 21407,     //    w11 w03 w10 w02
    -16384, 16384, -21407, -8867,   //    w15 w07 w14 w06
    22725, 12873, 19266, -22725,    //    w22 w20 w18 w16
    19266, 4520, -4520, -12873,     //    w23 w21 w19 w17
    12873, 4520, 4520, 19266,       //    w30 w28 w26 w24
    -22725, 19266, -12873, -22725,  //    w31 w29 w27 w25

    //row1
    22725, 22725, 29692, -12299,    //    w09 w01 w08 w00
    22725, 22725, 12299, -29692,    //    w13 w05 w12 w04
    22725, -22725, 12299, 29692,    //    w11 w03 w10 w02
    -22725, 22725, -29692, -12299,  //    w15 w07 w14 w06
    31521, 17855, 26722, -31521,    //    w22 w20 w18 w16
    26722, 6270, -6270, -17855,     //    w23 w21 w19 w17
    17855, 6270, 6270, 26722,       //    w30 w28 w26 w24
    -31521, 26722, -17855, -31521,  //    w31 w29 w27 w25

    //row2
    21407, 21407, 27969, -11585,    //    w09 w01 w08 w00
    21407, 21407, 11585, -27969,    //    w13 w05 w12 w04
    21407, -21407, 11585, 27969,    //    w11 w03 w10 w02
    -21407, 21407, -27969, -11585,  //    w15 w07 w14 w06
    29692, 16819, 25172, -29692,    //    w22 w20 w18 w16
    25172, 5906, -5906, -16819,     //    w23 w21 w19 w17
    16819, 5906, 5906, 25172,       //    w30 w28 w26 w24
    -29692, 25172, -16819, -29692,  //    w31 w29 w27 w25

    //row3
    19266, 19266, 25172, -10426,    //    w09 w01 w08 w00
    19266, 19266, 10426, -25172,    //    w13 w05 w12 w04
    19266, -19266, 10426, 25172,    //    w11 w03 w10 w02
    -19266, 19266, -25172, -10426,  //    w15 w07 w14 w06, 
    26722, 15137, 22654, -26722,    //    w22 w20 w18 w16
    22654, 5315, -5315, -15137,     //    w23 w21 w19 w17
    15137, 5315, 5315, 22654,       //    w30 w28 w26 w24
    -26722, 22654, -15137, -26722,  //    w31 w29 w27 w25, 

    //row4
    16384, 16384, 21407, -8867,     //    w09 w01 w08 w00
    16384, 16384, 8867, -21407,     //    w13 w05 w12 w04
    16384, -16384, 8867, 21407,     //    w11 w03 w10 w02
    -16384, 16384, -21407, -8867,   //    w15 w07 w14 w06
    22725, 12873, 19266, -22725,    //    w22 w20 w18 w16
    19266, 4520, -4520, -12873,     //    w23 w21 w19 w17
    12873, 4520, 4520, 19266,       //    w30 w28 w26 w24
    -22725, 19266, -12873, -22725,  //    w31 w29 w27 w25 

    //row5
    19266, 19266, 25172, -10426,    //    w09 w01 w08 w00
    19266, 19266, 10426, -25172,    //    w13 w05 w12 w04
    19266, -19266, 10426, 25172,    //    w11 w03 w10 w02
    -19266, 19266, -25172, -10426,  //    w15 w07 w14 w06
    26722, 15137, 22654, -26722,    //    w22 w20 w18 w16
    22654, 5315, -5315, -15137,     //    w23 w21 w19 w17
    15137, 5315, 5315, 22654,       //    w30 w28 w26 w24
    -26722, 22654, -15137, -26722,  //    w31 w29 w27 w25

    //row6
    21407, 21407, 27969, -11585,    //    w09 w01 w08 w00
    21407, 21407, 11585, -27969,    //    w13 w05 w12 w04
    21407, -21407, 11585, 27969,    //    w11 w03 w10 w02
    -21407, 21407, -27969, -11585,  //    w15 w07 w14 w06, 
    29692, 16819, 25172, -29692,    //    w22 w20 w18 w16
    25172, 5906, -5906, -16819,     //    w23 w21 w19 w17
    16819, 5906, 5906, 25172,       //    w30 w28 w26 w24
    -29692, 25172, -16819, -29692,  //    w31 w29 w27 w25, 

    //row7
    22725, 22725, 29692, -12299,    //    w09 w01 w08 w00
    22725, 22725, 12299, -29692,    //    w13 w05 w12 w04
    22725, -22725, 12299, 29692,    //    w11 w03 w10 w02
    -22725, 22725, -29692, -12299,  //    w15 w07 w14 w06, 
    31521, 17855, 26722, -31521,    //    w22 w20 w18 w16
    26722, 6270, -6270, -17855,     //    w23 w21 w19 w17
    17855, 6270, 6270, 26722,       //    w30 w28 w26 w24
    -31521, 26722, -17855, -31521   //    w31 w29 w27 w25
};


#define x0 (inp + 0*8)
#define x1 (inp + 1*8)
#define x2 (inp + 2*8)
#define x3 (inp + 3*8)
#define x4 (inp + 4*8)
#define x5 (inp + 5*8)
#define x6 (inp + 6*8)
#define x7 (inp + 7*8)
#define y0 (out + 0*8)
#define y1 (out + 1*8)
#define y2 (out + 2*8)
#define y3 (out + 3*8)
#define y4 (out + 4*8)
#define y5 (out + 5*8)
#define y6 (out + 6*8)
#define y7 (out + 7*8)

#define round_frw_row fdct_r_row

     ////////////////////////////////////////////////////////////////////////
     //
     // The high-level pseudocode for the fdct_am32() routine :
     //
     // fdct_am32()
     // {
     //    forward_dct_col03(); // dct_column transform on cols 0-3
     //    forward_dct_col47(); // dct_column transform on cols 4-7
     //    for ( j = 0; j < 8; j=j+1 )
     //      forward_dct_row1(j); // dct_row transform on row #j
     // }
     //

void mp2_fdct_mmx(int16_t *blk)
{
	int16_t *inp, *out;
	int16_t *table;
	int i;
	
	/* transform the left half of the matrix (4 columns) */
	
	out = inp = blk;

	/*
	 * for ( i = 0; i < 2; i = i + 1)
	 * the for-loop is executed twice.  We are better off unrolling the 
	 * loop to avoid branch misprediction.
	 * .mmx32_fdct_col03: 
	 */
	movq_m2r(*x1, mm0);		/* 0 ; x1 			*/
	
	movq_m2r(*x6, mm1);		/* 1 ; x6 			*/
	movq_r2r(mm0, mm2);		/* 2 ; x1			*/
	
	movq_m2r(*x2, mm3);		/* 3 ; x2 			*/
	paddsw_r2r(mm1, mm0);		/* t1 = x[1] + x[6] 		*/

	movq_m2r(*x5, mm4);		/* 4 ; x5 			*/
	psllw_i2r(SHIFT_FRW_COL, mm0);	/* t1 				*/

	movq_m2r(*x0, mm5);		/* 5 ; x0 			*/
	paddsw_r2r(mm3, mm4);		/* t2 = x[2] + x[5]		*/
	
	paddsw_m2r(*x7, mm5);		/* t0 = x[0] + x[7]		*/
	psllw_i2r(SHIFT_FRW_COL, mm4);	/* t2				*/
	
	movq_r2r(mm0, mm6);		/* 6 ; t1			*/
	psubsw_r2r(mm1, mm2);		/* 1 ; t6 = x[1] - x[6]		*/
	
	movq_m2r(*(fdct_tg_all_16 + 4), mm1); 	/* 1 ; tg_2_16 		*/
	psubsw_r2r(mm4, mm0);		/* tm12 = t1 - t2		*/
	
	movq_m2r(*x3, mm7);		/* x3				*/
	pmulhw_r2r(mm0, mm1);		/* tm12*tg_2_16			*/
	
	paddsw_m2r(*x4, mm7);		/* t3 = x[3] + x[4]		*/
	psllw_i2r(SHIFT_FRW_COL, mm5);	/* t0				*/
	
	paddsw_r2r(mm4, mm6);		/* 4 ; tp12 = t1 + t2		*/
	psllw_i2r(SHIFT_FRW_COL, mm7);	/* t3				*/
	
	movq_r2r(mm5, mm4);		/* 4 ; t0			*/
	psubsw_r2r(mm7, mm5);		/* tm03 = t0 - t3		*/
	
	paddsw_r2r(mm5, mm1);		/* y2 = tm03 + tm12*tg_2_16 	*/
	paddsw_r2r(mm7, mm4);		/* 7 ; tp03 = t0 + t3		*/

	por_m2r(fdct_one_corr, mm1);	/* correction y2 +0.5		*/
	psllw_i2r(SHIFT_FRW_COL+1, mm2); /* t6 				*/
	 
	pmulhw_m2r(*(fdct_tg_all_16 + 4), mm5); 	/* tm03*tg_2_16			*/
	movq_r2r(mm4, mm7);		/* 7 ; tp03			*/
	
	psubsw_m2r(*x5, mm3);		/* t5 = x[2] - x[5]		*/
	psubsw_r2r(mm6, mm4);		/* y4 = tp03 - tp12		*/
	
	movq_r2m(mm1, *y2);		/* 1 ; save y2			*/
	paddsw_r2r(mm6, mm7);		/* 6 ; y0 = tp03 + tp12		*/

	movq_m2r(*x3, mm1);		/* 1 ; x3			*/
	psllw_i2r(SHIFT_FRW_COL+1, mm3); /* t5				*/
	
	psubsw_m2r(*x4, mm1);		/* t4 = x[3] - x[4]		*/
	movq_r2r(mm2, mm6);		/* 6 ; t6			*/
	
	movq_r2m(mm4, *y4);		/* 4 ; save y4			*/
	paddsw_r2r(mm3, mm2);		/* t6 + t5			*/
	
	pmulhw_m2r(*ocos_4_16, mm2);	/* tp65 = (t6 + t5)*cos_4_16 	*/
	psubsw_r2r(mm3, mm6);		/* 3 ; t6 - t5			*/
	
	pmulhw_m2r(*ocos_4_16, mm6);	/* tm65 = (t6 - t5)*cos_4_16 	*/
	psubsw_r2r(mm0, mm5);		/* 0 ; y6 = tm03*tg_2_16 - tm12 */
	
	por_m2r(fdct_one_corr, mm5);	/* correction y6 +0.5		*/
	psllw_i2r(SHIFT_FRW_COL, mm1);	/* t4				*/
	
	por_m2r(fdct_one_corr, mm2);	/* correction tp65 +0.5		*/
	movq_r2r(mm1, mm4);		/* 4 ; t4			*/
	
	movq_m2r(*x0, mm3);		/* 3 ; x0			*/
	paddsw_r2r(mm6, mm1);		/* tp465 = t4 + tm65		*/

	psubsw_m2r(*x7, mm3);		/* t7 = x[0] - x[7]		*/
	psubsw_r2r(mm6, mm4);		/* 6 ; tm465 = t4 - tm65	*/
	
	movq_m2r(*(fdct_tg_all_16 + 0), mm0);		/* 0 ; tg_1_16			*/
	psllw_i2r(SHIFT_FRW_COL, mm3);	/* t7				*/
	
	movq_m2r(*(fdct_tg_all_16 + 8), mm6);	/* 6 ; tg_3_16			*/
	pmulhw_r2r(mm1, mm0);		/* tp465*tg_1_16		*/
	
	movq_r2m(mm7, *y0);		/* 7 ; save y0			*/
	pmulhw_r2r(mm4, mm6);		/* tm465*tg_3_16		*/
	
	movq_r2m(mm5, *y6);		/* save y6			*/
	movq_r2r(mm3, mm7);		/* t7				*/
	
	movq_m2r(*(fdct_tg_all_16 + 8), mm5);		/* 5 ; tg_3_16			*/
	psubsw_r2r(mm2, mm7);		/* tm765 = t7 - tp65		*/
	
	paddsw_r2r(mm2, mm3);		/* 2 ; tp765 = t7 + tp65	*/
	pmulhw_r2r(mm7, mm5);		/* tm765*tg_3_16		*/
	
	paddsw_r2r(mm3, mm0);		/* y1 = tp765 + tp465*tg_1_16 	*/
	paddsw_r2r(mm4, mm6);		/* tm465*tg_3_16		*/
	
	pmulhw_m2r(*(fdct_tg_all_16 + 0), mm3);	/* tp765*tg_1_16		*/
	
	por_m2r(fdct_one_corr, mm0);	/* correction y1 +0.5		*/
	paddsw_r2r(mm7, mm5);		/* tm765*tg_3_16		*/
	
	psubsw_r2r(mm6, mm7);		/* 6 ; y3 = tm765 - tm465*tg_3_16 */
	inp += 4;			/* increment pointer		*/
	
	movq_r2m(mm0, *y1);		/* 0 ; save y1			*/
	paddsw_r2r(mm4, mm5);		/* 4 ; y5 = tm765*tg_3_16 + tm465 */
	
	movq_r2m(mm7, *y3);		/* 7 ; save y3			*/
	psubsw_r2r(mm1, mm3);		/* 1 ; y7 = tp765*tg_1_16 - tp465 */
	
	movq_r2m(mm5, *y5);		/* save y5			*/
	
	/* .mmx32_fdct_col47: ; begin processing last four columns */
	movq_m2r(*x1, mm0);		/* 0 ; x1 			*/
	
	movq_r2m(mm3, *y7);		/* 3 ; save y7 (columns 0-4)	*/
	
	movq_m2r(*x6, mm1);		/* 1 ; x6			*/
	movq_r2r(mm0, mm2);		/* 2 ; x1			*/
	
	movq_m2r(*x2, mm3);		/* 3 ; x2			*/
	paddsw_r2r(mm1, mm0);		/* t1 = x[1] + x[6]		*/
	
	movq_m2r(*x5, mm4);		/*  4 ; x5			*/
	psllw_i2r(SHIFT_FRW_COL, mm0);	/* t1				*/
	
	movq_m2r(*x0, mm5);		/* 5 ; x0			*/
	paddsw_r2r(mm3, mm4);		/* t2 = x[2] + x[5]		*/
	
	paddsw_m2r(*x7, mm5);		/* t0 = x[0] + x[7]		*/
	psllw_i2r(SHIFT_FRW_COL, mm4);	/* t2				*/
	
	movq_r2r(mm0, mm6);		/* 6 ; t1			*/
	psubsw_r2r(mm1, mm2);		/* 1 ; t6 = x[1] - x[6]		*/
	
	movq_m2r(*(fdct_tg_all_16 + 4), mm1);		/* 1 ; tg_2_16			*/
	psubsw_r2r(mm4, mm0);		/* tm12 = t1 - t2 		*/
	
	movq_m2r(*x3, mm7);		/* 7 ; x3			*/
	pmulhw_r2r(mm0, mm1);		/* tm12*tg_2_16			*/
	
	paddsw_m2r(*x4, mm7);		/* t3 = x[3] + x[4]		*/
	psllw_i2r(SHIFT_FRW_COL, mm5);	/* t0				*/
	
	paddsw_r2r(mm4, mm6);		/* 4 ; tp12 = t1 + t2		*/
	psllw_i2r(SHIFT_FRW_COL, mm7);	/* t3				*/
	
	movq_r2r(mm5, mm4);		/* 4 ; t0			*/
	psubsw_r2r(mm7, mm5);		/* tm03 = t0 - t3		*/
	
	paddsw_r2r(mm5, mm1);		/* y2 = tm03 + tm12*tg_2_16	*/
	paddsw_r2r(mm7, mm4);		/* 7 ; tp03 = t0 + t3		*/
	
	por_m2r(fdct_one_corr, mm1);	/* correction y2 +0.5		*/
	psllw_i2r(SHIFT_FRW_COL+1, mm2); /* t6				*/
	
	pmulhw_m2r(*(fdct_tg_all_16 + 4), mm5);	/* tm03*tg_2_16			*/
	movq_r2r(mm4, mm7);		/* 7 ; tp03			*/
	
	psubsw_m2r(*x5, mm3);		/* t5 = x[2] - x[5]		*/
	psubsw_r2r(mm6, mm4);		/* y4 = tp03 - tp12		*/
	
	movq_r2m(mm1, *(y2+4));		/* save y2			*/
	paddsw_r2r(mm6, mm7);		/* y0 = tp03 + tp12		*/
	
	movq_m2r(*x3, mm1);		/* 1 ; x3			*/
	psllw_i2r(SHIFT_FRW_COL+1, mm3); /* t5				*/
	
	psubsw_m2r(*x4, mm1);		/* t4 = x[3] - x[4]		*/
	movq_r2r(mm2, mm6);		/* 6 ; t6			*/
	
	movq_r2m(mm4, *(y4+4));		/* save y4			*/
	paddsw_r2r(mm3, mm2);		/* t6 + t5			*/
	
	pmulhw_m2r(*ocos_4_16, mm2);	/* tp65 = (t6 + t5)*cos_4_16	*/
	psubsw_r2r(mm3, mm6);		/* 3 ; t6 - t5			*/
	
	pmulhw_m2r(*ocos_4_16, mm6);	/* tm65 = (t6 - t5)*cos_4_16	*/
	psubsw_r2r(mm0, mm5);		/* 0 ; y6 = tm03*tg_2_16 - tm12 */
	
	por_m2r(fdct_one_corr, mm5);	/* correction y6 +0.5		*/
	psllw_i2r(SHIFT_FRW_COL, mm1);	/* t4				*/
	
	por_m2r(fdct_one_corr, mm2);	/* correction tp65 +0.5		*/
	movq_r2r(mm1, mm4);		/* 4 ; t4			*/
	
	movq_m2r(*x0, mm3);		/* 3 ; x0			*/
	paddsw_r2r(mm6, mm1);		/* tp465 = t4 + tm65		*/
	
	psubsw_m2r(*x7, mm3);		/* t7 = x[0] - x[7]		*/
	psubsw_r2r(mm6, mm4);		/* 6 ; tm465 = t4 - tm65	*/
	
	movq_m2r(*(fdct_tg_all_16 + 0), mm0);		/* 0 ; tg_1_16			*/
	psllw_i2r(SHIFT_FRW_COL, mm3);	/* t7				*/
	
	movq_m2r(*(fdct_tg_all_16 + 8), mm6);		/* 6 ; tg_3_16			*/
	pmulhw_r2r(mm1, mm0);		/* tp465*tg_1_16		*/
	
	movq_r2m(mm7, *(y0+4));		/* 7 ; save y0			*/
	pmulhw_r2r(mm4, mm6);		/* tm465*tg_3_16		*/
	
	movq_r2m(mm5, *(y6+4));		/* 5 ; save y6			*/
	movq_r2r(mm3, mm7);		/* 7 ; t7			*/
	
	movq_m2r(*(fdct_tg_all_16 + 8), mm5);		/* 5 ; tg_3_16			*/
	psubsw_r2r(mm2, mm7);		/* tm765 = t7 - tp65		*/
	
	paddsw_r2r(mm2, mm3);		/* 2 ; tp765 = t7 + tp65	*/
	pmulhw_r2r(mm7, mm5);		/* tm765*tg_3_16		*/
	
	paddsw_r2r(mm3, mm0);		/* y1 = tp765 + tp465*tg_1_16	*/
	paddsw_r2r(mm4, mm6);		/* tm465*tg_3_16		*/
	
	pmulhw_m2r(*(fdct_tg_all_16 + 0), mm3);	/* tp765*tg_1_16		*/
	
	por_m2r(fdct_one_corr, mm0);	/* correction y1 +0.5		*/
	paddsw_r2r(mm7, mm5);		/* tm765*tg_3_16		*/
	
	psubsw_r2r(mm6, mm7);		/* 6 ; y3 = tm765 - tm465*tg_3_16 */
	
	movq_r2m(mm0, *(y1+4));		/* 0 ; save y1			*/
	paddsw_r2r(mm4, mm5);		/* 4 ; y5 = tm765*tg_3_16 + tm465 */
	
	movq_r2m(mm7, *(y3+4));		/* 7 ; save y3			*/
	psubsw_r2r(mm1, mm3);		/* 1 ; y7 = tp765*tg_1_16 - tp465 */
	
	movq_r2m(mm5, *(y5+4));		/* 5 ; save y5			*/
	
	movq_r2m(mm3, *(y7+4));		/* 3 ; save y7			*/
	
	/*    }   ; end of forward_dct_col07() */
	/*  done with dct_row transform        */
	
/*
 * fdct_mmx32_cols() --
 * the following subroutine repeats the row-transform operation, 
 * except with different shift&round constants.  This version
 * does NOT transpose the output again.  Thus the final output
 * is transposed with respect to the source.
 * 
 *  The output is stored into blk[], which destroys the original
 *  input data.
 */

	out = inp = blk;

	table = (int16_t *)tab_frw_01234567;		
		
		/*
		 * for ( x = 8; x > 0; --x )  ; transform one row per iteration
 		 *  ---------- loop begin
 		*/
	for(i=0; i<8; i++)
	{
		movd_m2r(*(inp+6), mm5);	/* mm5 = 7 6 		*/
		
		punpcklwd_m2r(*(inp+4), mm5);	/* mm5 =  5 7 4 6	*/
		
		movq_r2r(mm5, mm2);		/* mm2 = 5 7 4 6	*/
		psrlq_i2r(32, mm5);		/* mm5 = _ _ 5 7	*/
		
		movq_m2r(*inp, mm0);		/* mm0 = 3 2 1 0	*/
		punpcklwd_r2r(mm2, mm5);	/* mm5 = 4 5 6 7	*/
		
		movq_r2r(mm0, mm1);		/* mm1 = 3 2 1 0	*/
		paddsw_r2r(mm5, mm0);		/* mm0 = [3+4, 2+5, 1+6, 0+7] (xt3, xt2, xt1, xt0) */
		
		psubsw_r2r(mm5, mm1);		/* mm1 = [3-4, 2-5, 1-6, 0-7] (xt7, xt6, xt5, xt4) */
		movq_r2r(mm0, mm2);		/* mm2 = [ xt3 xt2 xt1 xt0 ] */
		
		/* movq [ xt3xt2xt1xt0 ], mm0; */
		/* movq [ xt7xt6xt5xt4 ], mm1; */
		
		punpcklwd_r2r(mm1, mm0);	/* mm0 = [ xt5 xt1 xt4 xt0 ] */
		
		punpckhwd_r2r(mm1, mm2);	/* mm2 = [ xt7 xt3 xt6 xt2 ] */
		movq_r2r(mm2, mm1);		/* mm1			*/
		
		/* shuffle bytes around */
		
			/*  movq mm0,  [INP] ; 0 ; x3 x2 x1 x0	 */
			/*  movq mm1,  [INP+8] ; 1 ; x7 x6 x5 x4 */
		movq_r2r(mm0, mm2);	 	/* 2 ; x3 x2 x1 x0	*/
		
		movq_m2r(*table, mm3); 		/* 3 ; w06 w04 w02 w00 	*/
		punpcklwd_r2r(mm1, mm0);	/* x5 x1 x4 x0 		*/	

		movq_r2r(mm0, mm5);		/* 5 ; x5 x1 x4 x0	*/
		punpckldq_r2r(mm0, mm0);	/* x4 x0 x4 x0  [ xt2 xt0 xt2 xt0 ] */
		
		movq_m2r(*(table+4), mm4);	/* 4 ; w07 w05 w03 w01	*/
		punpckhwd_r2r(mm1, mm2);	/* 1 ; x7 x3 x6 x2	*/
		
		pmaddwd_r2r(mm0, mm3);		/* x4*w06+x0*w04 x4*w02+x0*w00 */
		movq_r2r(mm2, mm6);		/* 6 ; x7 x3 x6 x2	*/
		
		movq_m2r(*(table+16), mm1);	/* 1 ; w22 w20 w18 w16	*/
		punpckldq_r2r(mm2, mm2);	/* x6 x2 x6 x2  [ xt3 xt1 xt3 xt1 ] */
		
		pmaddwd_r2r(mm2, mm4);		/* x6*w07+x2*w05 x6*w03+x2*w01 */
		punpckhdq_r2r(mm5, mm5);	/* x5 x1 x5 x1  [ xt6 xt4 xt6 xt4 ] */
		
		pmaddwd_m2r(*(table+8), mm0);	/* x4*w14+x0*w12 x4*w10+x0*w08 */
		punpckhdq_r2r(mm6, mm6);	/*  x7 x3 x7 x3  [ xt7 xt5 xt7 xt5 ] */
		
		movq_m2r(*(table+20), mm7);	/* 7 ; w23 w21 w19 w17		*/
		pmaddwd_r2r(mm5, mm1);		/* x5*w22+x1*w20 x5*w18+x1*w16 	*/
		
			/*
			 * mm3 = a1, a0 (y2,y0)
			 * mm1 = b1, b0 (y3,y1)
			 * mm0 = a3,a2  (y6,y4)
			 * mm5 = b3,b2  (y7,y5)
			 */

		paddd_m2r(*round_frw_row, mm3);	/* +rounder (y2,y0) 	*/
		pmaddwd_r2r(mm6, mm7);		/* x7*w23+x3*w21 x7*w19+x3*w17 */
		
		pmaddwd_m2r(*(table+12), mm2);	/* x6*w15+x2*w13 x6*w11+x2*w09 */
		paddd_r2r(mm4, mm3);		/* 4 ; a1=sum(even1) a0=sum(even0) ; now ( y2, y0) */
		
		pmaddwd_m2r(*(table+24), mm5);	/* x5*w30+x1*w28 x5*w26+x1*w24	*/
		
		pmaddwd_m2r(*(table+28), mm6);	/* x7*w31+x3*w29 x7*w27+x3*w25 */
		paddd_r2r(mm7, mm1);		/* 7 ; b1=sum(odd1) b0=sum(odd0) ; now ( y3, y1) */
		
		paddd_m2r(*round_frw_row, mm0);	/* +rounder (y6,y4)	*/
		psrad_i2r(SHIFT_FRW_ROW, mm3);	/* (y2, y0)		*/
		
		paddd_m2r(*round_frw_row, mm1);	/* +rounder (y3,y1) 	*/
		paddd_r2r(mm2, mm0);		/* 2 ; a3=sum(even3) a2=sum(even2) ; now (y6, y4) */
		
		paddd_m2r(*round_frw_row, mm5);	/* +rounder (y7,y5)	*/
		psrad_i2r(SHIFT_FRW_ROW, mm1);	/* y1=a1+b1 y0=a0+b0	*/
		
		paddd_r2r(mm6, mm5);		/* 6 ; b3=sum(odd3) b2=sum(odd2) ; now ( y7, y5) */
		psrad_i2r(SHIFT_FRW_ROW, mm0);	/* y3=a3+b3 y2=a2+b2	*/
		
		out += 8;			/* increment row-output address by 1 row */
		psrad_i2r(SHIFT_FRW_ROW, mm5);	/* y4=a3-b3 y5=a2-b2	*/
		
		inp += 8;			/*  increment row-address by 1 row */
		packssdw_r2r(mm0, mm3);		/* 0 ; y6 y4 y2 y0	*/
		
		packssdw_r2r(mm5, mm1);		/* 3 ; y7 y5 y3 y1	*/
		movq_r2r(mm3, mm6);		/* mm0 = y6 y4 y2 y0	*/
		
		punpcklwd_r2r(mm1, mm3);	/* y3 y2 y1 y0		*/
		
		punpckhwd_r2r(mm1, mm6);	/* y7 y6 y5 y4		*/
		table += 32;			/* increment to next table */
	
		movq_r2m(mm3, *(out-8));	/* 1 ; save y3 y2 y1 y0	*/
		
		movq_r2m(mm6, *(out-4));	/* 7 ; save y7 y6 y5 y4 */
	}

	emms();
}
#endif
