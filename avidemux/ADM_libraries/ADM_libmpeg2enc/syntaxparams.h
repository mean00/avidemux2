/*	Modded by Mean to cut mpeg2enc from encoding engine
	Original code from mjpeg.sf.net see other files for copyright
*/

/* syntaxparams.h, Global flags and constants controlling MPEG syntax */
#ifndef _SYNTAXPARAMS_H
#define _SYNTAXPARAMS_H


#include "ADM_default.h"
#include "mjpeg_types.h"

/* choose between declaration (GLOBAL undefined) and definition
 * (GLOBAL defined) GLOBAL is defined in exactly one file (mpeg2enc.c)
 * TODO: Get rid of this and do it cleanly....
 */

#ifndef GLOBAL
#define EXTERN extern
#else
#define EXTERN
#endif


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


/* SCale factor for fast integer arithmetic routines */
/* Changed this and you *must* change the quantisation routines as they depend on its absolute
	value */
#define IQUANT_SCALE_POW2 16
#define IQUANT_SCALE (1<<IQUANT_SCALE_POW2)
#define COEFFSUM_SCALE (1<<16)

#define BITCOUNT_OFFSET  0LL

/*
  How many frames to read in one go and the size of the frame data buffer.
*/
#ifdef PUSH
#define READ_CHUNK_SIZE 1
#else
#define READ_CHUNK_SIZE 5
#endif

/*
  How many frames encoding may be concurrently under way.
  N.b. there is no point setting this greater than M.
  Additional parallelism can be exposed at a finer grain by
  parallelising per-macro-block computations.
 */

#define MAX_WORKER_THREADS 1 //MEANX : No parallel stuff
/* motion data */
struct motion_data
{
  unsigned int forw_hor_f_code, forw_vert_f_code;	/* vector range */
  unsigned int sxf, syf;	/* search range */
  unsigned int back_hor_f_code, back_vert_f_code;
  unsigned int sxb, syb;
};
/*----------------------*/
typedef struct
{

  unsigned int 		horizontal_size;
  unsigned int 		vertical_size;	/* frame size (pels) */

  unsigned int 		aspectratio;	/* aspect ratio information (pel or display) */
  unsigned int 		frame_rate_code;	/* coded value of playback display
				 * frame rate */
  int 				dctsatlim;		/* Value to saturated DCT coeffs to */
  double 			frame_rate;		/* Playback display frames per
				   second N.b. when 3:2 pullback
				   is active this is higher than
				   the frame decode rate.
				 */
  double 			bit_rate;		/* bits per second */

  unsigned int 		vbv_buffer_code;	/* Code for size of VBV buffer (*
				 * 16 kbit) */
  double 			vbv_buffer_size;
  unsigned int 		still_size;	/* If non-0 encode a stills
				 * sequence: 1 I-frame per
				 * sequence pseudo VBR. Each
				 * frame sized to still_size
				 * KB */
  unsigned int 		vbv_buffer_still_size;	/* vbv_buffer_size holds still
					   size.  Ensure still size
					   matches. */


  int 				phy_chrom_width;
  int				phy_chrom_height;
  int 				phy_width2;
  int 				phy_height2;
  int				enc_height2;
  int				phy_chrom_width2;	/* picture size */


  unsigned int 		profile, level;	/* syntax / parameter constraints */

  int 				chroma_format;
  int 				low_delay;		/* no B pictures, skipped pictures */


/* sequence specific data (sequence display extension) */

  unsigned int 		video_format;	/* component, PAL, NTSC, SECAM or MAC */
  unsigned int 		color_primaries;	/* source primary chromaticity coordinates */
  unsigned int 		transfer_characteristics;	/* opto-electronic transfer char. (gamma) */
  unsigned int 		matrix_coefficients;	/* Eg,Eb,Er / Y,Cb,Cr matrix coefficients */
  unsigned int 		display_horizontal_size;
  unsigned int  	display_vertical_size;	/* display size */

/* 1 if maxval->hor_size, maxval->vert_size and maxval->samp_rate in conform.c should not be checked */
  int 				ignore_constraints;



/* picture specific data (currently controlled by global flags) */

  unsigned int 		dc_prec;


  int 				enc_width;
  int	 			enc_height;	/* encoded frame size (pels) multiples of 16 or 32 */

  int 				phy_width;
  int 				phy_height;	/* Physical Frame buffer size (pels) may differ
				 * from encoded size due to alignment
				 * constraints */


/* use only frame prediction and frame DCT (I,P,B) */
  int 				frame_pred_dct_tab[3];
  int 				qscale_tab[3];		/* linear/non-linear quantizaton table */
  int 				intravlc_tab[3];		/* intra vlc format (I,P,B) */
  int 				altscan_tab[3];		/* alternate scan (I,P,B */


  struct motion_data *motion_data;



/* Orginal intra / non_intra quantization matrices */
  uint16_t 		*intra_q;
  uint16_t			 *inter_q;

  bool 			topfirst;
  bool 			mpeg1;			/* ISO/IEC IS 11172-2 sequence */
  bool 			fieldpic;		/* use field pictures */
  bool 			pulldown_32;		/* 3:2 Pulldown of movie material */
  bool 			seq_hdr_every_gop;
  bool 			seq_end_every_gop;	/* Useful for Stills
				 * sequences... */
  bool 			svcd_scan_data;

  bool 			constrparms;		/* constrained parameters flag (MPEG-1 only) */
  bool 			load_iquant, load_niquant;	/* use non-default quant. matrices */
  bool 			prog_seq;		/* progressive sequence */


  /*----------------------- From here it is old param_ stuff that get into the setting, so it is INPUT parameters -------*/
 
} Mpeg2Settings;
/*----------------------*/

EXTERN Mpeg2Settings mySettings;
#ifndef NO_GLOBAL
EXTERN Mpeg2Settings *opt
#endif
#ifdef GLOBAL
  = &mySettings;
#endif
;

EXTERN uint16_t *i_intra_q, *i_inter_q;


/* **************************
 * Global flags controlling encoding behaviour
 ************************** */
typedef struct
{
double decode_frame_rate;	/* Actual stream frame
					   decode-rate. This is lower
					   than playback rate if 3:2
					   pulldown is active.
					 */
int video_buffer_size;	/* Video buffer requirement target */

int N_max;		/* number of frames in Group of Pictures (max) */
int N_min;		/* number of frames in Group of Pictures (min) */
int M;		/* distance between I/P frames */

int M_min;		/* Minimum distance between I/P frames */

bool closed_GOPs;	/* Force all GOPs to be closed -
				 * useful for satisfying
				 * requirements for multi-angle
				 * DVD authoring */

bool refine_from_rec;	/* Is final refinement of motion
					 * compensation computed from
					 * reconstructed reference frame
					 * image (slightly higher quality,
					 * bad for multi-threading) or
					 * original reference frame
					 * (opposite) */

int _44_red;		/* Sub-mean population reduction passes for 4x4 and 2x2 */
int _22_red;		/* Motion compensation stages                                           */
int seq_length_limit;
double nonvid_bit_rate;	/* Bit-rate for non-video to assume for
					   sequence splitting calculations */

double quant_floor;	/* quantisation floor [1..10] (0 for
				 * CBR) */


double act_boost;	/* Quantisation reduction factor for blocks
				   with little texture (low variance) */

double boost_var_ceil;	/* Variance below which
					 * quantisation boost cuts in */


int max_encoding_frames;	/* Maximum number of concurrent
					   frames to be concurrently encoded 
					   Used to control multi_threading.
					 */

bool parallel_read;	/* Does the input reader / bufferer
				   run as a seperate thread?
				 */
}t_control;;

/* *************************
 * input stream attributes
 ************************* */

EXTERN int istrm_nframes;	/* total number of frames to encode
				   Note: this may start enormous and shrink
				   down later if the input stream length is
				   unknown at the start of encoding.
				 */
//EXTERN int istrm_fd;

/* ***************************
 * Encoder internal derived values and parameters
 *************************** */



EXTERN int lum_buffer_size, chrom_buffer_size;



EXTERN int block_count;
EXTERN int mb_width, mb_height;	/* frame size (macroblocks) */

EXTERN int mb_height2;
EXTERN int qsubsample_offset, fsubsample_offset;
// EXTERN int  rowsums_offset, colsums_offset;	/* Offset from picture buffer start of sub-sampled data... */

EXTERN int mb_per_pict;		/* Number of macro-blocks in a picture */

#endif
