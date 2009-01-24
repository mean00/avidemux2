
/*
 *  audiostrm_out.cpp: Members of audio stream classes related to
 *  muxing out into the output stream.
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

#include "mjpeg_types.h"
#include "audiostrm.hpp"
#include "multiplexor.hpp"


AudioStream::AudioStream(IBitStream &ibs, Multiplexor &into) : 
	ElementaryStream( ibs, into,  ElementaryStream::audio ),
	num_syncword(0)
{
    FRAME_CHUNK = 24;
}



/*********************************
 * Signals when audio stream has completed mux run-out specified
 * in associated mux stream. 
 *********************************/

bool AudioStream::RunOutComplete()
{
	return (au_unsent == 0 || 
			( muxinto.running_out && RequiredPTS() >= muxinto.runout_PTS));
}


/******************************************************************
	Output_Audio
	generates Pack/Sys Header/Packet information from the
	audio stream and saves them into the sector
******************************************************************/

void AudioStream::OutputSector ( )

{
	clockticks   PTS;
	unsigned int max_packet_data; 	 
	unsigned int actual_payload;
	unsigned int old_au_then_new_payload;

	PTS = RequiredDTS();
	old_au_then_new_payload = 
		muxinto.PacketPayload( *this, buffers_in_header, false, false );
    bool last_packet = Lookahead() == 0;
    // Ensure we have access units data buffered to allow a sector to be
    // written.
	max_packet_data = 0;
	if( (muxinto.running_out && NextRequiredPTS() > muxinto.runout_PTS)
        || last_packet)
	{
		/* We're now in the last AU of a segment.  So we don't want to
		   go beyond it's end when writing sectors. Hence we limit
		   packet payload size to (remaining) AU length.
		*/
		max_packet_data = au_unsent+StreamHeaderSize();
	}
  
	/* CASE: packet starts with new access unit			*/
	
	if (new_au_next_sec)
    {
		actual_payload = 
			muxinto.WritePacket ( max_packet_data,
								  *this,
								  buffers_in_header, PTS, 0,
								  TIMESTAMPBITS_PTS);

    }


	/* CASE: packet starts with old access unit, no new one	*/
	/*       starts in this very same packet			*/
	else if (!(new_au_next_sec) && 
			 (au_unsent >= old_au_then_new_payload))
    {
		actual_payload = 
			muxinto.WritePacket ( max_packet_data,
								  *this,
								  buffers_in_header, 0, 0,
								  TIMESTAMPBITS_NO );
    }


	/* CASE: packet starts with old access unit, a new one	*/
	/*       starts in this very same packet			*/
	else /* !(new_au_next_sec) &&  (au_unsent < old_au_then_new_payload)) */
    {
		/* is there another access unit anyway ? */
		if( !last_packet )
		{
			PTS = NextRequiredDTS();
			actual_payload = 
				muxinto.WritePacket ( max_packet_data,
									  *this,
									  buffers_in_header, PTS, 0,
									  TIMESTAMPBITS_PTS );

		} 
		else
		{
			actual_payload = muxinto.WritePacket ( max_packet_data,
                                                   *this,
                                                   buffers_in_header, 0, 0,
                                                   TIMESTAMPBITS_NO );
		};
		
    }

    ++nsec;

	buffers_in_header = always_buffers_in_header;
	
}



/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
