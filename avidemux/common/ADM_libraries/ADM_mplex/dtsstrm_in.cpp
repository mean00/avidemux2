/*
 *  dtsstrm_in.c: dts Audio stream class members handling scanning and
 *  buffering raw input stream.
 *
 *  Copyright (C) 2003 Markus Plail <plail@web.de>
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

#define DTS_SYNCWORD            0x7ffe8001
#define DTS_PACKET_SAMPLES      1536

const unsigned int DTSStream::default_buffer_size = 16*1024;

/// table for the available dts bitrates
static const unsigned int dts_bitrate_index[32] =
{ 32,56,64,96,112,128,192,224,
  256,320,384,448,512,576,640,768,
  960,1024,1152,1280,1344,1408,1411,1472,
  1536,1920,2048,3072,3840,0,0,0
};

/// table for the available dts frequencies
static const unsigned int dts_frequency[17] = 
{ 0, 8000, 16000, 32000, 0, 0, 11025, 22050, 44100, 0, 0, 12000, 24000, 48000, 0, 0 };

/// samples per frame
static const unsigned int dts_packet_samples[4] = 
{ 256, 512, 1024, 2048 };


DTSStream::DTSStream(IBitStream &ibs, Multiplexor &into) : 
	AudioStream( ibs, into )
{
	num_frames = 0;
}

bool DTSStream::Probe(IBitStream &bs )
{
    return bs.GetBits(32) == DTS_SYNCWORD;
}

#ifdef DEBUG_DTS

static char *binString(int value, int length)
{
    char *bin = (char *) malloc((length + 1) * sizeof(char));
    int index;
    int dummy = 1 << (length - 1);

    for(index = 0; index < length; index++)
    {
        if(value & dummy)
            bin[index] = '1';
        else
            bin[index] = '0';

        dummy >>= 1;
    }
    bin[index] = '\0';

    return(bin);
}


/*************************************************************************
 *
 * Reads initial stream parameters and displays feedback banner to users
 * @param stream_num dts substream ID
 *************************************************************************/


void  DTSStream::DisplayDtsHeaderInfo()
{
    /* Some stuff to generate frame-header information */
    printf( "normal/termination? = %i\n", bs.GetBits(1) ); 
    printf( "deficit sample count = %i\n", bs.GetBits(5) ); 
    int crc = bs.GetBits(1);
    printf( "CRC present? = %i\n", crc ); 
    printf( "PCM samples = %i\n", bs.GetBits(7) ); 
    printf( "frame byte size = %i\n", bs.GetBits(14) ); 
    int acmode = bs.GetBits(6);
    printf( "audio channel = %s\n", binString(acmode, 6) ); 
    printf( "audio sampling freqency = %s\n", binString(bs.GetBits(4), 4) ); 
    printf( "bit rate = %s\n", binString(bs.GetBits(5), 5) ); 
    printf( "downmix enabled = %i\n", bs.GetBits(1) ); 
    printf( "dynamic range flag = %i\n", bs.GetBits(1) ); 
    printf( "time stamp = %i\n", bs.GetBits(1) ); 
    printf( "auxiliary data = %i\n", bs.GetBits(1) ); 
    printf( "HDCD = %i\n", bs.GetBits(1) ); 
    printf( "extended coding flag = %i\n", bs.GetBits(1) ); 
    printf( "audio sync word insert = %i\n", bs.GetBits(1) ); 
    printf( "low frequency effects = %i\n", bs.GetBits(1) ); 
    printf( "predictor history = %i\n", bs.GetBits(1) ); 
    if (crc) printf( "CRC = %i\n", bs.GetBits(16) ); 
    printf( "multirate interpolator = %i\n", bs.GetBits(1) ); 
    printf( "encoder software revision = %i\n", bs.GetBits(4) ); 
    printf( "copy history = %i\n", bs.GetBits(2) ); 
    printf( "PCM resolution = %s\n", binString(bs.GetBits(3), 3) ); 
    printf( "front sums difference flags = %i\n", bs.GetBits(1) ); 
    printf( "surround sums difference flags = %i\n", bs.GetBits(1) ); 
    printf( "dialog normalization parameter = %i\n", bs.GetBits(4) ); 
}
#endif
void DTSStream::Init ( const int _stream_num)

{
    stream_num = _stream_num;
	MuxStream::Init( PRIVATE_STR_1, 
					 1,  // Buffer scale
					 default_buffer_size,
					 false,
					 muxinto.buffers_in_audio,
					 muxinto.always_buffers_in_audio
		);
    mjpeg_info ("Scanning for header info: dts Audio stream %02x (%s)",
                stream_num,
                bs.StreamName()
                );

	AU_start = bs.bitcount();
    if (bs.GetBits(32)==DTS_SYNCWORD)
    {
		num_syncword++;
        bs.GetBits(6);         // additional sync
        bs.GetBits(1);         // CRC
        bs.GetBits(7);         // pcm samples
        framesize = bs.GetBits(14) + 1;        // frame size

        bs.GetBits(6);         // audio channels
        frequency = bs.GetBits(4);  // sample rate code
        bit_rate = dts_bitrate_index[bs.GetBits(5)];
        bs.GetBits(5);              // misc.

        header_skip = 10;        // Initially skipped past 10 bytes of header 

		num_frames++;
        access_unit.start = AU_start;
		access_unit.length = framesize;
        mjpeg_info( "dts frame size = %d", framesize );
		samples_per_second = dts_frequency[frequency];

		/* Presentation time-stamping  */
		access_unit.PTS = static_cast<clockticks>(decoding_order) * 
			static_cast<clockticks>(DTS_PACKET_SAMPLES) * 
			static_cast<clockticks>(CLOCKS)	/ samples_per_second;
		access_unit.DTS = access_unit.PTS;
		access_unit.dorder = decoding_order;
		++decoding_order;
		aunits.Append( access_unit );

    } else
    {
		mjpeg_error ( "Invalid dts Audio stream header.");
		exit (1);
    }
	OutputHdrInfo();
}

/// @returns the current bitrate
unsigned int DTSStream::NominalBitRate()
{ 
	return bit_rate*1024;
}

/// Prefills the internal buffer for output multiplexing.
/// @param frames_to_buffer the number of audio frames to read ahead
void DTSStream::FillAUbuffer(unsigned int frames_to_buffer )
{
    unsigned int packet_samples;

	last_buffered_AU += frames_to_buffer;
	mjpeg_debug( "Scanning %d dts audio frames to frame %d", 
				 frames_to_buffer, last_buffered_AU );

	while( !bs.eos() && decoding_order < last_buffered_AU 
            && !muxinto.AfterMaxPTS(access_unit.PTS) )
	{
		int skip = access_unit.length - header_skip; 
        bs.SeekFwdBits(skip);
		prev_offset = AU_start;
		AU_start = bs.bitcount();

        if( AU_start - prev_offset != access_unit.length*8 )
        {
            mjpeg_warn( "Discarding incomplete final frame dts stream %d!",
                       stream_num);
            aunits.DropLast();
            decoding_order--;
            break;
        }

		/* Check if we have reached the end or have  another catenated 
		   stream to process before finishing ... */
		if ( (syncword = bs.GetBits(32))!=DTS_SYNCWORD )
		{
			if( !bs.eos()   )
			{
				mjpeg_error_exit1( "Can't find next dts frame: @ %lld we have %04x - broken bit-stream?", AU_start/8, syncword );
            }
            break;
		}

        bs.GetBits(6);         // additional sync
        bs.GetBits(1);         // CRC
        packet_samples = (bs.GetBits(7) + 1) * 32;         // pcm samples
        framesize = bs.GetBits(14) + 1;        // frame size

        bs.GetBits(6);              // audio channels
        bs.GetBits(4);              // sample rate code
        bs.GetBits(5);              // bitrate
        bs.GetBits(5);              // misc.

        access_unit.start = AU_start;
		access_unit.length = framesize;
		access_unit.PTS = static_cast<clockticks>(decoding_order) * 
			static_cast<clockticks>(packet_samples) * 
			static_cast<clockticks>(CLOCKS)	/ samples_per_second;
		access_unit.DTS = access_unit.PTS;
		access_unit.dorder = decoding_order;
		decoding_order++;
		aunits.Append( access_unit );
		num_frames++;

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


/// Closes the dts stream and prints some statistics.
void DTSStream::Close()
{
    stream_length = AU_start >> 3;
	mjpeg_info ("DTS STATISTICS: %02x", stream_id); 
    mjpeg_info ("Audio stream length %lld bytes.", stream_length);
    mjpeg_info   ("Frames         : %8u",  num_frames);
}

/*************************************************************************
	OutputAudioInfo
	gibt gesammelte Informationen zu den Audio Access Units aus.

	Prints information on audio access units
*************************************************************************/

void DTSStream::OutputHdrInfo ()
{
	mjpeg_info("dts AUDIO STREAM:");

    mjpeg_info ("Bit rate       : %8u bytes/sec (%3u kbit/sec)",
				bit_rate*128, bit_rate);

    if (frequency == 3)
		mjpeg_info ("Frequency      : reserved");
    else
		mjpeg_info ("Frequency      :     %d Hz",
				dts_frequency[frequency]);

}

/**
Reads the bytes neccessary to complete the current packet payload. 
@param to_read number of bytes to read
@param dst byte buffer pointer to read to 
@returns the number of bytes read
 */
unsigned int 
DTSStream::ReadPacketPayload(uint8_t *dst, unsigned int to_read)
{
	clockticks   decode_time;
    // TODO: BUG BUG BUG: if there is a change in format in the stream
    // this framesize will be invalid!  It only *looks* like it works...
    // really each AU should store its own framesize...
    unsigned int frames = to_read / framesize;
    bitcount_t read_start = bs.GetBytePos();
    unsigned int bytes_read =  bs.GetBytes( dst + 4, framesize * frames);
    unsigned int bytes_muxed = bytes_read;

    assert( bytes_read > 0 );   // Should never try to read nothing

    bs.Flush( read_start );

    unsigned int first_header = 
        (new_au_next_sec || au_unsent > bytes_read )
        ? 0 
        : au_unsent;

    // BUG BUG BUG: how do we set the 1st header pointer if we have
    // the *middle* part of a large frame?
    assert( first_header+2 <= to_read );

    unsigned int syncwords = 0;
  
	if (bytes_muxed == 0 || MuxCompleted() )
    {
		goto completion;
    }


	/* Work through what's left of the current AU and the following AU's
	   updating the info until we reach a point where an AU had to be
	   split between packets.
	   NOTE: It *is* possible for this loop to iterate. 

	   The DTS/PTS field for the packet in this case would have been
	   given the that for the first AU to start in the packet.

	*/

	decode_time = RequiredDTS();
	while (au_unsent < bytes_muxed)
	{	  
        // BUG BUG BUG: if we ever had odd payload / packet size we might
        // split a DTS frame in the middle of the syncword!
        assert( bytes_muxed > 1 );
		bufmodel.Queued(au_unsent, decode_time);
		bytes_muxed -= au_unsent;
        if( new_au_next_sec )
            ++syncwords;
		if( !NextAU() )
        {
            goto completion;
        }
		new_au_next_sec = true;
		decode_time = RequiredDTS();
	};

	// We've now reached a point where the current AU overran or
	// fitted exactly.  We need to distinguish the latter case
	// so we can record whether the next packet starts with an
	// existing AU or not - info we need to decide what PTS/DTS
	// info to write at the start of the next packet.
	
	if (au_unsent > bytes_muxed)
	{
        if( new_au_next_sec )
            ++syncwords;
		bufmodel.Queued( bytes_muxed, decode_time);
		au_unsent -= bytes_muxed;
		new_au_next_sec = false;
	} 
	else //  if (au_unsent == bytes_muxed)
	{
		bufmodel.Queued(bytes_muxed, decode_time);
        if( new_au_next_sec )
            ++syncwords;
        new_au_next_sec = NextAU();
	}	   

completion:
    // Generate the dts header...
    // Note the index counts from the low byte of the offset so
    // the smallest value is 1!
    dst[0] = DTS_SUB_STR_0 + stream_num;
    dst[1] = frames;
    dst[2] = (first_header+1)>>8;
    dst[3] = (first_header+1)&0xff;

	return bytes_read + 4;
}



/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
