/******************************************************************************
 *
 *   XviD VBR Library
 *   
 *   Copyright (C) 2002 Edouard Gomez <ed.gomez@wanadoo.fr>
 *
 *   The curve treatment algorithm is based on work done by Foxer <email?> and
 *   Dirk Knop <dknop@gwdg.de> for the XviD vfw dynamic library.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/

/* Standard Headers */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <math.h>
#include "ADM_assert.h"

#include "config.h"
/* Local headers */
#include "xvid_vbr.h"

/******************************************************************************
 * Build time constants
 *****************************************************************************/

/*
 * Portability note
 * Perhaps the msvc headers define Pi with another constant name
 */
#define DEG2RAD (M_PI / 180.0)

/* Defaults settings will be computed with the help of these constants */
#define DEFAULT_DESIRED_SIZE    700
#define DEFAULT_AUDIO_BITRATE   128
#define DEFAULT_MOVIE_LENGTH      2
#define DEFAULT_TWOPASS_BOOST  1000
#define DEFAULT_FPS           25.0f
#define DEFAULT_CREDITS_SIZE      0

#define DEFAULT_XVID_DBG_FILE   "xvid.dbg"
#define DEFAULT_XVID_STATS_FILE "xvid.stats"

#define aprintf printf
/*#include "ADM_osSupport/ADM_debugID.h"
#define MODULE_NAME 0
#include "ADM_osSupport/ADM_debug.h"*/
/******************************************************************************
 * Local prototypes
 *****************************************************************************/

/* Sub vbrInit cases functions */
static vbr_init_function vbr_init_dummy;
static vbr_init_function vbr_init_2pass1;
static vbr_init_function vbr_init_2pass2;
static vbr_init_function vbr_init_fixedquant;

/* Sub vbrGetQuant cases functions */
static vbr_get_quant_function vbr_getquant_1pass;
static vbr_get_quant_function vbr_getquant_2pass1;
static vbr_get_quant_function vbr_getquant_2pass2;
static vbr_get_quant_function vbr_getquant_fixedquant;

/* Sub vbrGetIntra cases functions */
static vbr_get_intra_function vbr_getintra_1pass;
static vbr_get_intra_function vbr_getintra_2pass1;
static vbr_get_intra_function vbr_getintra_2pass2;
static vbr_get_intra_function vbr_getintra_fixedquant;

/* Sub vbrUpdate prototypes */
static vbr_update_function vbr_update_dummy;
static vbr_update_function vbr_update_2pass1;
static vbr_update_function vbr_update_2pass2;

/* Sub vbrFinish cases functions */
static vbr_finish_function vbr_finish_dummy;
static vbr_finish_function vbr_finish_2pass1;
static vbr_finish_function vbr_finish_2pass2;

/*------------MEANX------------------*/
/*
	This table gives more strength to I frame
	The average I frame should be around 10


*/
static int vbr_predict(vbr_control_t *state,int original_size,int qp);
static float vbr_get_comp(int oldbits, int qporg, int newbits, int qpused);
static int  vbr_make_variance(vbr_control_t *state, float compression,  int *variance,int *bitrate);
static int vbr_getstuff(vbr_control_t *state, int dbytes,double diff);
static int  vbr_make_clipping(vbr_control_t *state, float compression);
static const int quant_I_modifier[32]=
{
	2,	2,	2,	3,	//00--33
	4,	5,	6,	6,	//4--7
	7, 	7, 	8,	8,   	//8--11
	9,	9, 	10,	10, 	//12--15
	11,	12, 	12,	13,	//16--19
	13,	14, 	14,	14,	//20--23
	15,	15,	15, 	16,	//24--27
	16,	16,	17,	17  //28--31
};
/*
	That one gives less to B frame
*/
static const int quant_B_modifier[32]=
{
	3,	3,	3,	3,	//00--33
	5,	6,	8,	9,	//4--7
	10, 	11, 	12,	13,   	//8--11
	14,	16, 	17,	18, 	//12--15
	20,	21, 	23,	24,	//16--19
	25,	26, 	28,	29,	//20--23
	30,	30,	31, 	31,	//24--27
	31,	31,	31,	31  //28--31
};

/*------------MEANX------------------*/

/* Is the encoder in the credits */
#define FRAME_TYPE_NORMAL_MOVIE     0x00
#define FRAME_TYPE_STARTING_CREDITS 0x01
#define FRAME_TYPE_ENDING_CREDITS   0x02

/******************************************************************************
 * Inline utility functions
 *****************************************************************************/

static __inline int util_frametype(vbr_control_t *state)
{

	if(state->credits_start) {

		if(state->cur_frame >= state->credits_start_begin &&
		   state->cur_frame < state->credits_start_end)
			return(FRAME_TYPE_STARTING_CREDITS);

	}

	if(state->credits_end) {

		if(state->cur_frame >= state->credits_end_begin &&
		   state->cur_frame < state->credits_end_end)
			return(FRAME_TYPE_STARTING_CREDITS);

	}

	return(FRAME_TYPE_NORMAL_MOVIE);


}

static __inline int util_creditsframes(vbr_control_t *state)
{

	int frames = 0;

	if(state->credits_start)
		frames += state->credits_start_end - state->credits_start_begin;
	if(state->credits_end)
		frames += state->credits_end_end - state->credits_end_begin;

	return(frames);

}

/******************************************************************************
 * Functions
 *****************************************************************************/

/*****************************************************************************
 * Function description :
 *
 * This function initialiazes the vbr_control_t with safe defaults for all
 * modes.
 *
 * Return Values :
 *   = 0
 ****************************************************************************/

int vbrSetDefaults(vbr_control_t *state)
{

	/* Set all the structure to zero */
	memset(state, 0, sizeof(state));

	/* Default mode is CBR */
	state->mode = VBR_MODE_1PASS;

	/* Default statistic filename */
	state->filename = DEFAULT_XVID_STATS_FILE;

	/*
	 * Default is a 2hour movie on 700Mo CD-ROM + 128kbit sound track
	 * This represents a target bitrate of 687kbit/s
	 */
	state->desired_size = DEFAULT_DESIRED_SIZE*1024*1024 -
		DEFAULT_MOVIE_LENGTH*3600*DEFAULT_AUDIO_BITRATE*1000/8;
	state->desired_bitrate = state->desired_size*8/(DEFAULT_MOVIE_LENGTH*3600);

	/* Credits */
	state->credits_mode = VBR_CREDITS_MODE_RATE;
	state->credits_start = 0;
	state->credits_start_begin = 0;
	state->credits_start_end = 0;
	state->credits_end = 0;
	state->credits_end_begin = 0;
	state->credits_end_end = 0;
	state->credits_quant_ratio = 20;
	state->credits_fixed_quant = 20;
	state->credits_quant_i = 20;
	state->credits_quant_p = 20;
	state->credits_start_size = DEFAULT_CREDITS_SIZE*1024*1024;
	state->credits_end_size = DEFAULT_CREDITS_SIZE*1024*1024;

	/* Keyframe boost */
	state->keyframe_boost = 0;
	state->kftreshold = 10;
	state->kfreduction = 30;
	state->min_key_interval = 1;
	state->max_key_interval = (int)DEFAULT_FPS*10;

	/* Normal curve treatment */
	state->curve_compression_high = 25;
	state->curve_compression_low = 10;

	/* Alt curve */
	state->use_alt_curve = 1;
	state->alt_curve_type = VBR_ALT_CURVE_LINEAR;
	state->alt_curve_low_dist = 90;
	state->alt_curve_high_dist = 500;
	state->alt_curve_min_rel_qual = 50;
	state->alt_curve_use_auto = 1;
	state->alt_curve_auto_str = 30;
	state->alt_curve_use_auto_bonus_bias = 1;
	state->alt_curve_bonus_bias = 50;
	state->bitrate_payback_method = VBR_PAYBACK_BIAS;
	state->bitrate_payback_delay = 250;
	state->twopass_max_bitrate = DEFAULT_TWOPASS_BOOST*state->desired_bitrate;
	state->twopass_max_overflow_improvement = 60;
	state->twopass_max_overflow_degradation = 60;
	state->max_iquant = 31;
	state->min_iquant = 2;
	state->max_pquant = 31;
	state->min_pquant = 2;
	state->fixed_quant = 3;

	state->max_framesize = (1.0/(float)DEFAULT_FPS) * state->twopass_max_bitrate / 8;

	state->fps = (float)DEFAULT_FPS;

	state->maxAllowedBitrate=0;

	return(0);

}

/*****************************************************************************
 * Function description :
 *
 * This function initialiaze the vbr_control_t state passed in parameter.
 *
 * The initialization depends on state->mode, there are 4 modes allowed.
 * Each mode description is done in the README file shipped with the lib.
 *
 * Return values :
 *
 *    =  0 on success
 *    = -1 on error
 *****************************************************************************/

int vbrInit(vbr_control_t *state)
{

	if(state == NULL) return(-1);

	/* Function pointers safe initialization */
	state->init     = NULL;
	state->getquant = NULL;
	state->getintra = NULL;
	state->update   = NULL;
	state->finish   = NULL;

// MeanX
	state->roundup=(int)floor(state->fps+0.99);

	state->size   = NULL;
	state->kf   = NULL;
	state->type   = NULL;
	printf("\n Roundup : %d\n",state->roundup);
// MeanX
	if(state->debug) {

		state->debug_file = fopen(DEFAULT_XVID_DBG_FILE, "w+");

		if(state->debug_file == NULL)
			return(-1);

		fprintf(state->debug_file, "# XviD Debug output\n");
		fprintf(state->debug_file, "# quant | intra | header bytes"
			"| total bytes | kblocks | mblocks | ublocks"
			"| vbr overflow | vbr kf overflow"
			"| vbr kf partial overflow\n\n");
	}

	/* Function pointers sub case initialization */
	switch(state->mode) {
	case VBR_MODE_1PASS:
		state->init     = vbr_init_dummy;
		state->getquant = vbr_getquant_1pass;
		state->getintra = vbr_getintra_1pass;
		state->update   = vbr_update_dummy;
		state->finish   = vbr_finish_dummy;
		break;
	case VBR_MODE_2PASS_1:
		state->init     = vbr_init_2pass1;
		state->getquant = vbr_getquant_2pass1;
		state->getintra = vbr_getintra_2pass1;
		state->update   = vbr_update_2pass1;
		state->finish   = vbr_finish_2pass1;
		break;
	case VBR_MODE_FIXED_QUANT:
		state->init     = vbr_init_fixedquant;
		state->getquant = vbr_getquant_fixedquant;
		state->getintra = vbr_getintra_fixedquant;
		state->update   = vbr_update_dummy;
		state->finish   = vbr_finish_dummy;
		break;
	case VBR_MODE_2PASS_2:
		state->init     = vbr_init_2pass2;
		state->getintra = vbr_getintra_2pass2;
		state->getquant = vbr_getquant_2pass2;
		state->update   = vbr_update_2pass2;
		state->finish   = vbr_finish_2pass2;
		break;
	default:
		return(-1);
	}
	
	return(state->init(state));

}

/******************************************************************************
 * Function description :
 *
 * This function returns an adapted quantizer according to the current vbr
 * controler state
 *
 * Return values :
 *  the quantizer value (0 <= value <= 31)
 *  (0 is a special case, means : let XviD decide)
 *
 *****************************************************************************/

int vbrGetQuant(vbr_control_t *state)
{

	/* Returns Zero, so XviD decides alone */
	if(state == NULL || state->getquant == NULL) return(0);

	return(state->getquant(state));

}

/******************************************************************************
 * Function description :
 *
 * This function returns the type of the frame to be encoded next (I or P/B)
 *
 * Return values :
 *  = -1 let the XviD encoder decide wether or not the next frame is I
 *  =  0 no I frame
 *  =  1 force keyframe
 *
 *****************************************************************************/

int vbrGetIntra(vbr_control_t *state)
{

	/* Returns -1, means let XviD decide */
	if(state == NULL || state->getintra == NULL) return(-1);

	return(state->getintra(state));

}

/******************************************************************************
 * Function description :
 *
 * This function updates the vbr control state according to collected statistics
 * from XviD core
 *
 * Return values :
 *
 *    =  0 on success
 *    = -1 on error
 *****************************************************************************/

int vbrUpdate(vbr_control_t *state,
	      int quant,
	      int intra,
	      int header_bytes,
	      int total_bytes,
	      int kblocks,
	      int mblocks,
	      int ublocks)
{

	if(state == NULL || state->update == NULL) return(-1);

	if(state->debug && state->debug_file != NULL) {
		int idx;

		fprintf(state->debug_file, "%d %d %d %d %d %d %d %d %d %d\n",
			quant, intra, header_bytes, total_bytes, kblocks,
			mblocks, ublocks, state->overflow, state->KFoverflow,
			state->KFoverflow_partial);

		idx = quant;

		if(quant < 1)
			idx = 1;
		if(quant > 31)
			idx = 31;

		idx--;

		state->debug_quant_count[idx]++;

	}

	return(state->update(state, quant, intra, header_bytes, total_bytes,
			     kblocks, mblocks, ublocks));

}

/******************************************************************************
 * Function description :
 *
 * This function stops the vbr controller
 *
 * Return values :
 *
 *    =  0 on success
 *    = -1 on error
 *****************************************************************************/

int vbrFinish(vbr_control_t *state)
{

	if(state == NULL || state->finish == NULL) return(-1);

	if(state->debug && state->debug_file != NULL) {

		int i;

		fprintf(state->debug_file, "\n\n");

		for(i=0; i<79; i++)
			fprintf(state->debug_file, "#");

		fprintf(state->debug_file, "\n# Quantizer distribution :\n\n");

		for(i=0;i<32; i++) {

			fprintf(state->debug_file, "# quant %d : %d\n",
				i+1,
				state->debug_quant_count[i]);

		}

		fclose(state->debug_file);

	}

	return(state->finish(state));

}

/******************************************************************************
 * Dummy functions - Used when a mode does not need such a function
 *****************************************************************************/

static int vbr_init_dummy(void *sstate)
{

	vbr_control_t *state = sstate;

	state->cur_frame = 0;

	return(0);

}

static int vbr_update_dummy(void *state,
			    int quant,
			    int intra,
			    int header_bytes,
			    int total_bytes,
			    int kblocks,
			    int mblocks,
			    int ublocks)
{
	quant=intra+header_bytes+total_bytes+kblocks+mblocks+ublocks; // MEANX : No warning
	((vbr_control_t*)state)->cur_frame++;

	return(0);

}

static int vbr_finish_dummy(void *state)
{
	if(state)
	{

	}
	return(0);

}

/******************************************************************************
 * 1 pass mode - XviD will do its job alone.
 *****************************************************************************/

static int vbr_getquant_1pass(void *state)
{
if(state)
	{

	}

	return(0);

}

static int vbr_getintra_1pass(void *state)
{
if(state)
	{

	}
	return(-1);

}

/******************************************************************************
 * 2 pass mode - first pass functions
 *****************************************************************************/

static int vbr_init_2pass1(void *sstate)
{

	FILE *f;
	vbr_control_t *state = sstate;

	/* Check the filename */
	if(state->filename == NULL || state->filename[0] == '\0')
		return(-1);

	printf("XvidVBR: initialized with file :%s, pass 1\n",state->filename);
	/* Initialize safe defaults for 2pass 1 */ 
	state->pass1_file = NULL;
	state->nb_frames = 0;
	state->nb_keyframes = 0;
	state->cur_frame = 0;

	/* Open the 1st pass file */
	if((f = fopen(state->filename, "w+")) == NULL)
		return(-1);

	/*
	 * The File Header
	 *
	 * The extra white spaces will be used during the vbrFinish to write
	 * the resulting number of frames and keyframes (10 spaces == maximum
	 * string length of an int on 32bit machines, i don't think anyone is
	 * encoding more than 4 billion frames :-)
	 */
	fprintf(f, "# ASCII XviD vbr stat file version %d\n#\n", VBR_VERSION);
	fprintf(f, "# frames    :           \n");
	fprintf(f, "# keyframes :           \n");
	fprintf(f, "#\n# quant | intra | header bytes | total bytes | kblocks |"
		" mblocks | ublocks\n\n");

	/* Save file pointer */
	state->pass1_file   = f;

	return(0);

}

static int vbr_getquant_2pass1(void *state)
{

if(state)
{
}
	return(2);

}

static int vbr_getintra_2pass1(void *state)
{
if(state)
{
}

	return(-1);

}

static int vbr_update_2pass1(void *sstate,
			     int quant,
			     int intra,
			     int header_bytes,
			     int total_bytes,
			     int kblocks,
			     int mblocks,
			     int ublocks)
			     

{

	vbr_control_t *state = sstate;

	if(state->pass1_file == NULL)
		return(-1);

	/* Writes the resulting statistics */
	fprintf(state->pass1_file, "%d %d %d %d %d %d %d\n",
		quant,
		intra,
		header_bytes,
		total_bytes,
		kblocks,
		mblocks,
		ublocks);

	/* Update vbr control state */
	if(intra) state->nb_keyframes++;
	state->nb_frames++;
	state->cur_frame++;

	return(0);
	
}

static int vbr_finish_2pass1(void *sstate)
{

	int c, i;
	vbr_control_t *state = sstate;

	if(state->pass1_file == NULL)
		return(-1);

	/* Goto to the file beginning */
	fseek(state->pass1_file, 0, SEEK_SET);

	/* Skip the version line  and the empty line */
	c = i = 0;
	do {
		c = fgetc(state->pass1_file);

		if(c == EOF) return(-1);
		if(c == '\n') i++;

	}while(i < 2);

	fseek(state->pass1_file,0,SEEK_CUR);	// Needed for win32/Mingw
	
	/* Overwrite the frame field - safe as we have written extra spaces */
	fprintf(state->pass1_file, "# frames    : %.10d\n", state->nb_frames);

	/* Overwrite the keyframe field */
	fprintf(state->pass1_file, "# keyframes : %.10d\n",
		state->nb_keyframes);

	/* Close the file */
	if(fclose(state->pass1_file) != 0)
		return(-1);

	return(0);

}

/******************************************************************************
 * 2 pass mode - 2nd pass functions (Need to be finished)
 *****************************************************************************/

static int vbr_init_2pass2(void *sstate)
{

	FILE *f;
	int c, n, pos_firstframe, credits_frames;
	long long credits1_bytes;
	long long credits2_bytes;
	long long desired;
	long long total_bytes;
	long long itotal_bytes;
	long long start_curved;
	long long end_curved;
	double total1;
	double total2;
	int precalced=0;


	vbr_control_t *state = sstate;

	/* Check the filename */
	if(state->filename == NULL || state->filename[0] == '\0')
		return(-1);

	/* Initialize safe defaults for 2pass 2 */ 
	state->pass1_file = NULL;
	state->nb_frames = 0;
	state->nb_keyframes = 0;

	/* Open the 1st pass file */
	if((f = fopen(state->filename, "r")) == NULL)
		return(-1);

	printf("XvidVBR: initialized with file :%s, pass 2\n",state->filename);
	state->pass1_file = f;

	/* Get the file version and check against current version */
	fscanf(state->pass1_file, "# ASCII XviD vbr stat file version %d\n", &n);

	if(n != VBR_VERSION) {
		fclose(state->pass1_file);
		state->pass1_file = NULL;
		return(-1);
	}

	/* Skip the blank commented line */
	c = n = 0;
	do {

		c = fgetc(state->pass1_file);

		if(c == EOF) {
			fclose(state->pass1_file);
			state->pass1_file = NULL;
			return(-1);
		}

		if(c == '\n') n++;

	}while(n < 1);


	/* Get the number of frames */
	fscanf(state->pass1_file, "# frames : %d\n", &state->nb_frames);

	/* Compute the desired size */
	state->desired_size = (long long)
		(((long long)state->nb_frames * (long long)state->desired_bitrate) /
		 (state->fps * 8.0));

	/* Get the number of keyframes */
	fscanf(state->pass1_file, "# keyframes : %d\n", &state->nb_keyframes);

	/* Allocate memory space for the keyframe_location array */
	if((state->keyframe_locations
	    = (int*)ADM_alloc((state->nb_keyframes+1)*sizeof(int))) == NULL) {
		fclose(state->pass1_file);
		state->pass1_file = NULL;
		return(-1);
	}

	/* Skip the blank commented line and the colum description */
	c = n = 0;
	do {

		c = fgetc(state->pass1_file);

		if(c == EOF) {
			fclose(state->pass1_file);
			state->pass1_file = NULL;
			return(-1);
		}

		if(c == '\n') n++;

	}while(n < 2);

	/* Save position for future use */
	pos_firstframe = ftell(state->pass1_file);
_AGAIN_:
	/* Read and initialize some variables */
	credits1_bytes = credits2_bytes = 0;
	total_bytes = itotal_bytes = 0;
	start_curved = end_curved = 0;
	credits_frames = 0;
	// MeanX
	//---------------------------------MeanX----------------------------------
	if(!precalced)
	{
		printf("nb_frames : %d\n",state->nb_frames);
		state->size=(int *)ADM_alloc(sizeof(int)*state->nb_frames);
		state->kf=(int *)ADM_alloc(sizeof(int)*state->nb_frames);
		state->type=(int *)ADM_alloc(sizeof(int)*state->nb_frames);

	// 1 read the whole file in memory
	//-----------------------------------------------------

		for(state->cur_frame=0; state->cur_frame<state->nb_frames; state->cur_frame++)
		{

			int quant, keyframe, frame_hbytes, frame_bytes;
			int kblocks, mblocks, ublocks;

			fscanf(state->pass1_file, "%d %d %d %d %d %d %d\n",
		       	&quant, &keyframe, &frame_hbytes, &frame_bytes,
			       &kblocks, &mblocks, &ublocks);

			//
			// rescale
			frame_bytes=(frame_bytes*quant)>>1;
			// In case of max allowed byterate we caps the size to avoid
			// having a ridiculously high ratio between in quant and out quant

			// ublocks is the frame type : 1 -> I / 2-> P/ 3-> B
			state->size[state->cur_frame]=frame_bytes;
			state->type[state->cur_frame]=ublocks;
			state->kf[state->cur_frame]=keyframe;
		}
	} // /precalced

	// 2- compute size


/*---------------------------------------------------------------------------------------------------------------------------*/
	for(state->cur_frame = c = 0; state->cur_frame<state->nb_frames; state->cur_frame++)
	{

		int  keyframe,  frame_bytes;

		keyframe=state->kf[state->cur_frame];
		frame_bytes=state->size[state->cur_frame];

		/* Is the frame in the beginning credits */
		if(util_frametype(state) == FRAME_TYPE_STARTING_CREDITS) {
			credits1_bytes += frame_bytes;
			credits_frames++;
			continue;
		}

		/* Is the frame in the eding credits */
		if(util_frametype(state) == FRAME_TYPE_ENDING_CREDITS) {
			credits2_bytes += frame_bytes;
			credits_frames++;
			continue;
		}

		//
		//
		/* We only care about Keyframes when not in credits */
		if(keyframe) {
			itotal_bytes +=	frame_bytes + frame_bytes *
				state->keyframe_boost / 100;
			total_bytes  += frame_bytes *
				state->keyframe_boost / 100;
			state->keyframe_locations[c++] = state->cur_frame;
		}

		total_bytes += frame_bytes;

	}

	/*-----------------------------------------------------------------------------------------*/
		printf("__________________\n");
		printf("desired size : %09lld\n",state->desired_size);
		printf("pass1   size : %09lld\n",total_bytes);
		printf("__________________\n");
	// 3- lookup cap
/*-----------------------------------------------------------------------------------
---------------------------------------------------------------------------------------*/
/* Meanx*/
	/* Here we check if there is a cap influence */
#if 0
	if(state->maxAllowedBitrate && !precalced) // maxAllowedBitrte is in BYTE/second not bit/s
	{
		float var;
		int bitrate;
		int out_bitrate[32];
		int i;
		int error,index;
		
		printf("Desired bitrate : %d\n",state->desired_bitrate);
		
		for(i=2;i<32;i++)
		{		
		vbr_make_variance(state,i,&var,&bitrate);
		out_bitrate[i]=bitrate;
		printf("Qz : %d Size: %d Ratio:%f\n",i,out_bitrate[i],(float)out_bitrate[i]/(float)state->desired_bitrate);
		}
		// search best match
		// 20% margin
		index=0;
		error=30*1000*1000;
		
		for(i=2;i<32;i++)
		{
			if(abs(out_bitrate[i]-state->desired_bitrate)<error)
			{
				error=abs(out_bitrate[i]-state->desired_bitrate);
				//printf("For quant : %d, error is %d\n",i,error);
				index=i;
			}	
		}
		printf("Best match found : Quantizer scale = %d\n",index);
		if(index)
		{
			vbr_make_clipping(state,index);
		}
		precalced=1;
		goto _AGAIN_;		
	}
		
#endif
	/*
	 * Last frame is treated like an I Frame so we can dispatch overflow
	 * all other the last film segment
	 */
	state->keyframe_locations[c] = state->cur_frame;

	/* Compensate AVI overhead */
	desired = state->desired_size -  state->nb_frames*24;

	switch(state->credits_mode) {
	case VBR_CREDITS_MODE_QUANT :

		state->movie_curve = (double)
			(total_bytes - credits1_bytes - credits2_bytes) /
			(desired  - credits1_bytes - credits2_bytes);

		start_curved = credits1_bytes;
		end_curved   = credits2_bytes;

		break;
	case VBR_CREDITS_MODE_SIZE:

		/* start curve = (start / start desired size) */
		state->credits_start_curve = (double)
			(credits1_bytes / state->credits_start_size);

		/* end curve = (end / end desired size) */
		state->credits_end_curve = (double)
			(credits2_bytes / state->credits_end_size);

		start_curved = (long long)
			(credits1_bytes / state->credits_start_curve);

		end_curved   = (long long)
			(credits2_bytes / state->credits_end_curve);

		/* movie curve=(total-credits)/(desired_size-curved credits) */
		state->movie_curve = (double)
			(total_bytes - credits1_bytes - credits2_bytes) /
			(desired - start_curved - end_curved);

		break;
	case VBR_CREDITS_MODE_RATE:
	default:

		/* credits curve = (total/desired_size)*(100/credits_rate) */
		state->credits_start_curve = state->credits_end_curve =
			((double)total_bytes / desired) *
			((double)100 / state->credits_quant_ratio);

		start_curved =
			(long long)(credits1_bytes/state->credits_start_curve);

		end_curved   =
			(long long)(credits2_bytes/state->credits_end_curve);

		state->movie_curve = (double)
			(total_bytes - credits1_bytes - credits2_bytes) /
			(desired - start_curved - end_curved);

		break;
	}

	/*
	 * average frame size = (desired - curved credits - curved keyframes) /
	 *                      (frames - credits frames - keyframes)
	 */
	state->average_frame = (double)
		(desired - start_curved - end_curved -
		 (itotal_bytes / state->movie_curve)) /
		(state->nb_frames - util_creditsframes(state) -
		 state->nb_keyframes);

	/* Initialize alt curve parameters */
	if (state->use_alt_curve) {

		state->alt_curve_low =
			state->average_frame - state->average_frame *
			(double)(state->alt_curve_low_dist / 100.0);

		state->alt_curve_low_diff =
			state->average_frame - state->alt_curve_low;

		state->alt_curve_high =
			state->average_frame + state->average_frame *
			(double)(state->alt_curve_high_dist / 100.0);

		state->alt_curve_high_diff =
			state->alt_curve_high - state->average_frame;

		if (state->alt_curve_use_auto) {

			if (state->movie_curve > 1.0)	{

				state->alt_curve_min_rel_qual =
					(int)(100.0 - (100.0 - 100.0 / state->movie_curve) *
					      (double)state->alt_curve_auto_str / 100.0);

				if (state->alt_curve_min_rel_qual < 20)
					state->alt_curve_min_rel_qual = 20;
			}
			else {
				state->alt_curve_min_rel_qual = 100;
			}

		}

		state->alt_curve_mid_qual =
		(1.0 + (double)state->alt_curve_min_rel_qual / 100.0) / 2.0;

		state->alt_curve_qual_dev = 1.0 - state->alt_curve_mid_qual;

		if (state->alt_curve_low_dist > 100) {

			switch(state->alt_curve_type) {
			case VBR_ALT_CURVE_AGGRESIVE:
				/* Sine Curve (high aggressiveness) */
				state->alt_curve_qual_dev *=
					2.0 /
					(1.0 +  sin(DEG2RAD * (state->average_frame * 90.0 / state->alt_curve_low_diff)));

				state->alt_curve_mid_qual =
					1.0 - state->alt_curve_qual_dev *
					sin(DEG2RAD * (state->average_frame * 90.0 / state->alt_curve_low_diff));
				break;

			default:
			case VBR_ALT_CURVE_LINEAR:
				/* Linear (medium aggressiveness) */
				state->alt_curve_qual_dev *=
					2.0 /
					(1.0 + state->average_frame / state->alt_curve_low_diff);

				state->alt_curve_mid_qual =
					1.0 - state->alt_curve_qual_dev *
					state->average_frame / state->alt_curve_low_diff;

				break;

			case VBR_ALT_CURVE_SOFT:
				/* Cosine Curve (low aggressiveness) */
				state->alt_curve_qual_dev *=
					2.0 /
					(1.0 + (1.0 - cos(DEG2RAD * (state->average_frame * 90.0 / state->alt_curve_low_diff))));

				state->alt_curve_mid_qual =
					1.0 - state->alt_curve_qual_dev *
					(1.0 - cos(DEG2RAD * (state->average_frame * 90.0 / state->alt_curve_low_diff)));

				break;
			}
		}
	}

	/* Go to the first non credits frame stats line into file */
	fseek(state->pass1_file, pos_firstframe, SEEK_SET);

/*-------------------------------------------------------------------------
---------------------------------------------------------------------------*/
	/* Perform prepass to compensate for over/undersizing */
	total1 = total2 = 0.0;

	for(state->cur_frame=0; state->cur_frame<state->nb_frames; state->cur_frame++)
	{

		int quant, keyframe, frame_hbytes, frame_bytes;
		int kblocks, mblocks, ublocks;

		fscanf(state->pass1_file, "%d %d %d %d %d %d %d\n",
		       &quant, &keyframe, &frame_hbytes, &frame_bytes,
		       &kblocks, &mblocks, &ublocks);
		//
		// rescale
		frame_bytes=state->size[state->cur_frame];
		keyframe=state->kf[state->cur_frame];

		if(util_frametype(state) != FRAME_TYPE_NORMAL_MOVIE)
			continue;

		if(!keyframe) {

			double dbytes = frame_bytes / state->movie_curve;
			total1 += dbytes;

			if (state->use_alt_curve) {

				if (dbytes > state->average_frame) {

					if (dbytes >= state->alt_curve_high) {
						total2 += dbytes * (state->alt_curve_mid_qual - state->alt_curve_qual_dev);
					}
					else {
						total2+=vbr_getstuff(state,dbytes, state->alt_curve_high_diff);
					}
				}
				else {

					if (dbytes <= state->alt_curve_low) {
						total2 += dbytes;
					}
					else {
						total2+=vbr_getstuff(state,dbytes, state->alt_curve_high_diff);
					}
				}
			}
			else {
				if (dbytes > state->average_frame) {
					total2 +=
						((double)dbytes +
						 (state->average_frame - dbytes) *
						 state->curve_compression_high / 100.0);
				}
				else {
					total2 +=
						((double)dbytes +
						 (state->average_frame - dbytes) *
						 state->curve_compression_low / 100.0);
				}
			}
		}
	}

	state->curve_comp_scale= total1 / total2;



	printf("Average comp ratio : %f\n",state->curve_comp_scale);
	if (state->use_alt_curve) {

		double curve_temp, dbytes;
		int newquant, percent;
		int oldquant = 1;

		if (state->alt_curve_use_auto_bonus_bias)
			state->alt_curve_bonus_bias = state->alt_curve_min_rel_qual;

		state->curve_bias_bonus =
			(total1 - total2) * (double)state->alt_curve_bonus_bias /
			(100.0 * (double)(state->nb_frames - util_creditsframes(state) - state->nb_keyframes));
		state->curve_comp_scale =
			((total1 - total2) * (1.0 - (double)state->alt_curve_bonus_bias / 100.0) + total2) /
			total2;


		for (n=1; n <= (int)(state->alt_curve_high*2) + 1; n++) {
			dbytes = n;
			if (dbytes > state->average_frame)
			{
				if (dbytes >= state->alt_curve_high) {
					curve_temp = dbytes * (state->alt_curve_mid_qual - state->alt_curve_qual_dev);
				}
				else {
					curve_temp=vbr_getstuff(state,dbytes, state->alt_curve_high_diff);
					}
			}
			else {
				if (dbytes <= state->alt_curve_low) {
					curve_temp = dbytes;
				}
				else {
				curve_temp=vbr_getstuff(state,dbytes,state->alt_curve_low_diff);

				}
			}

			if (state->movie_curve > 1.0)
				dbytes *= state->movie_curve;

			newquant = (int)(dbytes * 2.0 / (curve_temp * state->curve_comp_scale + state->curve_bias_bonus));
			if (newquant > 1)
			{
				if (newquant != oldquant)
				{
					oldquant = newquant;
					percent = (int)((n - state->average_frame) * 100.0 / state->average_frame);
				}

			}

		}

	}

	state->overflow = 0;
	state->KFoverflow = 0;
	state->KFoverflow_partial = 0;
	state->KF_idx = 1;

	for (n=0 ; n < 32 ; n++) {
		state->quant_error[n] = 0.0;
		state->quant_count[n] = 0;
	}

	state->curve_comp_error = 0.0;
	state->last_quant = 0;

	/*
	 * Above this frame size limit, normal vbr rules will not apply
	 * This means :
	 *      1 - Quant can de/increase more than -/+2 between 2 frames
	 *      2 - Leads to artifacts because of 1
	 */
	state->max_framesize = state->twopass_max_bitrate/state->fps;

	/* Get back to the beginning of frame statistics */
	fseek(state->pass1_file, pos_firstframe, SEEK_SET);

	/*
	 * Small hack : We have to get next frame stats before the
	 * getintra/quant calls
	 * User clients update the data when they call vbrUpdate
	 * we are just bypassing this because we don't have to update
	 * the overflow and so on...
	 */
	{

		/* Fake vars */
		int next_hbytes, next_kblocks, next_mblocks, next_ublocks;

		fscanf(state->pass1_file, "%d %d %d %d %d %d %d\n",
		       &state->pass1_quant, &state->pass1_intra, &next_hbytes,
		       &state->pass1_bytes, &next_kblocks, &next_mblocks,
		       &next_ublocks);

	}
	/* MeanX same as before, scale it to 2 */
		state->pass1_bytes=state->size[0];;
		state->pass1_quant=2;

	/* /MeanX*/

	/* Initialize the frame counter */
	state->cur_frame = 0;
	state->last_keyframe = 0;
	/* Reset compressibility to 0 = linear scale */
	{
		int j;
		for(j=0;j<AVG_LOOKUP;j++)
			state->compr[j]=0;
	}

	// VBV buffer init
	state->vbv_fullness=state->vbv_buffer_size;
	state->bits_per_image=(state->maxAllowedBitrate*8)/state->roundup; // Fixme, small error here
	printf("Xvid 2 pass Bitrate enforcement\n");
	printf("===============================\n");
	printf("Xvid:Using a buffer of %d kByte\n",state->vbv_buffer_size/(8*1024));
	printf("Xvid:Using a bitrate   %d kbps\n",state->maxAllowedBitrate*8/1000);
	printf("Xvid:Bits per image    %d bits\n",state->bits_per_image);
	return(0);

}

static int vbr_getquant_2pass2(void *sstate)
{
	int quant;
	int intra;
	int bytes1, bytes2;
	int overflow;
	int capped_to_max_framesize = 0;
	int KFdistance, KF_min_size;
	vbr_control_t *state;
	int i;
	int sum=0;
	int ratio;
	int target;
	int projected ;
	double r;
	int projected_vbv;
	
	int tryme,ok,orgquant;

	 state= (vbr_control_t	* )sstate;
	 target=state->cur_frame%state->roundup;

	bytes1 = state->pass1_bytes;
	overflow = state->overflow / 8;
	/* To shut up gcc warning */
	bytes2 = bytes1;


	if (state->pass1_intra)
	{
		overflow = 0;
	}

	if (util_frametype(state) != FRAME_TYPE_NORMAL_MOVIE)
	{


		switch (state->credits_mode) {
		case VBR_CREDITS_MODE_QUANT :
			if (state->credits_quant_i != state->credits_quant_p) {
				quant = state->pass1_intra ?
					state->credits_quant_i:
					state->credits_quant_p;
			}
			else {
				quant = state->credits_quant_p;
			}

			state->bytes1 = bytes1;
			state->bytes2 = bytes1;
			state->desired_bytes2 = bytes1;
			return(quant);
		default:
		case VBR_CREDITS_MODE_RATE :
		case VBR_CREDITS_MODE_SIZE :
			if(util_frametype(state) == FRAME_TYPE_STARTING_CREDITS)
				bytes2 = (int)(bytes1 / state->credits_start_curve);
			else
				bytes2 = (int)(bytes1 / state->credits_end_curve);
			break;
		}
	}
	else {
		/* Foxer: apply curve compression outside credits */
		double dbytes, curve_temp;

		bytes2 = bytes1;

		if (state->pass1_intra)
			dbytes = ((int)(bytes2 + bytes2 * state->keyframe_boost / 100)) /
				state->movie_curve;
		else
			dbytes = bytes2 / state->movie_curve;

		/* spread the compression error accross payback_delay frames */
		if (state->bitrate_payback_method == VBR_PAYBACK_BIAS)	{
			bytes2 = (int)(state->curve_comp_error / state->bitrate_payback_delay);
		}
		else {
			bytes2 = (int)(state->curve_comp_error * dbytes /
				state->average_frame / state->bitrate_payback_delay);

			if (labs(bytes2) > fabs(state->curve_comp_error))
				bytes2 = (int)state->curve_comp_error;
		}

		state->curve_comp_error -= bytes2;

		if (state->use_alt_curve) {

			if (!state->pass1_intra) {

				if (dbytes > state->average_frame) {
					if (dbytes >= state->alt_curve_high)
						curve_temp = dbytes * (state->alt_curve_mid_qual - state->alt_curve_qual_dev);
					else
					{
						curve_temp = vbr_getstuff(state,dbytes,state->alt_curve_high_diff);
					}
				}
				else {
					if (dbytes <= state->alt_curve_low)
						curve_temp = dbytes;
					else
					{
						curve_temp = vbr_getstuff(state,dbytes,state->alt_curve_low_diff);
					}
				}

				curve_temp = curve_temp * state->curve_comp_scale + state->curve_bias_bonus;

				bytes2 += ((int)curve_temp);
				state->curve_comp_error += curve_temp - ((int)curve_temp);

			}
			else {
				state->curve_comp_error += dbytes - ((int)dbytes);
				bytes2 += ((int)dbytes);
			}
		}
		else if ((state->curve_compression_high + state->curve_compression_low) &&
			!state->pass1_intra) {

			if (dbytes > state->average_frame) {
				curve_temp = state->curve_comp_scale *
					((double)dbytes + (state->average_frame - dbytes) *
					state->curve_compression_high / 100.0);
			}
			else {
				curve_temp = state->curve_comp_scale *
					((double)dbytes + (state->average_frame - dbytes) *
					state->curve_compression_low / 100.0);
			}

			bytes2 += ((int)curve_temp);
			state->curve_comp_error += curve_temp - ((int)curve_temp);
		}
		else {
			state->curve_comp_error += dbytes - ((int)dbytes);
			bytes2 += ((int)dbytes);
		}

		/* cap bytes2 to first pass size, lowers number of quant=1 frames */
		if (bytes2 > bytes1) {
			state->curve_comp_error += bytes2 - bytes1;
			bytes2 = bytes1;
		}
		else if (bytes2 < 1) {
			state->curve_comp_error += --bytes2;
			bytes2 = 1;
		}
	}

	state->desired_bytes2 = bytes2;

	/* Ugly dependance between getquant and getintra */
	intra = state->getintra(state);

	if(intra) {

		KFdistance = state->keyframe_locations[state->KF_idx] -
			state->keyframe_locations[state->KF_idx - 1];

		if (KFdistance < state->kftreshold) {
			KFdistance = KFdistance - state->min_key_interval;

			if (KFdistance >= 0) {

				KF_min_size = bytes2 * (100 - state->kfreduction) / 100;
				if (KF_min_size < 1)
					KF_min_size = 1;

				bytes2 = KF_min_size + (bytes2 - KF_min_size) * KFdistance /
					(state->kftreshold - state->min_key_interval);

				if (bytes2 < 1)
					bytes2 = 1;
			}
		}
	}

	/*
	 * Foxer: scale overflow in relation to average size, so smaller frames don't get
	 * too much/little bitrate
	 */
	overflow = (int)((double)overflow * bytes2 / state->average_frame);

	/* Foxer: reign in overflow with huge frames */
	if (labs(overflow) > labs(state->overflow)) {
		overflow = state->overflow;
	}

	/* Foxer: make sure overflow doesn't run away */
	if(overflow > bytes2 * state->twopass_max_overflow_improvement / 100) {
		bytes2 += (overflow <= bytes2) ? bytes2 * state->twopass_max_overflow_improvement / 100 :
			overflow * state->twopass_max_overflow_improvement / 100;
	}
	else if(overflow < bytes2 * state->twopass_max_overflow_degradation / -100) {
		bytes2 += bytes2 * state->twopass_max_overflow_degradation / -100;
	}
	else {
		bytes2 += overflow;
	}

	if(bytes2 > state->max_framesize) {
		capped_to_max_framesize = 1;
		bytes2 = state->max_framesize;
	}

	if(bytes2 < 1) {
		bytes2 = 1;
	}

	state->bytes1 = bytes1;
	state->bytes2 = bytes2;

	/*printf("Pass 1 : %d \n",bytes1);
	printf("Pass 2 : %d \n",bytes2);
	printf("Quant1 : %d \n",state->pass1_quant);
*/
	/* very 'simple' quant<->filesize relationship */
	quant = state->pass1_quant * bytes1 / bytes2;
//	printf("Quant2 : %d \n",quant);
	if(quant < 1)
		quant = 1;
	else if(quant > 31)
		quant = 31;
	else if(!state->pass1_intra) {

		/* Foxer: aid desired quantizer precision by accumulating decision error */
		state->quant_error[quant] += ((double)(state->pass1_quant * bytes1) / bytes2) - quant;

		if (state->quant_error[quant] >= 1.0) {
			state->quant_error[quant] -= 1.0;
			quant++;
		}
	}

	/* we're done with credits */
	if(util_frametype(state) != FRAME_TYPE_NORMAL_MOVIE) {
		return(quant);
	}

	if(intra) {

		if (quant < state->min_iquant)
			quant = state->min_iquant;
		if (quant > state->max_iquant)
			quant = state->max_iquant;
	}
	else {

		if(quant > state->max_pquant)
			quant = state->max_pquant;
		if(quant < state->min_pquant)
			quant = state->min_pquant;

		/* subsequent frame quants can only be +- 2 */
		if(state->last_quant && capped_to_max_framesize == 0) {
			if (quant > state->last_quant + 2)
				quant = state->last_quant + 2;
			if (quant < state->last_quant - 2)
				quant = state->last_quant - 2;
		}
	}

	

	// we make a rough estimation of the size this frame will be...
	

	if(state->maxAllowedBitrate && state->cur_frame+state->roundup<state->nb_frames)
	{
	//*******************************************************************************	
	// meanX
	//*******************************************************************************
	tryme=20;
	ok=0;
	
	//quant+=state->qinc;	
	orgquant=quant;
_requant:
	while(tryme && !ok)
	{
		int frame;
		
		projected_vbv=state->vbv_fullness;
 		for( i=0;i<state->roundup>>1;i++)
		{
			frame=vbr_predict(state,state->size[state->cur_frame+i]*2,quant);
			frame*=8;
			projected_vbv-=frame;
			if(projected_vbv<0)
			{
				aprintf("underflow at %d + %d\n",state->cur_frame,i);
				quant++;
				if(quant>31)
				{
					 quant=31;
					 tryme=1;
				}
				tryme--;
				goto _requant;
			}
			projected_vbv+=state->bits_per_image;
			if(projected_vbv>state->vbv_buffer_size)
			{
				projected_vbv=state->vbv_buffer_size;
			}
		}
		ok=1;
	}
	if(!ok)
	{
		printf("Could not compensate underflow!\n");
	}
	aprintf("Predicted size : %d\n",vbr_predict(state,state->size[state->cur_frame]*2,quant));
	aprintf("Orgquant:%d finalQuant:%d\n",orgquant,quant);
	if(quant-orgquant>2) state->qinc+=1+(quant-orgquant)>>1;
	if(state->qinc) state->qinc--;	
	if(quant>31) quant=31;
	}
	
	// MEANX
	return(quant);

}

static int vbr_getintra_2pass2(void *sstate)
{

	int intra=0;
	vbr_control_t *state = sstate;


	/* Get next intra state (fetched by update) */
	intra = state->pass1_intra;
	//printf("1st pass: %d \n",intra);
	/* During credits, XviD will decide itself */
	if(util_frametype(state) != FRAME_TYPE_NORMAL_MOVIE) {


		switch(state->credits_mode) {
		default:
		case VBR_CREDITS_MODE_RATE :
		case VBR_CREDITS_MODE_SIZE :
			intra = -1;
			break;
		case VBR_CREDITS_MODE_QUANT :
			/* Except in this case */
			if (state->credits_quant_i == state->credits_quant_p)
				intra = -1;
			break;
		}

	}

	/* Force I Frame when max_key_interval is reached */
	if((state->cur_frame - state->last_keyframe) > state->max_key_interval)
	{
	//	printf("Too far away -> intra cur : %d last : %d\n", 
	//		state->cur_frame,state->last_keyframe);		
		intra = 1;
	}

	/*
	 * Force P or B Frames for frames whose distance is less than the
	 * requested minimum
	 */
	if((state->cur_frame - state->last_keyframe) < state->min_key_interval)
	{
		intra = 0;
	}
	//printf("cur : %d / last %d intra  : %d min: %d max: %d\n",state->cur_frame,state->last_keyframe,intra,
	//		 state->min_key_interval, state->max_key_interval			);

	/* Return the given intra mode except for first frame */
	return((state->cur_frame==0)?1:intra);

}

static int vbr_update_2pass2(void *sstate,
			     int quant,
			     int intra,
			     int header_bytes,
			     int total_bytes,
			     int kblocks,
			     int mblocks,
			     int ublocks)


{


//	int next_hbytes, next_kblocks, next_mblocks, next_ublocks;
	int tempdiv;
	int target;
	vbr_control_t *state = sstate;

	header_bytes=total_bytes+kblocks+mblocks+ublocks; // MEAN: Remove warning
	/*
	 * We do not depend on getintra/quant because we have the real results
	 * from the xvid core
	 */

	if (util_frametype(state) == FRAME_TYPE_NORMAL_MOVIE) {

		state->quant_count[quant]++;

		if (state->pass1_intra) {

			state->overflow += state->KFoverflow;
			state->KFoverflow = state->desired_bytes2 - total_bytes;

			tempdiv = (state->keyframe_locations[state->KF_idx] -
				   state->keyframe_locations[state->KF_idx - 1]);

			/* redistribute correctly (by koepi) */
			if (tempdiv > 1) {
				/* non-consecutive keyframes */
				state->KFoverflow_partial = state->KFoverflow /
					(tempdiv - 1);
			}
			else {
				state->overflow  += state->KFoverflow;
				state->KFoverflow = 0;
				state->KFoverflow_partial = 0;
			}
			state->KF_idx++;

		}
		else {
			state->overflow += state->desired_bytes2 - total_bytes +
				state->KFoverflow_partial;
			state->KFoverflow -= state->KFoverflow_partial;
		}
	}
	else {

		state->overflow += state->desired_bytes2 - total_bytes;
		state->overflow += state->KFoverflow;
		state->KFoverflow = 0;
		state->KFoverflow_partial = 0;
	}

	/* Save old quant */
	state->last_quant = quant;
	/* Save the last Keyframe pos */
	if(intra)
		state->last_keyframe = state->cur_frame;

		target=state->cur_frame%state->roundup;
		

		if(ublocks!=state->type[state->cur_frame])
		{
				//aprintf("\n************************* GOP CHANGE******************\n");
		}

/* Ok next frame */

		state->cur_frame++;
		state->pass1_bytes=state->size[state->cur_frame];;
		state->pass1_intra=state->kf[state->cur_frame];
		state->pass1_quant=2;
/* Update vbv */
	if(state->maxAllowedBitrate && state->cur_frame<state->nb_frames-state->roundup)
	{
		int rank,old;
		float comp;
			state->vbv_fullness-=total_bytes*8;
			state->vbv_fullness+=state->bits_per_image;
			if(state->vbv_fullness>state->vbv_buffer_size)
			{
				state->vbv_fullness=state->vbv_buffer_size;
			}
			if(state->vbv_fullness<0)
			{
				printf("** Buffer undeflow: %d at frame :%d **\n",state->vbv_fullness,state->cur_frame);
				state->vbv_fullness=0;
			}
			// Update compressility
			old=state->cur_frame-1;
			rank=old%AVG_LOOKUP;
			state->compr[rank]=vbr_get_comp(state->size[old],2,total_bytes,quant);
			aprintf(">>Size : %d\n",total_bytes);
			aprintf("Buffer fullness:%d\n",100*state->vbv_fullness/state->vbv_buffer_size);
	}		

	return(0);

}

static int vbr_finish_2pass2(void *sstate)
{

	vbr_control_t *state = sstate;

	if(state->pass1_file == NULL)
		return(-1);

	/* Close the file */
	if(fclose(state->pass1_file) != 0)
		return(-1);

	/* Free the memory */
	if(state->keyframe_locations)
		ADM_dealloc(state->keyframe_locations);

	{ // meanX
		if(state->size)
			{
				ADM_dealloc(state->size);
				state->size=NULL;
			}
		if(state->kf)
			{
				ADM_dealloc(state->kf);
				state->kf=NULL;
			}
		if(state->type)
			{
				ADM_dealloc(state->type);
				state->type=NULL;
			}


	}

	return(0);

}


/******************************************************************************
 * Fixed quant mode - Most of the functions will be dummy functions
 *****************************************************************************/

static int vbr_init_fixedquant(void *sstate)
{

	vbr_control_t *state = sstate;

	if(state->fixed_quant < 1)
		state->fixed_quant = 1;

	if(state->fixed_quant > 31)
		state->fixed_quant = 31;

	state->cur_frame = 0;

	return(0);

}

static int vbr_getquant_fixedquant(void *sstate)
{

	vbr_control_t *state = sstate;

	/* Credits' frame ? */
	if(util_frametype(state) != FRAME_TYPE_NORMAL_MOVIE) {

		int quant;

		switch(state->credits_mode) {
		case VBR_CREDITS_MODE_RATE:
			quant = state->fixed_quant * state->credits_quant_ratio;
			break;
		case VBR_CREDITS_MODE_QUANT:
			quant = state->credits_fixed_quant;
			break;
		default:
			quant = state->fixed_quant;

		}

		return(quant);

	}
		
	/* No credit frame - return fixed quant */
	return(state->fixed_quant);

}
/*__________________________________________________________________

	Reverse the below formula
	newbits/oldbuts=newquang^ -comp
	log(newq^ -comp)=log(newbit/oldbits)
	comp=-log(newbits/oldbits)/log(newq/oldq)
*/
float vbr_get_comp(int oldbits, int qporg, int newbits, int qpused)
{
	float comp;
	
	comp=newbits;
	comp/=oldbits;
	comp=log(comp);
	comp/=log(qpused/qporg);
	aprintf("Old q:%d new q : %d oldBits:%d newbits:%d comp:%f\n",
			qporg,qpused,oldbits,newbits,-comp);
	return -comp;
}
/*_______________________________________________________________
	Predict the size of the image
	Using a exp(-comp) formula instead of linear formula
	
	Idea by Peter Cheat
__________________________________________________________________
*/
int vbr_predict(vbr_control_t *state,int original_size,int qp)
{
 // Peter Cheat formula :Pridicted Bits Frame 10 Will Use = (Bits Used At Quantiser 1) * (New Quantiser ^ -Compressibility)
 // avg lookup compressibility
 	int start;
	int pred,i;
	float comp=0;
		
	start=state->cur_frame;
	start+=state->roundup;
	start-=AVG_LOOKUP;
	start%=state->roundup;
	
	for(i=0;i<AVG_LOOKUP;i++)
	{
		comp+=state->compr[start];
		start++;
		start%=AVG_LOOKUP;
	}
	comp=comp/AVG_LOOKUP;
	
	pred=original_size;
	pred=pred*pow(qp/2,-comp);
	aprintf("Avg comp: %f initial size : %d predicted size:%d\n",comp,original_size,pred);
	return pred;

}
/*
	Return if next frame is intra
*/
int vbr_next_intra( void *sstate)
{

	vbr_control_t *state = sstate;

	if(state->cur_frame>state->nb_frames -2) return 0;
	return state->kf[state->cur_frame+1];

}
static int vbr_getintra_fixedquant(void *state)
{
	if(state)
	{
	}
	return(-1);

}
int vbr_getstuff(vbr_control_t *state, int dbytes,double diff)
{
	int total2=0;
	switch(state->alt_curve_type)
						{
						case VBR_ALT_CURVE_AGGRESIVE:
							total2 +=
								dbytes *
								(state->alt_curve_mid_qual - state->alt_curve_qual_dev *
								 sin(DEG2RAD * ((dbytes - state->average_frame) * 90.0 / diff)));
							break;
						default:
						case VBR_ALT_CURVE_LINEAR:
							total2 +=
								dbytes *
								(state->alt_curve_mid_qual - state->alt_curve_qual_dev *
								 (dbytes - state->average_frame) / diff);
							break;
						case VBR_ALT_CURVE_SOFT:
							total2 +=
								dbytes *
								(state->alt_curve_mid_qual + state->alt_curve_qual_dev *
								 (1.0 - cos(DEG2RAD * ((dbytes - state->average_frame) * 90.0 / diff))));
						}
	return total2;
}
/*---------------------------------------------------------------------------------------------------------------------
	
	For each quantizer given we clip size until it does not reach maxiumu
	bitrate, that way we got an good average size value to decide
	which quantizer is good depending on the max bitrate
	
-----------------------------------------------------------------------------------------------------------------------
*/
int  vbr_make_variance(vbr_control_t *state, float compression, int *variance, int *bitrate)
{
		
		float *cur_mod=NULL;
		float *final_mod=NULL;
		int roundup;
		int nb_maxxed;
		
		int i,tmp;
		double ibitrate, instant;
		int try;
		double d_average,d_variance,v;
		long long int in_sum, out_sum;
		
		float compr_out;
					
		roundup=state->roundup;	
		cur_mod=ADM_alloc(sizeof(float)*state->nb_frames);
		final_mod=ADM_alloc(sizeof(float)*state->nb_frames);
		
		
		for(i=0;i<state->nb_frames;i++)
		{						
			final_mod[i]=2/compression;			
		}
		
		// Calculate bitrate
		for(try=0;try<5;try++)
		{		
		ibitrate=0;
		nb_maxxed=0;
		
		// Reset
		for(tmp=0;tmp<state->nb_frames;tmp++)
		{
			cur_mod[tmp]=1.;			
		}
		
		for(i=roundup;i<(state->nb_frames-roundup);i++)
		{
			instant=state->size[i-(roundup>>1)];
			instant*=final_mod[i-(roundup>>1)];
			ibitrate-=instant;
			
			instant=state->size[i+(roundup>>1)];
			instant*=final_mod[i+(roundup>>1)];
			ibitrate+=instant;
			
			if(ibitrate<10.) ibitrate=10.;			
			//printf("Iteration %0d Bitrate at %d : %03.0f",try,i,(ibitrate*8)/1000);			
			if(ibitrate> state->maxAllowedBitrate)
			{
				nb_maxxed++;				
			//	printf(" Maxx!");
				for(tmp=-(roundup>>1);tmp<(roundup>>1);tmp++)
				{
					cur_mod[i+tmp]=0.8;
				}
			}
			//printf("\n");
		}
		for(i=roundup;i<(state->nb_frames-roundup);i++)
		{
			final_mod[i]*=cur_mod[i];
		}
		} // end try
		/*for(i=roundup;i<(state->nb_frames-roundup);i++)
		{
			tmp=(int)floor(final_mod[i]*compression*100.);	
			printf("Final mod  %d %d %%\n",
					i,tmp);
			
		}*/
		d_average=0;
		d_variance=0;
		in_sum=0;
		out_sum=0;
		//___________________________
		// compute average  && variance
		//___________________________
		for(i=0;i<state->nb_frames;i++)
		{
			in_sum+=state->size[i];
			out_sum+=(state->size[i]*final_mod[i]);			
		}		
		d_average=out_sum;
		d_average/=state->nb_frames;
		//printf("Average bitrate out : %f\n",(d_average*state->fps*8)/1000);
		*bitrate=floor(d_average*state->fps*8);
//		printf("Average bitrate out : %d\n",*bitrate);
		
		for(i=0;i<(state->nb_frames);i++)
		{
			v=state->size[i]*final_mod[i]-d_average;
			d_variance+=abs(v);	
		}		
		d_variance/=state->nb_frames;			
		*variance=floor(d_variance);
				
		//printf("in : %lld		\n",in_sum);
		//printf("out : %lld		\n",out_sum);
		compr_out=in_sum;
		compr_out/=out_sum;
		
		//printf("Compression in /out : %f /  %f\n",compression, compr_out);
		//printf("Variance : %f\n",d_variance);
		ADM_dealloc(cur_mod);		
		ADM_dealloc(final_mod);		
		return 0;
}	
/*
	Same as above except this time we alter the size to reflect the
	max bitrate (on the original size)

*/	
int  vbr_make_clipping(vbr_control_t *state, float compression)
{
		
		float *cur_mod=NULL;
		float *final_mod=NULL;
		int roundup;
		int nb_maxxed;
		
		int i,tmp;
		double ibitrate, instant;
		int try;
		int before,after;
					
		roundup=state->roundup;	
		cur_mod=ADM_alloc(sizeof(float)*state->nb_frames);
		final_mod=ADM_alloc(sizeof(float)*state->nb_frames);
		
		
		for(i=0;i<state->nb_frames;i++)
		{			
			
			final_mod[i]=2./compression;			
		}
		
		// Calculate bitrate
		for(try=0;try<5;try++)
		{		
		ibitrate=0;
		nb_maxxed=0;
		
		// Reset
		for(tmp=0;tmp<state->nb_frames;tmp++)
		{
			cur_mod[tmp]=1.;			
		}
		
		for(i=roundup;i<(state->nb_frames-roundup);i++)
		{
			instant=state->size[i-(roundup>>1)];
			instant*=final_mod[i-(roundup>>1)];
			ibitrate-=instant;
			
			instant=state->size[i+(roundup>>1)];
			instant*=final_mod[i+(roundup>>1)];
			ibitrate+=instant;
			
			if(ibitrate<10.) ibitrate=10.;			
			//printf("Iteration %0d Bitrate at %d : %03.0f",try,i,(ibitrate*8)/1000);			
			if(ibitrate> state->maxAllowedBitrate)
			{
				nb_maxxed++;				
			//	printf(" Maxx!");
				for(tmp=-(roundup>>1);tmp<(roundup>>1);tmp++)
				{
					cur_mod[i+tmp]=0.8;
				}
			}
			//printf("\n");
		}
		for(i=roundup;i<(state->nb_frames-roundup);i++)
		{
			final_mod[i]*=cur_mod[i];
		}
		} // end try
		for(i=roundup;i<(state->nb_frames-roundup);i++)
		{
			before=state->size[i];
			state->size[i]=state->size[i]*final_mod[i]*compression/2;
			after=state->size[i];
			//printf("%d --> %d (%f)\n",before,after,final_mod[i]*50);
		}
		ADM_dealloc(cur_mod);		
		ADM_dealloc(final_mod);	
		//exit(0);	
		return 0;		
}		
