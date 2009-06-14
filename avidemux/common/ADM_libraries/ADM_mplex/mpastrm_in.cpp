/*
 *  audiostrm_in.c: MPEG Audio strem class members handling scanning
 *  and buffering raw input stream.
 *
 *  Copyright (C) 2001 Andrew Stevens <andrew.stevens@philips.com>
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU General Public License
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <config.h>
#include <math.h>
#include <stdlib.h>

#include "audiostrm.hpp"
#include "interact.hpp"
#include "multiplexor.hpp"


static const char *mpa_audio_version[4] =
{
	"2.5",
	"2.0",
	"reserved",
	"1.0"
};

static const unsigned int mpa_bitrates_kbps [4][3][16] =
{
	{ /* MPEG audio V2.5 */
		{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}
	},
	{ /*RESERVED*/
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	},
	{ /* MPEG audio V2 */
		{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,0},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}
	},
	{ /* MPEG audio V1 */
		{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,0},
		{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384,0},
		{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
	}

};


static const int mpa_freq_table [4][4] = 
{
	/* MPEG audio V2.5 */
	{11025,12000,8000,0},
	/* RESERVED */
	{ 0, 0, 0, 0 }, 
	/* MPEG audio V2 */
	{22050,24000, 16000,0},
	/* MPEG audio V1 */
	{44100, 48000, 32000, 0}
};

static const char mpa_stereo_mode [4][15] =
{ "stereo", "joint stereo", "dual channel", "single channel" };
static const char mpa_copyright_status [2][20] =
{ "no copyright","copyright protected" };
static const char mpa_original_bit [2][10] =
{ "copy","original" };
static const char mpa_emphasis_mode [4][20] =
{ "none", "50/15 microseconds", "reserved", "CCITT J.17" };
static const unsigned int mpa_slots [4] = {12, 144, 144, 0};
static const unsigned int mpa_samples [4] = {384, 1152, 1152, 0};



MPAStream::MPAStream(IBitStream &ibs, Multiplexor &into) : 
	AudioStream( ibs, into )
{
	for( int i = 0; i <2 ; ++i )
		num_frames[i] = size_frames[i] = 0;
}

bool MPAStream::Probe(IBitStream &bs )
{
    return bs.GetBits(11) == AUDIO_SYNCWORD;
}


/*************************************************************************
 *
 * Reads initial stream parameters and displays feedback banner to users
 *
 *************************************************************************/


void MPAStream::Init ( const int stream_num )

{
	int padding_bit;

	MuxStream::Init( AUDIO_STR_0 + stream_num, 
					 0,  // Buffer scale
					 muxinto.audio_buffer_size,
					 muxinto.vcd_zero_stuffing,
					 muxinto.buffers_in_audio,
					 muxinto.always_buffers_in_audio
		);
    mjpeg_info ("Scanning for header info: Audio stream %02x (%s)",
                AUDIO_STR_0 + stream_num,
                bs.StreamName()
                );

	/* A.Stevens 2000 - update to be compatible up to  MPEG2.5
	 */
    AU_start = bs.bitcount();
    if (bs.GetBits(11)==AUDIO_SYNCWORD)
    {
		num_syncword++;
		version_id = bs.GetBits( 2);
		layer 		= 3-bs.GetBits( 2); /* 0..2 not 1..3!! */
		protection 		= bs.Get1Bit();
		bit_rate_code	= bs.GetBits( 4);
		frequency 		= bs.GetBits( 2);
		padding_bit     = bs.Get1Bit();
		bs.Get1Bit();
		mode 		= bs.GetBits( 2);
		mode_extension 	= bs.GetBits( 2);
		copyright 		= bs.Get1Bit();
		original_copy 	= bs.Get1Bit ();
		emphasis		= bs.GetBits( 2);

		framesize =
			mpa_bitrates_kbps[version_id][layer][bit_rate_code]  * 
			mpa_slots[layer] *1000 /
			mpa_freq_table[version_id][frequency];

		size_frames[0] = framesize * ( layer == 0 ? 4 : 1);
		size_frames[1] = (framesize+1) * ( layer == 0 ? 4 : 1);
		num_frames[padding_bit]++;
        access_unit.start  = AU_start;
		access_unit.length = size_frames[padding_bit];
	  
		samples_per_second = mpa_freq_table[version_id][frequency];

		/* Presentation time-stamping  */
		access_unit.PTS = static_cast<clockticks>(decoding_order) * 
			static_cast<clockticks>(mpa_samples [layer]) * 
			static_cast<clockticks>(CLOCKS)	/ samples_per_second;
		access_unit.DTS = access_unit.PTS;
		access_unit.dorder = decoding_order;
		++decoding_order;
		aunits.Append( access_unit );

    } else
    {
		mjpeg_error ( "Invalid MPEG Audio stream header.");
		exit (1);
    }


	OutputHdrInfo();
}

unsigned int MPAStream::NominalBitRate()
{ 
	return mpa_bitrates_kbps[version_id][layer][bit_rate_code]*1024;
}


unsigned int MPAStream::SizeFrame( int rate_code, int padding )
{
	return ( mpa_bitrates_kbps[version_id][layer][rate_code]  * 
		mpa_slots [layer] *1000 /
		mpa_freq_table[version_id][frequency] + padding ) * ( layer == 0 ? 4 : 1);
}

void MPAStream::FillAUbuffer(unsigned int frames_to_buffer )
{
	unsigned int padding_bit;
	last_buffered_AU += frames_to_buffer;

    if( eoscan )
        return;

    mjpeg_debug( "Scanning %d MPA frames to frame %d", 
                frames_to_buffer,
                last_buffered_AU );
	while( !bs.eos() 
           && decoding_order < last_buffered_AU 
           && !muxinto.AfterMaxPTS(access_unit.PTS) )
	{

		int skip=access_unit.length-4;
        bs.SeekFwdBits( skip );
		prev_offset = AU_start;
		AU_start = bs.bitcount();
        if( AU_start - prev_offset != access_unit.length*8 )
        {
            mjpeg_warn("Discarding incomplete final frame MPEG audio stream %02x!",
                       stream_id
                       );
            aunits.DropLast();
            --decoding_order;
            break;
        }
		/* Check we have reached the end of have  another catenated 
		   stream to process before finishing ... */
		if ( (syncword = bs.GetBits( 11))!=AUDIO_SYNCWORD )
		{
            //
            // Handle a broken last frame...
			if( !bs.eos()   )
			{
                mjpeg_warn( "Data follows end of last recogniseable MPEG audio frame - bad stream?");
                eoscan = true;
                return;
			}
            break;
		}
		// Skip version_id:2, layer:2, protection:1
		(void) bs.GetBits( 5);
		int rate_code	= bs.GetBits( 4);
		// Skip frequency
		(void) bs.GetBits( 2);

		padding_bit=bs.Get1Bit();
		access_unit.start = AU_start;
		access_unit.length = SizeFrame( rate_code, padding_bit );
		access_unit.PTS = static_cast<clockticks>(decoding_order) * static_cast<clockticks>(mpa_samples[layer]) * static_cast<clockticks>(CLOCKS)
			/ samples_per_second;
		access_unit.DTS = access_unit.PTS;
		access_unit.dorder = decoding_order;
		decoding_order++;
		aunits.Append( access_unit );
		num_frames[padding_bit]++;

		bs.GetBits( 9);
		
		num_syncword++;

		if (num_syncword >= old_frames+10 )
		{
			mjpeg_debug ("Got %d frame headers.", num_syncword);
			old_frames=num_syncword;
		
		}
	


    }
	last_buffered_AU = decoding_order;
	eoscan = bs.eos() || muxinto.AfterMaxPTS(access_unit.PTS);
}



void MPAStream::Close()
{
    stream_length = AU_start >> 3;
	mjpeg_info ("AUDIO_STATISTICS: %02x", stream_id); 
    mjpeg_info ("Audio stream length %lld bytes.", stream_length);
    mjpeg_info   ("Syncwords      : %8u",num_syncword);
    mjpeg_info   ("Frames         : %8u padded",  num_frames[0]);
    mjpeg_info   ("Frames         : %8u unpadded", num_frames[1]);
	
}

/*************************************************************************
	OutputAudioInfo
	gibt gesammelte Informationen zu den Audio Access Units aus.

	Prints information on audio access units
*************************************************************************/

void MPAStream::OutputHdrInfo ()
{
    unsigned int bitrate;
    bitrate = mpa_bitrates_kbps[version_id][layer][bit_rate_code];


	mjpeg_info("MPEG AUDIO STREAM: %02x", stream_id);
	mjpeg_info("Audio version  : %s", mpa_audio_version[version_id]);
    mjpeg_info("Layer          : %8u",layer+1);

    if (protection == 0) mjpeg_info ("CRC checksums  :      yes");
    else  mjpeg_info ("CRC checksums  :       no");

    if (bit_rate_code == 0)
		mjpeg_info ("Bit rate       :     free");
    else if (bit_rate_code == 0xf)
		mjpeg_info ("Bit rate       : reserved");
    else
		mjpeg_info ("Bit rate       : %8u bytes/sec (%3u kbit/sec)",
				bitrate*128, bitrate);

    if (frequency == 3)
		mjpeg_info ("Frequency      : reserved");
    else
		mjpeg_info ("Frequency      :     %d Hz",
				mpa_freq_table[version_id][frequency]);

    mjpeg_info   ("Mode           : %8u %s",
			  mode,mpa_stereo_mode[mode]);
    mjpeg_info   ("Mode extension : %8u",mode_extension);
    mjpeg_info   ("Copyright bit  : %8u %s",
			  copyright,mpa_copyright_status[copyright]);
    mjpeg_info   ("Original/Copy  : %8u %s",
			  original_copy,mpa_original_bit[original_copy]);
    mjpeg_info   ("Emphasis       : %8u %s",
			  emphasis,mpa_emphasis_mode[emphasis]);
}



/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
