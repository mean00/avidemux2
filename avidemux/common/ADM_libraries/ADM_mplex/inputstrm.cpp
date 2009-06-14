
/*
 *  inputstrm.c:  Base classes related to muxing out input streams into
 *                the output stream.
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
#include <assert.h>
#include <limits.h>

#include "mjpeg_types.h"
#include "inputstrm.hpp"
#include "multiplexor.hpp"

MuxStream::MuxStream() : init(false) 
{
}

void MuxStream::Init( const int strm_id, 
			const unsigned int _buf_scale,
			const unsigned int buf_size,
			const unsigned int _zero_stuffing,
			bool bufs_in_first, 
			bool always_bufs) 
{
	stream_id = strm_id;
	nsec = 0;
	zero_stuffing = _zero_stuffing;
	buffer_scale = _buf_scale;
	buffer_size = buf_size;
	bufmodel.Init( buf_size );
	buffers_in_header = bufs_in_first;
	always_buffers_in_header = always_bufs;
	new_au_next_sec = true;
	init = true;
    min_pes_header_len = 0;
}


unsigned int 
MuxStream::BufferSizeCode()
{
	if( buffer_scale == 1 )
		return buffer_size / 1024;
	else if( buffer_scale == 0 )
		return buffer_size / 128;
	else
		assert(false);
    return 0;                   // Never reached...
}


ElementaryStream::ElementaryStream( IBitStream &ibs,
                                    Multiplexor &into, stream_kind _kind) : 
    InputStream( ibs ),
    au(0),
	muxinto( into ),
	kind(_kind),
    buffer_min(INT_MAX),
    buffer_max(1)
{
}

/***********************************
 *
 * Scan ahead to buffer enough info on the coming Access Units to
 * permit look-ahead of look_ahead/processing AUs forward from the
 * current AU *and* the muxing of at least one sector.
 *
 **********************************/

void 
ElementaryStream::AUBufferLookaheadFill( unsigned int look_ahead)
{
    while( !eoscan &&
           ( look_ahead+1 > aunits.MaxAULookahead() 
             || bs.BufferedBytes() < muxinto.sector_size ) )
    {
        FillAUbuffer(FRAME_CHUNK);
    }
}

/******************************************
 *
 * Move on to the next Access unit in the Elementary stream
 *
 *****************************************/

bool 
ElementaryStream::NextAU()
{
    // Free up no longer needed AU record
    if( au != 0 )
        delete au;

    // Ensure we have enough in the AU buffer!
    AUBufferLookaheadFill(1);

    // Get the details of the next AU to be muxed....
	AUnit *p_au = aunits.Next();
	if( p_au != NULL )
	{
		au = p_au;
		au_unsent = p_au->length;
		return true;
	}
	else
	{
		au_unsent = 0;
		return false;
	}
}


AUnit *
ElementaryStream::Lookahead( unsigned int n)
{
    AUBufferLookaheadFill(n);
    return aunits.Lookahead( n );
}

unsigned int 
ElementaryStream::BytesToMuxAUEnd(unsigned int sector_transport_size)
{
	return (au_unsent/min_packet_data)*sector_transport_size +
		(au_unsent%min_packet_data)+(sector_transport_size-min_packet_data);
}


/******************************************************************
 *	ElementaryStream::ReadPacketPayload
 *
 *  Reads the stream data from actual input stream, updates decode
 *  buffer model and current access unit information from the
 *  look-ahead scanning buffer to account for bytes_muxed bytes being
 *  muxed out.  Particular important is the maintenance of "au_unsent"
 *  the count of how much data in the current AU remains umuxed.  It
 *  not only allows us to keep track of AU's but is also used for
 *  generating substream headers
 *
 *  Unless we need to over-ride it to handle sub-stream headers
 * The packet payload for an elementary stream is simply the parsed and
 * spliced buffered stream data..
 *
 ******************************************************************/


unsigned int 
ElementaryStream::ReadPacketPayload(uint8_t *dst, unsigned int to_read)
{
    //
    // Allow for the possibility that stream sub-headers might be needed
    // E.g. AC3, LPCM, DTS....
    unsigned int header_size = StreamHeaderSize();
    bitcount_t read_start = bs.GetBytePos();
    unsigned int actually_read = bs.GetBytes( dst+header_size, to_read-header_size );
    bs.Flush( read_start );
    Muxed( actually_read );
    ReadStreamHeader(dst, header_size);
	return actually_read;
}

void ElementaryStream::Muxed (unsigned int bytes_muxed)
{
	clockticks   decode_time;
  
	if (bytes_muxed == 0 || MuxCompleted() )
		return;


	/* Work through what's left of the current AU and the following AU's
	   updating the info until we reach a point where an AU had to be
	   split between packets.
	   NOTE: It *is* possible for this loop to iterate. 

	   The DTS/PTS field for the packet in this case would have been
	   given the that for the first AU to start in the packet.
	   Whether Joe-Blow's hardware VCD player handles this properly is
	   another matter of course!
	*/

	decode_time = RequiredDTS();
	while (au_unsent < bytes_muxed)
	{	  
        AUMuxed(true);          // Update stream specific tracking 
                                // of AUs muxed...
		bufmodel.Queued(au_unsent, decode_time);
		bytes_muxed -= au_unsent;
        new_au_next_sec = NextAU();
        if( !new_au_next_sec )
			return;
		decode_time = RequiredDTS();
	};

	// We've now reached a point where the current AU overran or
	// fitted exactly.  We need to distinguish the latter case
	// so we can record whether the next packet starts with an
	// existing AU or not - info we need to decide what PTS/DTS
	// info to write at the start of the next packet.
	
	if (au_unsent > bytes_muxed)
	{
        AUMuxed(false);
		bufmodel.Queued( bytes_muxed, decode_time);
		au_unsent -= bytes_muxed;
		new_au_next_sec = false;
	} 
	else //  if (au_unsent == bytes_muxed)
	{
        AUMuxed(false);
		bufmodel.Queued(bytes_muxed, decode_time);
		new_au_next_sec = NextAU();
	}	   

}

bool ElementaryStream::MuxPossible(clockticks currentSCR)
{
	return (!RunOutComplete() &&
			bufmodel.Space() > max_packet_data);
}

void ElementaryStream::UpdateBufferMinMax()
{
    buffer_min =  buffer_min < bufmodel.Space() ? 
        buffer_min : bufmodel.Space();
    buffer_max = buffer_max > bufmodel.Space() ? 
        buffer_max : bufmodel.Space();
}


void ElementaryStream::AllDemuxed()
{
	bufmodel.Flushed();
}

void ElementaryStream::DemuxedTo( clockticks SCR )
{
	bufmodel.Cleaned( SCR );
}

bool ElementaryStream::MuxCompleted()
{
	return au_unsent == 0;
}

void 
ElementaryStream::SetSyncOffset( clockticks sync_offset )
{
	timestamp_delay = sync_offset;
}

void ElementaryStream::BufferAndOutputSector( )
{
    AUBufferLookaheadFill(1);   // TODO is this really needed here?
    OutputSector();
}


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
