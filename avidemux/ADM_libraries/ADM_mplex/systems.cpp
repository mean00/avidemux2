
/*
 *  systems.cpp: Program/System stream packet generator 
 *
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/param.h>
#include "systems.hpp"
#include "mplexconsts.hpp"

PS_Stream:: PS_Stream( unsigned _mpeg,
                       unsigned int _sector_size,
                       OutputStream &_output_strm, 
                       off_t max_seg_size )
    : mpeg_version( _mpeg),
      sector_size( _sector_size ),
      output_strm(_output_strm ),
      max_segment_size( max_seg_size )
{
    sector_buf = new uint8_t[sector_size];
	max_segment_size = max_seg_size;
}

PS_Stream::~PS_Stream()
{
    delete [] sector_buf;
}


bool
PS_Stream::SegmentLimReached()
{
	off_t written = output_strm.SegmentSize();
	return max_segment_size != 0 && written > max_segment_size;
}




/**************************************************************

 	 Packet payload compute how much payload a sector-sized packet with the 
	 specified headers can carry...
     TODO: Should really be called "Sector Payload"
**************************************************************/
	
		
unsigned int 
PS_Stream::PacketPayload( MuxStream &mux_strm,
						  Sys_header_struc *sys_header, 
						  Pack_struc *pack_header, 
						  int buffers, int PTSstamp, int DTSstamp )
{
  int payload = sector_size - (PACKET_HEADER_SIZE + mux_strm.zero_stuffing);
	if( sys_header != NULL )
		payload -= sys_header->length;
	if( mpeg_version == 2 )
	{
	  if( buffers )
	    payload -= MPEG2_BUFFERINFO_LENGTH;

	  payload -= MPEG2_AFTER_PACKET_LENGTH_MIN;
	  if( pack_header != NULL )
	    payload -= pack_header->length;
	  if( DTSstamp )
	    payload -= DTS_PTS_TIMESTAMP_LENGTH;
	  if ( PTSstamp )
	    payload -= DTS_PTS_TIMESTAMP_LENGTH;

	}
	else
	{
		if( buffers )
			payload -= MPEG1_BUFFERINFO_LENGTH;

		payload -= MPEG1_AFTER_PACKET_LENGTH_MIN;
		if( pack_header != NULL )
			payload -= pack_header->length;
		if( DTSstamp )
			payload -= DTS_PTS_TIMESTAMP_LENGTH;
		if (PTSstamp )
			payload -= DTS_PTS_TIMESTAMP_LENGTH;
		if( DTSstamp || PTSstamp )
			payload += 1;  /* No need for nostamp marker ... */

	}
	
	return payload;
}



/*************************************************************************
    Kopiert einen TimeCode in einen Bytebuffer. Dabei wird er nach
    MPEG-Verfahren in bits aufgesplittet.

    Makes a Copy of a TimeCode in a Buffer, splitting it into bitfields
    for MPEG-1/2 DTS/PTS fields and MPEG-1 pack scr fields
*************************************************************************/

void 
PS_Stream::BufferDtsPtsMpeg1ScrTimecode (clockticks    timecode,
										 uint8_t  marker,
										 uint8_t *&buffer)

{
	clockticks thetime_base;
    uint8_t temp;
    unsigned int msb, lsb;
     
 	/* MPEG-1 uses a 90KHz clock, extended to 300*90KHz = 27Mhz in MPEG-2 */
	/* For these fields we only encode to MPEG-1 90Khz resolution... */
	
	thetime_base = timecode /300;
	msb = (thetime_base >> 32) & 1;
	lsb = (thetime_base & static_cast<uint64_t>(0xFFFFFFFF));
		
    temp = (marker << 4) | (msb <<3) |
		((lsb >> 29) & 0x6) | 1;
    *(buffer++)=temp;
    temp = (lsb & 0x3fc00000) >> 22;
    *(buffer++)=temp;
    temp = ((lsb & 0x003f8000) >> 14) | 1;
    *(buffer++)=temp;
    temp = (lsb & 0x7f80) >> 7;
    *(buffer++)=temp;
    temp = ((lsb & 0x007f) << 1) | 1;
    *(buffer++)=temp;

}

/*************************************************************************
    Makes a Copy of a TimeCode in a Buffer, splitting it into bitfields
    for MPEG-2 pack scr fields  which use the full 27Mhz resolution
    
    Did they *really* need to put a 27Mhz
    clock source into the system stream.  Does anyone really need it
    for their decoders?  Get real... I guess they thought it might allow
    someone somewhere to save on a proper clock circuit.
*************************************************************************/


void 
PS_Stream::BufferMpeg2ScrTimecode( clockticks    timecode,
								   uint8_t *&buffer
	)
{
 	clockticks thetime_base;
	unsigned int thetime_ext;
    uint8_t temp;
    unsigned int msb, lsb;
     
	thetime_base = timecode /300;
	thetime_ext =  timecode % 300;
	msb = (thetime_base>> 32) & 1;
	lsb = thetime_base & static_cast<uint64_t>(0xFFFFFFFF);


      temp = (MARKER_MPEG2_SCR << 6) | (msb << 5) |
		  ((lsb >> 27) & 0x18) | 0x4 | ((lsb >> 28) & 0x3);
      *(buffer++)=temp;
      temp = (lsb & 0x0ff00000) >> 20;
      *(buffer++)=temp;
      temp = ((lsb & 0x000f8000) >> 12) | 0x4 |
             ((lsb & 0x00006000) >> 13);
      *(buffer++)=temp;
      temp = (lsb & 0x00001fe0) >> 5;
      *(buffer++)=temp;
      temp = ((lsb & 0x0000001f) << 3) | 0x4 |
             ((thetime_ext & 0x00000180) >> 7);
      *(buffer++)=temp;
      temp = ((thetime_ext & 0x0000007F) << 1) | 1;
      *(buffer++)=temp;
}

/*************************************************************************

BufferPaddingPacket - Insert a padding packet of the desired length
                      into the specified Program/System stream buffer

**************************************************************************/

void
PS_Stream::BufferPaddingPacket( int padding,  uint8_t *&buffer  )
{
    uint8_t *index = buffer;
    int i;

    assert( (mpeg_version == 2 && padding >= 6) ||
            (mpeg_version == 1 && padding >= 7) );

    *(index++) = static_cast<uint8_t>(PACKET_START)>>16;
    *(index++) = static_cast<uint8_t>(PACKET_START & 0x00ffff)>>8;
    *(index++) = static_cast<uint8_t>(PACKET_START & 0x0000ff);
    *(index++) = PADDING_STR;
    *(index++) = static_cast<uint8_t>((padding - 6) >> 8);
    *(index++) = static_cast<uint8_t>((padding - 6) & 0xff);
    if (mpeg_version == 2)
        {
            for (i = 0; i < padding - 6; i++)
                *(index++) = static_cast<uint8_t>(STUFFING_BYTE);
        }
        else
        {
            *(index++) = 0x0F;
            for (i = 0; i < padding - 7; i++)
                *(index++) = static_cast<uint8_t>(STUFFING_BYTE);
        }

    buffer = index;
}


void 
PS_Stream::BufferSectorHeader( uint8_t *index,
                               Pack_struc	 	 *pack,
                               Sys_header_struc *sys_header,
                               uint8_t     *&header_end
    )
{
    /* Pack header if present */

    if (pack != NULL)
    {
		memcpy ( index, pack->buf, pack->length);
		index += pack->length;
    }

    /* System header if present */

    if (sys_header != NULL)
    {
		memcpy (index, sys_header->buf, sys_header->length);
		index += sys_header->length;
    }
    header_end = index;
}

/******************************************
 *
 * BufferPacketHeader
 * Construct an MPEG-1/2 header for a packet in the specified
 * buffer (which *MUST* be long enough) and set points to the start of
 * the payload and packet length fields.
 *
 ******************************************/


void PS_Stream::BufferPacketHeader( uint8_t *buf,
                                    uint8_t type,
                                    unsigned int mpeg_version,
                                    bool buffers,
                                    unsigned int buffer_size,
                                    uint8_t buffer_scale,
                                    clockticks   	 PTS,
                                    clockticks   	 DTS,
                                    uint8_t 	 timestamps,
                                    unsigned    int min_pes_hdr_len,
                                    uint8_t     *&size_field,
                                    uint8_t     *&header_end
    )
{

    uint8_t *index = buf;
	uint8_t *pes_header_len_field = 0;


    /* konstante Packet Headerwerte eintragen */
    /* write constant packet header data */

    *(index++) = static_cast<uint8_t>(PACKET_START)>>16;
    *(index++) = static_cast<uint8_t>(PACKET_START & 0x00ffff)>>8;
    *(index++) = static_cast<uint8_t>(PACKET_START & 0x0000ff);
    *(index++) = type;	


    /* we remember this offset so we can fill in the packet size field once
	   we know the actual size... */
    size_field = index;   
    index += 2;

	if( mpeg_version == 1 )
	{
		/* MPEG-1: buffer information */
		if (buffers)
		{
			*(index++) = static_cast<uint8_t> (0x40 |
                                               (buffer_scale << 5) | (buffer_size >> 8));
			*(index++) = static_cast<uint8_t> (buffer_size & 0xff);
		}

		/* MPEG-1: PTS, PTS & DTS, oder gar nichts? */
		/* should we write PTS, PTS & DTS or nothing at all ? */

		switch (timestamps)
		{
		case TIMESTAMPBITS_NO:
			*(index++) = MARKER_NO_TIMESTAMPS;
			break;
		case TIMESTAMPBITS_PTS:
			BufferDtsPtsMpeg1ScrTimecode (PTS, MARKER_JUST_PTS, index);
			break;
		case TIMESTAMPBITS_PTS_DTS:
			BufferDtsPtsMpeg1ScrTimecode (PTS, MARKER_PTS, index);
			BufferDtsPtsMpeg1ScrTimecode (DTS, MARKER_DTS, index);
			break;
		}
	}
	else if( type != PADDING_STR )
	{
	  	/* MPEG-2 packet syntax header flags. */
        /* These *DO NOT* appear in padding packets 			*/
        /* TODO: They don't appear in several others either!	*/
		/* First byte:
		   <1,0><PES_scrambling_control:2=0><PES_priority><data_alignment_ind.=0>
		   <copyright=0><original=1> */
		*(index++) = 0x81;
		/* Second byte: PTS PTS_DTS or neither?  Buffer info?
		   <PTS_DTS:2><ESCR=0><ES_rate=0>
		   <DSM_trick_mode:2=0><PES_CRC=0><PES_extension=(!!buffers)>
		*/
		*(index++) = (timestamps << 6) | (!!buffers);
		/* Third byte:
		   <PES_header_length:8> */
		pes_header_len_field = index;  /* To fill in later! */
		index++;
		/* MPEG-2: the timecodes if required */
		switch (timestamps)
		{
		case TIMESTAMPBITS_PTS:
			BufferDtsPtsMpeg1ScrTimecode(PTS, MARKER_JUST_PTS, index);
			break;

		case TIMESTAMPBITS_PTS_DTS:
			BufferDtsPtsMpeg1ScrTimecode(PTS, MARKER_PTS, index);
			BufferDtsPtsMpeg1ScrTimecode(DTS, MARKER_DTS, index);
			break;
		}

		/* MPEG-2 The buffer information in a PES_extension */
		if( buffers )
		{
			/* MPEG-2 PES extension header
			   <PES_private_data:1=0><pack_header_field=0>
			   <program_packet_sequence_counter=0>
			   <P-STD_buffer=1><reserved:3=1><{PES_extension_flag_2=0> */
			*(index++) = static_cast<uint8_t>(0x1e);
			*(index++) = static_cast<uint8_t> (0x40 | (buffer_scale << 5) | 
                                               (buffer_size >> 8));
			*(index++) = static_cast<uint8_t> (buffer_size & 0xff);
		}
        /* If required pad the PES header: needed for some workarounds */
        while( index-(pes_header_len_field+1) < min_pes_hdr_len )
            *(index++)=static_cast<uint8_t>(STUFFING_BYTE);
	}

    if( mpeg_version == 2 && type != PADDING_STR )
    {
        *pes_header_len_field = 
            static_cast<uint8_t>(index-(pes_header_len_field+1));	
    }
    header_end = index;
}

/*************************************************************************
 *	CreateSector
 *
 *  Creates a complete sector to carry a padding packet or a packet
 *  from one of the elementary streams.  Pack and System headers are
 *  prepended if required.
 *
 *  We allow for situations where want to
 *  deliberately reduce the payload carried by stuffing.
 *  This allows us to deal with tricky situations where the
 *	header overhead of adding in additional information
 *	would exceed the remaining payload capacity.
 *
 *    Header stuffing and/or a padding packet is appended if the sector is
 *    unfilled.   Zero stuffing after the end of a packet is also supported
 *    to allow thos wretched audio packets from VCD's to be handled.
 *
 *  TODO: Should really be called "WriteSector" 
 * 
 * TODO: We need to add a generic mechanism for sub-headers of
 * private streams to be generated...
 *
 *************************************************************************/


unsigned int
PS_Stream::CreateSector (Pack_struc	 	 *pack,
						 Sys_header_struc *sys_header,
						 unsigned int     max_packet_data_size,
						 MuxStream        &mux_strm,
						 bool 	 buffers,
						 bool    end_marker,
						 clockticks   	 PTS,
						 clockticks   	 DTS,
						 uint8_t 	 timestamps
	)

{
    int i;
    uint8_t *index;
    uint8_t *size_offset;
	unsigned int target_packet_data_size;
	unsigned int actual_packet_data_size;
	int packet_data_to_read;
	int bytes_short;
	uint8_t 	 type = mux_strm.stream_id;
	uint8_t 	 buffer_scale = mux_strm.BufferScale();
	unsigned int buffer_size = mux_strm.BufferSizeCode();
	unsigned int sector_pack_area;
	index = sector_buf;

	sector_pack_area = sector_size - mux_strm.zero_stuffing;
	if( end_marker )
		sector_pack_area -= 4;

    BufferSectorHeader( index, pack, sys_header, index );
    BufferPacketHeader( index, type, mpeg_version,
                        buffers, buffer_size, buffer_scale,
                        PTS, DTS, timestamps,
                        mux_strm.min_pes_header_len,
                        size_offset,
                        index );

    
    /* MPEG-1, MPEG-2: data available to be filled is packet_size less
     * header and MPEG-1 trailer... */

    target_packet_data_size = sector_pack_area - (index - sector_buf );
	
		
    /* DEBUG: A handy consistency check when we're messing around */
#ifdef MUX_DEBUG		
    if( type != PADDING_STR &&

        (end_marker ? target_packet_data_size+4 : target_packet_data_size) 
        != 
        PacketPayload( mux_strm, sys_header, pack, buffers,
                       timestamps & TIMESTAMPBITS_PTS, timestamps & TIMESTAMPBITS_DTS) )
	
    { 
		printf("\nPacket size calculation error %d S%d P%d B%d %d %d!\n ",
               timestamps,
			   sys_header!=0, pack!=0, buffers,
			   target_packet_data_size , 
			   PacketPayload( mux_strm, sys_header, pack, buffers,
							  timestamps & TIMESTAMPBITS_PTS, 
                              timestamps & TIMESTAMPBITS_DTS));
        exit(1);
    }
#endif	
              
    /* If a maximum payload data size is specified (!=0) and is
       smaller than the space available thats all we read (the
       remaining space is stuffed) */
    if( max_packet_data_size != 0 && 
        max_packet_data_size < target_packet_data_size )
    {
        packet_data_to_read = max_packet_data_size;
    }
    else
        packet_data_to_read = target_packet_data_size;
              

    /* MPEG-1, MPEG-2: read in available packet data ... */

    actual_packet_data_size = mux_strm.ReadPacketPayload(index,packet_data_to_read);

    bytes_short = target_packet_data_size - actual_packet_data_size;
#ifdef MUX_DEBUG
    if( type == PRIVATE_STR_1 )
    {
        mjpeg_info( "Substream %02x short %d", index[0], bytes_short );

    }
#endif
	
    /* Handle the situations where we don't have enough data to fill
       the packet size fully ...  small shortfalls are dealt with here
       by stuffing, big ones dealt with later by appending padding packets.
    */


    if(  bytes_short < MINIMUM_PADDING_PACKET_SIZE && bytes_short > 0 )
    {
        if (mpeg_version == 1 )
        {
            /* MPEG-1 stuffing happens *before* header data fields. */
            uint8_t *fixed_packet_header_end = size_offset + 2;
            memmove( fixed_packet_header_end+bytes_short, 
                     fixed_packet_header_end, 
                     actual_packet_data_size+(index-fixed_packet_header_end)
                );
            for( i=0; i< bytes_short; ++i)
                fixed_packet_header_end[i] = static_cast<uint8_t>(STUFFING_BYTE);
        }
        else
        {
            memmove( index+bytes_short, index,  actual_packet_data_size );
            for( i=0; i< bytes_short; ++i)
                *(index+i)=static_cast<uint8_t>(STUFFING_BYTE);
            // Correct PES length field (if present)
            if( type != PADDING_STR )
            {
                uint8_t *pes_header_len_offset = size_offset + 4;
                unsigned int pes_header_len = 
                    index+bytes_short-(pes_header_len_offset+1);
                *pes_header_len_offset = static_cast<uint8_t>(pes_header_len);	            }
        }
        index += bytes_short;
        bytes_short = 0;
    }

	
    /* MPEG-2: We now know the final PES header after padding...
     */
    index += actual_packet_data_size;	 
    /* MPEG-1, MPEG-2: Now we know that actual packet size */
    size_offset[0] = static_cast<uint8_t>((index-size_offset-2)>>8);
    size_offset[1] = static_cast<uint8_t>((index-size_offset-2)&0xff);
	
    /* The case where we have fallen short enough to allow it to be
       dealt with by inserting a stuffing packet... */	
    if ( bytes_short != 0 )
    {
        *(index++) = static_cast<uint8_t>(PACKET_START)>>16;
        *(index++) = static_cast<uint8_t>(PACKET_START & 0x00ffff)>>8;
        *(index++) = static_cast<uint8_t>(PACKET_START & 0x0000ff);
        *(index++) = PADDING_STR;
        *(index++) = static_cast<uint8_t>((bytes_short - 6) >> 8);
        *(index++) = static_cast<uint8_t>((bytes_short - 6) & 0xff);
        if (mpeg_version == 2)
        {
            for (i = 0; i < bytes_short - 6; i++)
                *(index++) = static_cast<uint8_t>(STUFFING_BYTE);
        }
        else
        {
            *(index++) = 0x0F;
            for (i = 0; i < bytes_short - 7; i++)
                *(index++) = static_cast<uint8_t>(STUFFING_BYTE);
        }
		
        bytes_short = 0;
    }
	 
    if( end_marker )
    {
        *(index++) = static_cast<uint8_t>((ISO11172_END)>>24);
        *(index++) = static_cast<uint8_t>((ISO11172_END & 0x00ff0000)>>16);
        *(index++) = static_cast<uint8_t>((ISO11172_END & 0x0000ff00)>>8);
        *(index++) = static_cast<uint8_t>(ISO11172_END & 0x000000ff);
    }

    unsigned int j;
    for (j = 0; j < mux_strm.zero_stuffing; j++)
        *(index++) = static_cast<uint8_t>(0);
	

    /* At this point padding or stuffing will have ensured the packet
       is filled to target_packet_data_size
    */ 
    RawWrite(sector_buf, sector_size);
    return actual_packet_data_size;
}




/*************************************************************************
	Create_Pack
	erstellt in einem Buffer die spezifischen Pack-Informationen.
	Diese werden dann spaeter von der Sector-Routine nochmals
	in dem Sektor kopiert.

	writes specifical pack header information into a buffer
	later this will be copied from the sector routine into
	the sector buffer
*************************************************************************/

void 
PS_Stream::CreatePack ( Pack_struc	 *pack,
                        clockticks   SCR,
                        unsigned int 	 mux_rate
    )
{
    uint8_t *index;

    index = pack->buf;

    *(index++) = static_cast<uint8_t>((PACK_START)>>24);
    *(index++) = static_cast<uint8_t>((PACK_START & 0x00ff0000)>>16);
    *(index++) = static_cast<uint8_t>((PACK_START & 0x0000ff00)>>8);
    *(index++) = static_cast<uint8_t>(PACK_START & 0x000000ff);
        
    if (mpeg_version == 2)
    {
        /* Annoying: MPEG-2's SCR pack header time is different from
           all the rest... */
        BufferMpeg2ScrTimecode(SCR, index);
        *(index++) = static_cast<uint8_t>(mux_rate >> 14);
        *(index++) = static_cast<uint8_t>(0xff & (mux_rate >> 6));
        *(index++) = static_cast<uint8_t>(0x03 | ((mux_rate & 0x3f) << 2));
        *(index++) = (uint8_t)(RESERVED_BYTE << 3 | 0); /* No pack stuffing */
    }
    else
    {
        BufferDtsPtsMpeg1ScrTimecode(SCR, MARKER_MPEG1_SCR, index);
        *(index++) = static_cast<uint8_t>(0x80 | (mux_rate >> 15));
        *(index++) = static_cast<uint8_t>(0xff & (mux_rate >> 7));
        *(index++) = static_cast<uint8_t>(0x01 | ((mux_rate & 0x7f) << 1));
    }
    pack->SCR = SCR;
    pack->length = index-pack->buf;
}


/*************************************************************************
	Create_Sys_Header
	erstelle in einem Buffer die spezifischen Sys_Header
	Informationen. Diese werden spaeter von der Sector-Routine
	nochmals zum Sectorbuffer kopiert.

	writes specifical system header information into a buffer
	later this will be copied from the sector routine into
	the sector buffer
	RETURN: Length of header created...
*************************************************************************/

void 
PS_Stream::CreateSysHeader (	Sys_header_struc *sys_header,
                                unsigned int	 rate_bound,
                                bool	 fixed,
                                int	     CSPS,
                                bool	 audio_lock,
                                bool	 video_lock,
                                vector<MuxStream *> &streams
    )

{
    uint8_t *index;
    uint8_t *len_index;
    int system_header_size;
    index = sys_header->buf;
    int video_bound = 0;
    int audio_bound = 0;
	std::vector<MuxStream *>::iterator str;
    for( str = streams.begin(); str < streams.end(); ++str )
    {
        switch( ((*str)->stream_id & 0xf0) )
        {
        case 0xe0 :             // MPEG Video
            ++video_bound;
            break;
        case 0xb0 :             // DVD seems to use these stream id in
                                // system headers for buffer size counts
            if( (*str)->stream_id == 0xb9 )
                ++video_bound;   
            if( (*str)->stream_id == 0xbd )
                ++audio_bound;   
            break;
        case 0xc0 :
            ++audio_bound;      // MPEG Audio
            break;
        default :
            break;
        }
    }

    /* if we are not using both streams, we should clear some
       options here */

    *(index++) = static_cast<uint8_t>((SYS_HEADER_START)>>24);
    *(index++) = static_cast<uint8_t>((SYS_HEADER_START & 0x00ff0000)>>16);
    *(index++) = static_cast<uint8_t>((SYS_HEADER_START & 0x0000ff00)>>8);
    *(index++) = static_cast<uint8_t>(SYS_HEADER_START & 0x000000ff);

    len_index = index;	/* Skip length field for now... */
    index +=2;

    *(index++) = static_cast<uint8_t>(0x80 | (rate_bound >>15));
    *(index++) = static_cast<uint8_t>(0xff & (rate_bound >> 7));
    *(index++) = static_cast<uint8_t>(0x01 | ((rate_bound & 0x7f)<<1));
    *(index++) = static_cast<uint8_t>((audio_bound << 2)|(fixed << 1)|CSPS);
    *(index++) = static_cast<uint8_t>((audio_lock << 7)|
                                      (video_lock << 6)|0x20|video_bound);

    *(index++) = static_cast<uint8_t>(RESERVED_BYTE);
    for( str = streams.begin(); str < streams.end(); ++str )
    {
        *(index++) = (*str)->stream_id;
        *(index++) = static_cast<uint8_t> 
            (0xc0 |
             ((*str)->BufferScale() << 5) | ((*str)->BufferSizeCode() >> 8));
        *(index++) = static_cast<uint8_t>((*str)->BufferSizeCode() & 0xff);
    }


    system_header_size = (index - sys_header->buf);
    len_index[0] = static_cast<uint8_t>((system_header_size-6) >> 8);
    len_index[1] = static_cast<uint8_t>((system_header_size-6) & 0xff);
    sys_header->length = system_header_size;
}




/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
