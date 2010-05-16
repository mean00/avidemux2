/*
 *  TwoLAME: an optimized MPEG Audio Layer Two encoder
 *
 *  Copyright (C) 2001-2004 Michael Cheng
 *  Copyright (C) 2004-2005 The TwoLAME Project
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 */

#include "ADM_default.h"
#include "twolame.h"
#include "common.h"
#include "bitbuffer.h"
#include "mem.h"
#include "crc.h"
#include "dab.h"
#include "psycho_n1.h"
#include "psycho_0.h"
#include "psycho_1.h"
#include "psycho_2.h"
#include "psycho_3.h"
#include "psycho_4.h"
#include "availbits.h"
#include "subband.h"
#include "encode.h"
#include "energy.h"
#include "util.h"


// Returns 0 if successful
// Returns -1 if unsuccessful
static int init_header_info( twolame_options *glopts ) {
	frame_header *header = &glopts->header;
	
	/* use the options to create header info structure */
	header->lay = 2;
	header->error_protection = glopts->error_protection;
	header->version = glopts->version;
	
	// Convert the sampling frequency to the required index
	header->samplerate_idx = twolame_get_samplerate_index( glopts->samplerate_out ); 
	if (header->samplerate_idx < 0) {
		fprintf(stdout,"Not a valid samplerate: %i\n",glopts->samplerate_out );
		return -1;
	}
	
	// Convert the bitrate to the an index	
	header->bitrate_index = twolame_get_bitrate_index(glopts->bitrate, header->version);
	if (header->bitrate_index < 0) {
		fprintf(stdout,"Not a valid bitrate (%i) for MPEG version '%s'\n", glopts->bitrate, 
			twolame_mpeg_version_name(glopts->version));
		return -1;
	}
		
	// Convert the max VBR bitrate to the an index	
	glopts->vbr_upper_index = twolame_get_bitrate_index(glopts->vbr_max_bitrate, header->version);
	if (glopts->vbr_upper_index < 0) {
		fprintf(stdout,"Not a valid max VBR bitrate for this version: %i\n",glopts->vbr_max_bitrate);
		return -1;
	}
	
	// Copy accross the other settings	
	header->padding = glopts->padding;
	header->private_bit = glopts->private_bit;
	header->mode = glopts->mode;
	header->mode_ext = 0;
	header->copyright = glopts->copyright;
	header->original = glopts->original;
	header->emphasis = glopts->emphasis;
	
	return 0;
}



// Returns 0 if successful
// Returns -1 if unsuccessful
static int init_frame_info(twolame_options *glopts)
/* interpret data in hdr str to fields in frame */
{
	frame_info *frame = &glopts->frame;
	frame_header *header = &glopts->header;
	
	frame->actual_mode = header->mode;
	frame->nch = (header->mode == TWOLAME_MONO) ? 1 : 2;
	
	//frame->sblimit = pick_table ( glopts );
	/* MFC FIX this up */
	// Select table number and sblimit
	encode_init( glopts );
	
	if (glopts->mode == TWOLAME_JOINT_STEREO)
		frame->jsbound = get_js_bound(header->mode_ext);
	else
		frame->jsbound = frame->sblimit;
	/* alloc, tab_num set in pick_table */
	
	return 0;
}


/*
  twolame_init
  Create a set of encoding options and return a pointer to this structure
  
  Returns NULL if unsuccessful (can't allocate memory)
  Otherwise returns pointer to memory block
*/
twolame_options *twolame_init(void) {
	twolame_options *newoptions = NULL;  
	
	newoptions = (twolame_options *)twolame_malloc(sizeof(twolame_options), "twolame_options");
	if (newoptions==NULL) {
		return NULL;
	}
	
	newoptions->version = -1;
	newoptions->num_channels = 0;
	newoptions->samplerate_in = 0;
	newoptions->samplerate_out = 0;
	
	newoptions->mode = TWOLAME_AUTO_MODE;	// Choose a proper mode later
	newoptions->psymodel = 3;
	newoptions->bitrate = -1;				// Default bitrate is set in init_params
	newoptions->vbr = FALSE;
	newoptions->vbrlevel = 5.0;
	newoptions->athlevel = 0.0;
	
	newoptions->quickmode = FALSE;
	newoptions->quickcount = 10;
	newoptions->emphasis = TWOLAME_EMPHASIS_N;
	newoptions->private_bit = 0;
	newoptions->copyright = FALSE;
	newoptions->original = TRUE;
	newoptions->error_protection = FALSE;
	newoptions->padding = TWOLAME_PAD_NO;
	newoptions->do_dab = FALSE;
	newoptions->dab_crc_len = 2;
	newoptions->dab_xpad_len = 0;
	newoptions->verbosity = 2;
	newoptions->vbr_upper_index = 0;
	
	newoptions->scale = 1.0;			// scaling disabled
	newoptions->scale_left = 1.0;		// scaling disabled
	newoptions->scale_right = 1.0;		// scaling disabled
	
	newoptions->do_energy_levels = FALSE;
	newoptions->num_ancillary_bits = 0;
	
	
	newoptions->vbr_frame_count = 0;	// only used for debugging
    newoptions->tablenum = 0;	

	newoptions->twolame_init = 0;
	newoptions->subband = NULL;
	newoptions->j_sample = NULL;
	newoptions->sb_sample = NULL;
	newoptions->psycount = 0;
	newoptions->crc = 0;

	newoptions->p0mem = NULL;
	newoptions->p1mem = NULL;
	newoptions->p2mem = NULL;
	newoptions->p3mem = NULL;
	newoptions->p4mem = NULL;

    memset(newoptions->vbrstats, 0, sizeof(newoptions->vbrstats));
	
	return(newoptions);
}


/**
 * This function should actually *check* the parameters to see if they
 * make sense. 
 */

int twolame_init_params (twolame_options *glopts) {

	if (glopts->twolame_init) {
		fprintf(stderr,"Already called twolame_init_params() once.\n");
		return 1;
	}

	// Check the number of channels
	if (glopts->num_channels != 1 && glopts->num_channels != 2) {
		fprintf(stderr,"twolame_init_params(): must specify number of channels in input samples using twolame_set_num_channels().\n");
		return -1;
	}

	
	// If not output samplerate has been set, then set it to the input sample rate
	if (glopts->samplerate_out < 1) {
		glopts->samplerate_out = glopts->samplerate_in;
	}
	
	// If the MPEG version has not been set, then choose automatically
	if (glopts->version == -1) {
		// Get the MPEG version for the chosen samplerate
		glopts->version = twolame_get_version_for_samplerate( glopts->samplerate_out );
		if (glopts->version < 0) {
			fprintf(stdout,"twolame_init_params(): invalid samplerate: %i\n", glopts->samplerate_out );
			return -1;
		} else if (glopts->verbosity>=3) {
			fprintf(stderr, "Chosen version '%s' for samplerate of %d Hz.\n",
				twolame_mpeg_version_name(glopts->version), glopts->samplerate_out);
		}
	}

	// Choose mode (if none chosen)
	if (glopts->mode == TWOLAME_AUTO_MODE) {
		if (glopts->num_channels == 2) glopts->mode = TWOLAME_STEREO;
		else glopts->mode = TWOLAME_MONO;
		if (glopts->verbosity>=3) {
			fprintf(stderr, "Chosen mode to be '%s' because of %d input channels.\n",
				twolame_get_mode_name(glopts), glopts->num_channels);
		}
	}

	// Choose the bitrate (if none chosen)
	if (glopts->bitrate <= 0) {
		if (glopts->mode == TWOLAME_MONO) {
			switch(glopts->samplerate_out) {
				case 48000: glopts->bitrate = 96; break;	// (LAME=64)
				case 44100: glopts->bitrate = 96; break;	// (LAME=64)
				case 32000: glopts->bitrate = 80; break;	// (LAME=48)
				case 24000: glopts->bitrate = 48; break;	// (LAME=32)
				case 22050: glopts->bitrate = 48; break;	// (LAME=32)
				case 16000: glopts->bitrate = 32; break;	// (LAME=24)
			}
		} else {
			switch(glopts->samplerate_out) {
				case 48000: glopts->bitrate = 192; break;	// (LAME=128)
				case 44100: glopts->bitrate = 192; break;	// (LAME=128)
				case 32000: glopts->bitrate = 160; break;	// (LAME=96)
				case 24000: glopts->bitrate = 96; break;	// (LAME=64)
				case 22050: glopts->bitrate = 96; break;	// (LAME=64)
				case 16000: glopts->bitrate = 64; break;	// (LAME=48)
			}
		}
		if (glopts->verbosity>=3) {
			fprintf(stderr, "Chosen bitrate of %dkbps for samplerate of %d Hz.\n",
				glopts->bitrate, glopts->samplerate_out);
		}
	}
	
	

	/* Can't do DAB and energylevel extensions at the same time
	   Because both of them think they're the only ones inserting
	   information into the ancillary section of the frame */
	if (glopts->do_dab && glopts->do_energy_levels) {
		fprintf(stderr,"Can't do DAB and Energy Levels at the same time\n");
		return -1;
	}

	
	/* Check that if we're doing energy levels, that there's enough space 
	   to put the information */
	if (glopts->do_energy_levels) {
		if ((glopts->mode==TWOLAME_MONO)&&(glopts->num_ancillary_bits<16)) {
			fprintf(stderr,"Too few ancillary bits: %i<16\n",glopts->num_ancillary_bits);
			glopts->num_ancillary_bits = 16;
		}    
		if ((glopts->mode!=TWOLAME_MONO)&&(glopts->num_ancillary_bits<40)) {
			fprintf(stderr,"Too few ancillary bits: %i<40\n",glopts->num_ancillary_bits);
		glopts->num_ancillary_bits = 40;
		}
	}

	/*
	 * MFC Feb 2003: in VBR mode, joint
	 * stereo doesn't make any sense at
	 * the moment, as there are no noisy
	 * subbands according to
	 * bits_for_nonoise in vbr mode
	 */
	if (glopts->vbr && glopts->mode==TWOLAME_JOINT_STEREO) {
		// force stereo mode
		twolame_set_mode(glopts, TWOLAME_STEREO);
	}

	/* don't use padding for VBR */
	if (glopts->vbr) {
		twolame_set_padding(glopts, FALSE);
	}
	
	//MFC FIX:	Need to cross validate the number of ancillary_bits
	// with the energylevel setting.
	//
	// energylevel:
	// MFC FIX:		This option must be mutually exclusive with the
	// reservebits(-R) option *UNLESS * the number
	// of explicitly reserved bits > 5 bytes.


	// If energy information is required, see if we're in MONO mode in
	// which case, we only need 16 bits of ancillary data
	if (glopts->do_energy_levels) {
		if (glopts->mode==TWOLAME_MONO) {
			// only 2 bytes needed for energy level for mono channel
			twolame_set_num_ancillary_bits(glopts, 16);
		} else {
			// 5 bytes for the stereo energy info
			twolame_set_num_ancillary_bits(glopts, 40);
		}
	
	}
	


	// build mpeg header from parameters
	if (init_header_info( glopts )<0) {
		return -1;
	}
	
	// load the alloc tables and do some other stuff
	if (init_frame_info( glopts )<0) {
		return -1;
	}
	
	// initialise bitrate allocation
	if (init_bit_allocation( glopts )<0) {
		return -1;
	}
	
	// Check input samplerate is same as output samplerate
	if (glopts->samplerate_out != glopts->samplerate_in) {
		fprintf(stderr,"twolame_init_params(): sorry, twolame doesn't support resampling (yet).\n");
		return -1;
	}



	// Initialise interal variables
	glopts->samples_in_buffer = 0;
	glopts->psycount = 0;
	glopts->crc = 0;

  
  	// Allocate memory to larger buffers 
    glopts->subband = (subband_t *) twolame_malloc (sizeof (subband_t), "subband");
	glopts->j_sample = (jsb_sample_t *) twolame_malloc (sizeof (jsb_sample_t), "j_sample");
    glopts->sb_sample = (sb_sample_t *) twolame_malloc (sizeof (sb_sample_t), "sb_sample");
	
	// clear buffers
    memset ((char *) glopts->buffer, 0, sizeof(glopts->buffer));
    memset ((char *) glopts->bit_alloc, 0, sizeof (glopts->bit_alloc));
    memset ((char *) glopts->scfsi, 0, sizeof (glopts->scfsi));
    memset ((char *) glopts->scalar, 0, sizeof (glopts->scalar));
    memset ((char *) glopts->j_scale, 0, sizeof (glopts->j_scale));
    memset ((char *) glopts->smrdef, 0, sizeof (glopts->smrdef));
    memset ((char *) glopts->smr, 0, sizeof (glopts->smr));
    memset ((char *) glopts->max_sc, 0, sizeof (glopts->max_sc));

	// Initialise subband windowfilter
    if (init_subband(&glopts->smem)<0) {
		return -1;
	}

	// All initalised now :)
	glopts->twolame_init++;

	return(0);
}

/* Scale the samples in the frame sample buffer
   using the user specified values
   and downmix/upmix according to the number of input/output channels 
*/
static void scale_and_mix_samples( twolame_options *glopts )
{
	frame_info *frame = &glopts->frame;
	int num_samples = glopts->samples_in_buffer;
	int i;
	
	// apply scaling to both channels 
    if (glopts->scale != 0 && glopts->scale != 1.0) {
		for (i=0 ; i<num_samples; ++i) {
	    	glopts->buffer[0][i] *= glopts->scale;
	    	if (glopts->num_channels == 2)
			glopts->buffer[1][i] *= glopts->scale;
	    }
    }

    // apply scaling to channel 0 (left) 
    if (glopts->scale_left != 0 && glopts->scale_left != 1.0) {
		for (i=0 ; i<num_samples; ++i) {
			glopts->buffer[0][i] *= glopts->scale_left;
	    }
    }

    // apply scaling to channel 1 (right) 
	if (glopts->scale_right != 0 && glopts->scale_right != 1.0) {
	    for (i=0 ; i<num_samples; ++i) {
			glopts->buffer[1][i] *= glopts->scale_right;
	    }
	}

    // Downmix to Mono if 2 channels in and 1 channel out 
	if (glopts->num_channels == 2 && frame->nch == 1) {
		for (i=0; i<num_samples; ++i) {
			glopts->buffer[0][i] = ((long) glopts->buffer[0][i] + glopts->buffer[1][i]) / 2;
			glopts->buffer[1][i] = 0;
		}
	}

   // Upmix to Stereo if 2 channels out and 1 channel in
	if (glopts->num_channels == 1 && frame->nch == 2) {
		for (i=0; i<num_samples; ++i) {
			glopts->buffer[1][i] = glopts->buffer[0][i];
		}
	}

}
// MEANX

static unsigned int CRC_update2(unsigned int value, unsigned int crc,unsigned int nbBit)
{
    int i;
    value <<= 8;
    for (i = 0; i < nbBit; i++) {
	value <<= 1;
	crc <<= 1;

	if (((crc ^ value) & 0x10000))
	    crc ^= CRC16_POLYNOMIAL;
    }
    return crc;
}


static void
CRC_writeheader(int lenInBits, unsigned char *header)
{
    unsigned int crc = 0xffff; /* (jo) init crc16 for error_protection */
    int i,top;
    int round;

    crc = CRC_update2(header[2], crc,8);
    crc = CRC_update2(header[3], crc,8);
    
    top=6;
    round=lenInBits>>3;

    for(i=0;i<round;i++)
    {
        crc = CRC_update2(header[top++], crc,8);
    }
     if(lenInBits & 7)
     {
        crc = CRC_update2(header[top], crc,lenInBits&7);
    }
//     printf("%d Bits Old %02x%02x new %02x%02x\n",l,header[4],header[5],
//         (unsigned char)(crc>>8),(unsigned char)(crc & 0xff));
    header[4] = crc >> 8;
    header[5] = crc & 255;
}
// MEANX

/*
	Encode a single frame of audio from 1152 samples
	Audio samples are taken from glopts->buffer
	Encoded bit stream is placed in to parameter bs
	(not intended for use outside the library)
	
	Returns the size of the frame
	or -1 if there is an error
*/
static int encode_frame(twolame_options *glopts, bit_stream *bs)
{
	int nch = glopts->frame.nch;
	int sb, ch, adb, i;
	unsigned long frameBits, initial_bits;
	short sam[2][1056];
// MEANX
        int head1,head2;
        

// /MEANX
	if (!glopts->twolame_init) {
		fprintf (stderr, "Please call twolame_init_params() before starting encoding.\n");
		return -1;
	}
	
	// Scale and mix the input buffer
	scale_and_mix_samples( glopts );
	
	
    // Clear the saved audio buffer
    memset ((char *) sam, 0, sizeof (sam));

	/* Store the number of bits initially in the bit buffer */
	initial_bits = buffer_sstell(bs);

	adb = available_bits ( glopts );

    /* allow the user to reserve some space at the end of the frame
       This will however leave fewer bits for the audio.
       Need to do a sanity check here to see that there are *some* 
       bits left. */
	if (glopts->num_ancillary_bits > 0.6*adb) {
		/* Trying to reserve more than 60% of the frame.
		   0.6 is arbitrary. but since most applications
		   probably only want to reserve a few bytes, this seems fine.
		   Typical frame size is about 800bytes */
		fprintf(stderr,"You're trying to reserve more than 60%% of the mpeg frame for ancillary data\n");
		fprintf(stderr,"This is probably an error. But I'll keep going anyway...\n");
    } 

	adb -= glopts->num_ancillary_bits;


    /* MFC 26 July 2003
       Doing DAB became a bit harder in the reorganisation of the code.
       Now there is no guarantee that there is more than one frame in the bitbuffer.
       But DAB requires that the CRC for the *current* frame be written at the end 
       of the *previous* frame.
       Workaround: Users (Nicholas?) wanting to implement DAB will have to do some work
       in the frontend.
            First: Reserve some bits for yourself (options->num_ancillary_bits)
	    Second: Put the encoder into "single frame mode" i.e. only read 1152 samples
	            per channel. (frontendoptions->singleFrameMode)
	    Third: When you receive each mp2 frame back from the library, you'll have to insert
	           the options->dabCrc[i] values into the end of the frame yourself. (DAB crc calc is 
		   done below)
       The frontend will have to keep the previous frame in memory. 
       As of 26July all that needs to be done is for the frontend to buffer one frame in memory, such that
       the CRC for the next frame can be written in at the end of it.
    */

    {
		int gr, bl, ch;
		/* New polyphase filter
		 Combines windowing and filtering. Ricardo Feb'03 */
		for( gr = 0; gr < 3; gr++ )
		for ( bl = 0; bl < 12; bl++ )
		for ( ch = 0; ch < nch; ch++ )
			window_filter_subband( &glopts->smem, &glopts->buffer[ch][gr * 12 * 32 + 32 * bl], ch,
				 &(*glopts->sb_sample)[ch][gr][bl][0] );
    }

    scalefactor_calc(*glopts->sb_sample, glopts->scalar, nch, glopts->frame.sblimit);
    find_sf_max (glopts, glopts->scalar, glopts->max_sc);
    if (glopts->frame.actual_mode == TWOLAME_JOINT_STEREO) {
		/* this way we calculate more mono than we need */
		/* but it is cheap */
		combine_lr (*glopts->sb_sample, *glopts->j_sample, glopts->frame.sblimit);
		scalefactor_calc (glopts->j_sample, &glopts->j_scale, 1, glopts->frame.sblimit);
    }

    if ((glopts->quickmode == TRUE) && (++glopts->psycount % glopts->quickcount != 0)) {
		/* We're using quick mode, so we're only calculating the model every
		 'quickcount' frames. Otherwise, just copy the old ones across */
		for (ch = 0; ch < nch; ch++) {
			for (sb = 0; sb < SBLIMIT; sb++) {
				glopts->smr[ch][sb] = glopts->smrdef[ch][sb];
			}
		}
	} else {
		/* calculate the psymodel */
		switch (glopts->psymodel) {
			case -1:
				psycho_n1 (glopts, glopts->smr, nch);
			break;
			case 0:	/* Psy Model A */
				psycho_0 (glopts, glopts->smr, glopts->scalar);	
			break;
			case 1:
				psycho_1 (glopts, glopts->buffer, glopts->max_sc, glopts->smr);
			break;
			case 2:
				psycho_2 (glopts, glopts->buffer, sam, glopts->smr );	
			break;
			case 3:
				/* Modified psy model 1 */
				psycho_3 (glopts, glopts->buffer, glopts->max_sc, glopts->smr);
			break;
			case 4:
				psycho_4 (glopts, glopts->buffer, sam, glopts->smr );
			break;	
			default:
				fprintf (stderr, "Invalid psy model specification: %i\n", glopts->psymodel);
				return -1;
			break;
		}

		if (glopts->quickmode == TRUE) {
			/* copy the smr values and reuse them later */
			for (ch = 0; ch < nch; ch++) {
				for (sb = 0; sb < SBLIMIT; sb++) glopts->smrdef[ch][sb] = glopts->smr[ch][sb];
			}
		}
	}


    sf_transmission_pattern (glopts, glopts->scalar, glopts->scfsi);
    main_bit_allocation (glopts, glopts->smr, glopts->scfsi, glopts->bit_alloc, &adb);
#if 0 //MEANX
    if (glopts->error_protection)
      crc_calc (glopts, glopts->bit_alloc, glopts->scfsi, &glopts->crc);
#endif
    write_header (glopts, bs);
	if (glopts->error_protection)
		buffer_putbits (bs, glopts->crc, 16);
    
    head1=write_bit_alloc (glopts, glopts->bit_alloc, bs);

    head2=write_scalefactors(glopts, glopts->bit_alloc, glopts->scfsi, glopts->scalar, bs);


    
   //MEANX  printf("Bits: %d + %d = %d\n",head1,head2,head1+head2);
    subband_quantization (glopts, glopts->scalar, *glopts->sb_sample, glopts->j_scale, *glopts->j_sample, glopts->bit_alloc,
    			  *glopts->subband);
    write_samples(glopts, *glopts->subband, glopts->bit_alloc, bs);

    /* If not all the bits were used, write out a stack of zeros */
    for (i = 0; i < adb; i++)	buffer_put1bit (bs, 0);


    /* MFC July 03 
       Write an extra byte for 16/24/32/48 input when padding is on.
       Something must be going astray with the frame size calcs.
       This fudge works fine for the moment */
    if ((glopts->header.samplerate_idx != 0) && (glopts->padding)) // i.e. not a 44.1 or 22kHz input file
		buffer_putbits (bs, 0, 8);

    if (glopts->do_dab) {
		// Do the CRC calc for DAB stuff if required.
		// It will be up to the frontend to insert it into the end of the 
		// previous frame.
		for (i=glopts->dab_crc_len-1; i>=0; i--) {
			dab_crc_calc (glopts, glopts->bit_alloc, glopts->scfsi, glopts->scalar, &glopts->dab_crc[i], i);
		}
    }


	// Allocate space for the reserved ancillary bits
	for (i=0; i<glopts->num_ancillary_bits;i++) 
		buffer_put1bit(bs, 0);


	// Calulate the number of bits in this frame
    frameBits = buffer_sstell(bs) - initial_bits;
    if (frameBits % 8) {	/* a program failure */
		fprintf (stderr, "Sent %ld bits = %ld slots plus %ld\n", frameBits, frameBits/8, frameBits%8);
		fprintf (stderr, "If you are reading this, the program is broken\n");
		fprintf (stderr, "email %s with the command line arguments and other info\n", "noone@none.org");
		return -1;
    }

    //fprintf(stdout,"Frame size: %li\n\n",frameBits/8);

	// *** FIXME currently broken njh Sept 04
    if (glopts->do_energy_levels)
    	do_energy_levels(glopts, bs);

  // MEANX: Recompute checksum
  if (glopts->error_protection)
  {
      unsigned char *begin=(initial_bits>>3)+bs->buf;
      int protectedBits=head1+head2;

           CRC_writeheader(protectedBits, begin);
  }

	return frameBits/8;
}



/*
  glopts
  leftpcm - holds left channel (or mono channel)
  rightpcm - d'uh
  num_samples - the number of samples in each channel
  mp2buffer - a pointer to the place where we want the mpeg data to be written
  mp2buffer_size - how much space the user allocated for this buffer
  mp2fill_size - how much mpeg data the library has put into the mp2buffer 
*/

int twolame_encode_buffer(
		twolame_options *glopts,
		const short int leftpcm[],
		const short int rightpcm[],
		int num_samples,
		unsigned char *mp2buffer,
		int mp2buffer_size )
{
	int mp2_size=0;
    bit_stream mybs;
	int i;

	if (num_samples==0) return 0;

	// now would be a great time to validate the size of the buffer.
	// samples/1152 * sizeof(frame) < mp2buffer_size 
    buffer_init( mp2buffer, mp2buffer_size, &mybs );


	// Use up all the samples in in_buffer
	while( num_samples ) {
	
		// fill up glopts->buffer with as much as we can
		int samples_to_copy = 1152 - glopts->samples_in_buffer;
		if (num_samples < samples_to_copy) samples_to_copy = num_samples;

		/* Copy across samples */
		for(i=0; i<samples_to_copy; i++) {
			glopts->buffer[0][glopts->samples_in_buffer+i] = *leftpcm++;
			if (glopts->num_channels==2)
			glopts->buffer[1][glopts->samples_in_buffer+i] = *rightpcm++;
		}
		
		
		/* Update sample counts */
		glopts->samples_in_buffer += samples_to_copy;
		num_samples -= samples_to_copy;
		
		
		// is there enough to encode a whole frame ?
		if (glopts->samples_in_buffer >= 1152) {
			int bytes = encode_frame( glopts, &mybs );
			if (bytes<=0) {
				return bytes;
			}
			mp2_size += bytes;
			glopts->samples_in_buffer -= 1152;
		}
	}

	return(mp2_size);
}


int twolame_encode_buffer_interleaved(
		twolame_options *glopts,
		const short int pcm[],
		int num_samples,
		unsigned char *mp2buffer,
		int mp2buffer_size )
{
	int mp2_size=0;
    bit_stream mybs;
	int i;

	if (num_samples==0) return 0;


	// now would be a great time to validate the size of the buffer.
	// samples/1152 * sizeof(frame) < mp2buffer_size 
    buffer_init( mp2buffer, mp2buffer_size, &mybs );

	// Use up all the samples in in_buffer
	while( num_samples ) {
	
		// fill up glopts->buffer with as much as we can
		int samples_to_copy = 1152 - glopts->samples_in_buffer;
		if (num_samples < samples_to_copy) samples_to_copy = num_samples;

		/* Copy across samples */
		for(i=0; i<samples_to_copy; i++) {
			glopts->buffer[0][glopts->samples_in_buffer+i] = *pcm++;
			if (glopts->num_channels==2)
			glopts->buffer[1][glopts->samples_in_buffer+i] = *pcm++;
		}
		
		
		/* Update sample counts */
		glopts->samples_in_buffer += samples_to_copy;
		num_samples -= samples_to_copy;
		
		
		// is there enough to encode a whole frame ?
		if (glopts->samples_in_buffer >= 1152) {
			int bytes = encode_frame( glopts, &mybs );
			if (bytes<=0) {
				return bytes;
			}
			mp2_size += bytes;
			glopts->samples_in_buffer -= 1152;
		}
	}	
	
	return(mp2_size);
}


int twolame_encode_flush(twolame_options *glopts, unsigned char *mp2buffer, int mp2buffer_size) {
    bit_stream mybs;
	int mp2_size = 0;
	int i;

	if (glopts->samples_in_buffer==0) {
		// No samples left over
		return 0;
	}


	// Create bit stream structure
    buffer_init( mp2buffer, mp2buffer_size, &mybs );

	// Pad out the PCM buffers with 0 and encode the frame
	for (i=glopts->samples_in_buffer; i< 1152; i++) {
		glopts->buffer[0][i] = glopts->buffer[1][i] = 0;
	}

	// Encode the frame 
	mp2_size = encode_frame( glopts, &mybs );
	glopts->samples_in_buffer=0;

	return mp2_size;
}




void twolame_close(twolame_options **glopts) {
	twolame_options *opts = NULL;

	// Check input pointers aren't NULL
	if (glopts==NULL) return;
	opts = *glopts;
	if (opts==NULL) return;

	// free mem
	if (opts->p4mem) psycho_4_deinit( &opts->p4mem );
	if (opts->p3mem) psycho_3_deinit( &opts->p3mem );
	if (opts->p2mem) psycho_2_deinit( &opts->p2mem );
	if (opts->p1mem) psycho_1_deinit( &opts->p1mem );
	if (opts->p0mem) psycho_0_deinit( &opts->p0mem );
	
	if (opts->subband) twolame_free( (void **) &opts->subband );
	if (opts->j_sample) twolame_free( (void **) &opts->j_sample );
	if (opts->sb_sample) twolame_free( (void **) &opts->sb_sample );
	
	// Free the memory and zero the pointer
	twolame_free ( (void **)glopts );
}
