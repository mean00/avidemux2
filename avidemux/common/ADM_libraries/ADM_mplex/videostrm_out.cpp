
/*
 *  inptstrm.c:  Members of input stream classes related to muxing out into
 *               the output stream.
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
#include "videostrm.hpp"
#include "multiplexor.hpp"

VideoStream::VideoStream(IBitStream &ibs, VideoParams *parms, 
                         Multiplexor &into ) :
	ElementaryStream( ibs, into, ElementaryStream::video ),
	num_sequence(0),
	num_seq_end(0),
	num_pictures(0),
	num_groups(0),
	dtspts_for_all_au( into.dtspts_for_all_vau ),
    gop_control_packet( false ),
    parms(parms)
{
	prev_offset=0;
    decoding_order=0;
	fields_presented=0;
    group_start_pic=0;
	group_start_field=0;
    temporal_reference=0;
	pulldown_32 = 0;
    temporal_reference = -1;   // Needed to recognise 2nd field of 1st
                               // frame in a field pic sequence
	last_buffered_AU=0;
	max_bits_persec = 0;
	AU_hdr = SEQUENCE_HEADER;  /* GOP or SEQ Header starting AU? */
	for( int i =0; i<4; ++i )
		num_frames[i] = avg_frames[i] = 0;
    FRAME_CHUNK = 6;
		
}

bool VideoStream::Probe(IBitStream &bs )
{
    return bs.GetBits( 32)  == 0x1b3;
}

/*********************************
 * Signals when video stream has completed mux run-out specified
 * in associated mux stream.   Run-out is always to complete GOP's.
 *********************************/

bool VideoStream::RunOutComplete()
{
 
	return (au_unsent == 0 || 
			( muxinto.running_out &&
			  au->type == IFRAME && RequiredPTS() >= muxinto.runout_PTS));
}

/*********************************
 * Signals if it is permissible/possible to Mux out a sector from the Stream.
 * The universal constraints that muxing should not be complete and that
 * that the reciever buffer should have sufficient it also insists that
 * the muxed data won't hang around in the receiver buffer for more than
 * one second.  This is mainly for the benefit of (S)VCD and DVD applications
 * where long delays mess up random access.
 *******************************/

bool VideoStream::MuxPossible( clockticks currentSCR )
{
	return ( ElementaryStream::MuxPossible(currentSCR) && 
             RequiredDTS() < currentSCR + max_STD_buffer_delay );
}


void VideoStream::AUMuxed( bool first_in_sector )
{
    //DEBUG
    //mjpeg_info( "VidMuxed: %d %lld ", au->dorder, RequiredDTS()/300 );
}


/*********************************
 * Work out the timestamps to be set in the header of sectors starting
 * new AU's.
 *********************************/

uint8_t VideoStream::NewAUTimestamps( int AUtype )
{
	uint8_t timestamps;
    if( AUtype == BFRAME)
        timestamps=TIMESTAMPBITS_PTS;
    else 
        timestamps=TIMESTAMPBITS_PTS_DTS;

    if( muxinto.timestamp_iframe_only && AUtype != IFRAME)
        timestamps=TIMESTAMPBITS_NO;
    return timestamps;
}

/*********************************
 * Work out the buffer records to be set in the header of sectors
 * starting new AU's.
 *********************************/

bool VideoStream::NewAUBuffers( int AUtype )
{
    return buffers_in_header & 
        !(muxinto.video_buffers_iframe_only && AUtype != IFRAME);
}

/********************************
 *
 * Check if the next sector could potentially include parts of AUs
 * following a sequence end marker... in this case a run-out may be needed
 *
 *******************************/

bool VideoStream::SeqEndRunOut()
{
    unsigned int payload = au_unsent;
    unsigned int ahead = 0;
    AUnit *next_au = au;
    for(;;)
    {
        if( next_au->end_seq || payload >= muxinto.sector_size)
            break;  
        ++ahead;
        next_au = Lookahead(ahead);
        if( next_au == 0 )
            break;
        payload += next_au->PayloadSize();
    }
    
    // We don't need to start run-out if the next sector cannot contain
    // next sequence or there is no next sequence (no AU after the one with
    // the sequence end marker
    return next_au != 0 && next_au->end_seq 
        && payload < muxinto.sector_size
        && Lookahead(ahead+1) != 0;

}

/********************************
 *
 * Check if the next sector could potentially include a seq_end marker
 *
 *******************************/

const AUnit *VideoStream::NextIFrame()
{
    unsigned int ahead = 0;
    AUnit *au_ahead = Lookahead(ahead);
    while( au_ahead != 0 && au_ahead->type != IFRAME 
           && ahead < MAX_GOP_LENGTH )
    {
        ++ahead;
        au_ahead = Lookahead(ahead);
    }
    return au_ahead;
}

/********************************
 *
 * Calculate how much payload can be muxed next sector without
 * including the next IFRAME.
 *
 *******************************/

unsigned int VideoStream::ExcludeNextIFramePayload()
{
    unsigned int payload = au_unsent;
    unsigned int ahead = 0;
    AUnit *au_ahead;
    for(;;)
    {
        au_ahead = Lookahead(ahead);
        if( au_ahead == 0 || payload >= muxinto.sector_size || au_ahead->type == IFRAME )
            break;
        payload += au_ahead->PayloadSize();
        ++ahead;
    }
    assert( eoscan || au_ahead != 0 );
    return payload;
}

/******************************************************************
	Output_Video
	generiert Pack/Sys_Header/Packet Informationen aus dem
	Video Stream und speichert den so erhaltenen Sektor ab.

	generates Pack/Sys_Header/Packet information from the
	video stream and writes out the new sector
******************************************************************/

void VideoStream::OutputSector ( )

{
	unsigned int max_packet_payload; 	 
	unsigned int actual_payload;
	unsigned int old_au_then_new_payload;
	clockticks  DTS,PTS;
    int autype;

	max_packet_payload = 0;	/* 0 = Fill sector */
  	/* 	
       I-frame aligning.  For the last AU of segment or for formats
       with ACCESS-POINT sectors where I-frame (and preceding headers)
       are sector aligned.

       We need to look ahead to see how much we may put into the current packet
       without without touching the next I-frame (which is supposed to be
       placed at the start of its own sector).

       N.b.runout_PTS is the PTS of the after which the next I frame
       marks the start of the next sequence.
	*/
    
    /* TODO finish this: Need to look-ahead sufficiently far to
       guarantee finding an I-FRAME even if its predecessors are very
       small.  
    */
	if( muxinto.sector_align_iframeAUs || muxinto.running_out )
	{
		max_packet_payload = ExcludeNextIFramePayload();
	}

	/* Figure out the threshold payload size below which we can fit more
	   than one AU into a packet N.b. because fitting more than one in
	   imposses an overhead of additional header fields so there is a
	   dead spot where we *have* to stuff the packet rather than start
	   fitting in an extra AU.  Slightly over-conservative in the case
	   of the last packet...  */

	old_au_then_new_payload = muxinto.PacketPayload( *this,
					buffers_in_header, 
					true, true);

	/* CASE: Packet starts with new access unit			*/
	if (new_au_next_sec  )
	{
        autype = AUType();
        //
        // Some types of output format (e.g. DVD) require special
        // control sectors before the sector starting a new GOP
        // N.b. this implies muxinto.sector_align_iframeAUs
        //
        if( gop_control_packet && autype == IFRAME )
        {
            OutputGOPControlSector();
        }

        //
        // If we demand every AU should have its own timestamp
        // We can't start two in the same sector...
        //
        if(  dtspts_for_all_au  && max_packet_payload == 0 )
            max_packet_payload = au_unsent;

        PTS = RequiredPTS();
        DTS = RequiredDTS();
		actual_payload =
			muxinto.WritePacket ( max_packet_payload,
						*this,
						NewAUBuffers(autype), 
                                  		PTS, DTS,
						NewAUTimestamps(autype) );

	}

	/* CASE: Packet begins with old access unit, no new one	*/
	/*	     can begin in the very same packet					*/

	else if ( au_unsent >= old_au_then_new_payload ||
              (max_packet_payload != 0 && au_unsent >= max_packet_payload) )
	{
		actual_payload = 
			muxinto.WritePacket( au_unsent,
							*this,
							false, 0, 0,
							TIMESTAMPBITS_NO );
	}

	/* CASE: Packet begins with old access unit, a new one	*/
	/*	     could begin in the very same packet			*/
	else /* if ( !new_au_next_sec  && 
			(au_unsent < old_au_then_new_payload)) */
	{
		/* Is there a new access unit ? */
		if( Lookahead() != 0 )
		{
            autype = NextAUType();
			if(  dtspts_for_all_au  && max_packet_payload == 0 )
				max_packet_payload = au_unsent + Lookahead()->length;

			PTS = NextRequiredPTS();
			DTS = NextRequiredDTS();

			actual_payload = 
				muxinto.WritePacket ( max_packet_payload,
						*this,
						NewAUBuffers(autype), 
                                      		PTS, DTS,
						NewAUTimestamps(autype) );
		} 
		else
		{
			actual_payload = muxinto.WritePacket ( au_unsent, 
							*this, false, 0, 0,
							TIMESTAMPBITS_NO);
		}
	}
	++nsec;
	buffers_in_header = always_buffers_in_header;
}


/***********************************************
   OutputControlSector - Write control sectors prefixing a GOP
   For "normal" video streams this doesn't happen and so represents
   a bug and triggers an abort.

   In DVD's these sectors carry a system header and what is
   presumably indexing and/or sub-title information in
   private_stream_2 packets.  I have no idea what to put in here so we
   simply pad the sector out.
***********************************************/

void VideoStream::OutputGOPControlSector()
{
    abort();
}

 /******************************************************************
 *	OutputGOPControlSector
 *  DVD System headers are carried in peculiar sectors carrying 2
 *  PrivateStream2 packets.   We're sticking 0's in the packets
 *  as we have no idea what's supposed to be in there.
 ******************************************************************/

void DVDVideoStream::OutputGOPControlSector()
{
    muxinto.OutputDVDPriv2 ();
}


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
