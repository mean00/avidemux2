
/*
 *  inptstrm.hpp:  Input stream classes for MPEG multiplexing
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

#ifndef __INPUTSTRM_H__
#define __INPUTSTRM_H__

#include <stdio.h>
#include <vector>
#include <sys/stat.h>

#include "mjpeg_types.h"
#include "mpegconsts.h"
#include "format_codes.h"
#include "mjpeg_logging.h"

#include "mplexconsts.hpp"
#include "bits.hpp"
#include "aunitbuffer.hpp"
#include "decodebufmodel.hpp"

using std::vector;

class InputStream
{
public:
	InputStream( IBitStream &istream ) :
		stream_length(0),
        bs( istream ),
		eoscan(false),
		last_buffered_AU(0),
		decoding_order(0),
		old_frames(0)
		{}

	void SetBufSize( unsigned int buf_size )
		{
			bs.SetBufSize( buf_size );
		}

    bitcount_t stream_length;

protected:
	off_t      file_length;
    IBitStream &bs;
	bool eoscan;
	
	unsigned int last_buffered_AU;		// decode seq num of last buffered frame + 1
   	bitcount_t AU_start;
    uint32_t  syncword;
    bitcount_t prev_offset;
    unsigned int decoding_order;
    unsigned int old_frames;

};

class Multiplexor;


class MuxStream 
{
public:
	MuxStream();

    void Init( const int strm_id,
               const unsigned int _buf_scale,
			   const unsigned int buf_size,
			   const unsigned int _zero_stuffing,
			   const bool bufs_in_first, 
			   const bool always_bufs
		  );

	unsigned int BufferSizeCode();
	inline unsigned int BufferSize() { return buffer_size; }
	inline unsigned int BufferScale() { return buffer_scale; }

	
	inline void SetMaxPacketData( unsigned int max )
		{
			max_packet_data = max;
		}
	inline void SetMinPacketData( unsigned int min )
		{
			min_packet_data = min;
		}
	inline unsigned int MaxPacketData() { return max_packet_data; }
	inline unsigned int MinPacketData() { return min_packet_data; }
    inline bool NewAUNextSector() { return new_au_next_sec; }

	//
	//  Read the next packet payload (sub-stream headers plus
	//  parsed and spliced stream data) for a packet with the
	//  specified payload capacity.  Update the AU info.
	//

	virtual unsigned int ReadPacketPayload(uint8_t *dst, unsigned int to_read) = 0;

    //
    // Return the size of the substream headers...
    //
    virtual unsigned int StreamHeaderSize() { return 0; }

public:  // TODO should go protected once encapsulation complete
	int        stream_id;
	unsigned int    buffer_scale;
	unsigned int 	buffer_size;
	DecodeBufModel  bufmodel;
	unsigned int 	max_packet_data;
	unsigned int	min_packet_data;
	unsigned int    zero_stuffing;
	unsigned int    nsec;
    unsigned int    min_pes_header_len;
	bool buffers_in_header;
	bool always_buffers_in_header;
	bool new_au_next_sec;
	bool init;
};

class DummyMuxStream : public MuxStream
{
public:
    DummyMuxStream( const int strm_id,
                    const unsigned int buf_scale, 
                    unsigned int buf_size )
        {
            stream_id = strm_id;
            buffer_scale = buf_scale;
            buffer_size = buf_size;
        }

    unsigned int ReadPacketPayload(uint8_t *dst, unsigned int to_read)
        {
            abort();
            return 0;
        }
};


class ElementaryStream : public InputStream,
						 public MuxStream
{
public:
	enum stream_kind { audio, video, dummy };
	ElementaryStream( IBitStream &ibs,
                      Multiplexor &into, 
					  stream_kind kind
					  );
    virtual ~ElementaryStream () { }
	virtual void Close() = 0;

	bool NextAU();
	AUnit *Lookahead( unsigned int n = 0);
	unsigned int BytesToMuxAUEnd(unsigned int sector_transport_size);
	bool MuxCompleted();
	virtual bool MuxPossible(clockticks currentSCR );
	void DemuxedTo( clockticks SCR );
	void SetTSOffset( clockticks baseTS );
	void AllDemuxed();
	inline stream_kind Kind() { return kind; }
    inline unsigned int BufferMin() { return buffer_min; }
    inline unsigned int BufferMax() { return buffer_max; }
    inline clockticks BaseDTS() { return au->DTS; };
    inline clockticks BasePTS() { return au->PTS; };

    inline int        DecodeOrder() { return au->dorder; }

    inline clockticks RequiredDTS( const AUnit *unit )  
        { return unit->DTS + timestamp_delay; };
    inline clockticks RequiredPTS( const AUnit *unit ) 
        { return unit->PTS + timestamp_delay; };
    inline clockticks RequiredDTS()  
        { return RequiredDTS(au); };
    inline clockticks RequiredPTS() 
        { return  RequiredPTS(au); };
    inline clockticks NextRequiredDTS()
        { 
            AUnit *next = Lookahead();
            if( next != 0 )
                return RequiredDTS(next); 
            else
                return 0;
        };
    inline clockticks NextRequiredPTS()
        { 
            AUnit *next = Lookahead();
            if( next != 0 )
                return RequiredPTS(next); 
            else
                return 0;
        };

    void UpdateBufferMinMax();

	void SetSyncOffset( clockticks timestamp_delay );

	void BufferAndOutputSector();
 
	inline bool BuffersInHeader() { return buffers_in_header; }
	virtual unsigned int NominalBitRate() = 0;
	virtual bool RunOutComplete() = 0;


    /******************************************************************
     *  Reads the stream data from actual input stream, updates decode
     *  buffer model and current access unit information from the
     *  look-ahead scanning buffer to account for bytes_muxed bytes being
     *  muxed out.  
     * TODO: No longer needs to be virtual
     *
     ******************************************************************/
	virtual unsigned int ReadPacketPayload(uint8_t *dst, unsigned int to_read);

    /********************************************************************
     * Update stream-specific mux state information to reflect muxing of
     * current AU.  first_in_sector is set true if AU is first muxed into
     * the current sector.
     *
     *******************************************************************/

    virtual void AUMuxed( bool first_in_sector ) {}

    /**************************************************************
     * The size of the stream-specific  sub-header (if any)
     *************************************************************/
    virtual unsigned int StreamHeaderSize() { return 0; }
    
    /*****************************************************************
     * Reads/generates the stream-specific sub-header for AUs muxed
     * since last call AUMuxed( true );
     ****************************************************************/

    virtual void ReadStreamHeader( uint8_t *dst, unsigned int len ) {}

    bitcount_t bytes_read;
private:
    void AUBufferLookaheadFill( unsigned int look_ahead);

//protected:
public:
	virtual void FillAUbuffer(unsigned int frames_to_buffer) = 0;
    virtual void OutputSector() = 0;
	AUStream aunits;
	void Muxed( unsigned int bytes_muxed );
	AUnit *au;
	clockticks timestamp_delay;

	unsigned int au_unsent;
	AUnit *OLDnext();
	Multiplexor &muxinto;
	stream_kind kind;
    unsigned int buffer_min;
    unsigned int buffer_max;
    int FRAME_CHUNK;
									
};



#endif // __INPUTSTRM_H__


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
