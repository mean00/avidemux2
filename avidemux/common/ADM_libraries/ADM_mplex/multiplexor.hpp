
#ifndef __OUTPUTSTREAM_H__
#define __OUTPUTSTREAM_H__

#include <stdio.h>
#include "mjpeg_types.h"
#include "interact.hpp"
#include "inputstrm.hpp"
#include "padstrm.hpp"
#include "systems.hpp"


class Multiplexor
{
public:
	Multiplexor(MultiplexJob &job, OutputStream &output);
	void Multiplex ();


	void ByteposTimecode( bitcount_t bytepos, clockticks &ts );
	
	inline Sys_header_struc *SystemHeader() { return &sys_header; }

	unsigned int PacketPayload(	MuxStream &strm,
								bool buffers, bool PTSstamp, bool DTSstamp );
	unsigned int WritePacket( unsigned int     max_packet_data_size,
							  MuxStream        &strm,
							  bool 	 buffers,
							  clockticks   	 PTS,
							  clockticks   	 DTS,
							  uint8_t 	 timestamps
		);

	bool AfterMaxPTS(clockticks &timestamp) 
		{ return max_PTS != 0 && timestamp >= max_PTS; }


	/* Special "unusual" sector types needed for particular formats 
	 */
	  
	void OutputDVDPriv2 ();

	/* Syntax control parameters, public becaus they're partly referenced
	   by the input stream objects.
	 */
	
	bool always_sys_header_in_pack;
	bool dtspts_for_all_vau;
	bool sys_header_in_pack1;
	bool buffers_in_video;
	bool always_buffers_in_video;	
	bool buffers_in_audio;
	bool always_buffers_in_audio;
	bool sector_align_iframeAUs;
	bool split_at_seq_end;
	bool seg_starts_with_video;
	bool timestamp_iframe_only;
	bool video_buffers_iframe_only;
	unsigned int audio_buffer_size;
	unsigned int packets_per_pack;
	unsigned int min_pes_header_len;
	clockticks max_PTS;

	int mpeg;
	int data_rate;
	int mux_format;
	off_t max_segment_size;

	Workarounds workarounds;

/* In some situations the system/PES packets are embedded with
   external transport data which has to be taken into account for SCR
   calculations to be correct.  E.g. VCD streams. Where each 2324 byte
   system packet is embedded in a 2352 byte CD sector and the actual
   MPEG data is preceded by 30 empty sectors.
*/

	unsigned int	sector_transport_size;
	unsigned int    transport_prefix_sectors; 
	unsigned int 	sector_size;
	unsigned int	vcd_zero_stuffing;	/* VCD audio sectors have 20 0 bytes :-( */

	int 		dmux_rate;	/* Actual data mux-rate for calculations always a multiple of 50  */
	int 		mux_rate;	/* MPEG mux rate (50 byte/sec units      */


	/* Sequence run-out control */
	bool running_out;
	clockticks runout_PTS;
	
private:	
	
    /* Stream packet component buffers */
	
	Sys_header_struc 	sys_header;
	Pack_struc          pack_header;
	Pack_struc *pack_header_ptr;
	Sys_header_struc *sys_header_ptr;
	bool start_of_new_pack;
	bool include_sys_header;

	/* Under-run error messages */
	unsigned int underruns;
	unsigned int underrun_ignore;

	/* Output data stream... */
	PS_Stream *psstrm;
	bitcount_t bytes_output;
    clockticks ticks_per_sector;

public:
	clockticks current_SCR;
private:
	clockticks audio_delay;
	clockticks video_delay;
	bool vbr;
	/* Source data streams */
	/* Note: 1st video stream is regarded as the "master" stream for
	   the purpose of splitting sequences etc...
	*/
	vector<ElementaryStream *> estreams; // Complete set
	vector<ElementaryStream *> vstreams; // Video streams in estreams
	vector<ElementaryStream *> astreams; // Audio streams in estreams
	
	PaddingStream pstrm;
	VCDAPadStream vcdapstrm;
	DVDPriv2Stream dvdpriv2strm;

private:
	void InitSyntaxParameters(MultiplexJob &job);
	void InitInputStreams(MultiplexJob &job);
	void InitInputStreamsForStills(MultiplexJob & job );
	void InitInputStreamsForVideo(MultiplexJob & job );
	unsigned int RunInSectors();
	void Init();
	

	void NextPosAndSCR();
	void SetPosAndSCR( bitcount_t bytepos );

	void OutputPrefix( );

	void OutputSuffix();
	void OutputPadding ( bool vcd_audio_pad );
	void MuxStatus( log_level_t level );

	void WriteRawSector( uint8_t *rawpackets,
						 unsigned int     length
		);
	void AppendMuxStreamsOf( vector<ElementaryStream *> &elem, 
							 vector<MuxStream *> &mux );
};


#endif //__OUTPUTSTREAM_H__
