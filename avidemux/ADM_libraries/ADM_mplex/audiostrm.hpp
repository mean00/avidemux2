
/*
 *  audiostrm.hpp:  Audio stream class sub-hierarchy for MPEG multiplexing
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

#ifndef __AUDIOSTRM_H__
#define __AUDIOSTRM_H__

#include "inputstrm.hpp"
#include "stream_params.hpp"

// TODO AudioStream is reall NonVideoStream as it is also a base for
// SUBPStream (sub)classes

class AudioStream : public ElementaryStream
{
public:   
    AudioStream(IBitStream &ibs, Multiplexor &into );
    virtual void Init(const int stream_num) = 0;
    virtual void Close() = 0;

    void OutputSector();
    bool RunOutComplete();
    virtual unsigned int NominalBitRate() = 0;

    unsigned int num_syncword;

protected:
	virtual void FillAUbuffer(unsigned int frames_to_buffer) = 0;
    
	/* State variables for scanning source bit-stream */
    AUnit access_unit;
    unsigned int header_skip;
}; 	

class MPAStream : public AudioStream
{
public:   
    MPAStream(IBitStream &ibs, Multiplexor &into );
    virtual void Init(const int stream_num);
    static bool Probe(IBitStream &bs);
    virtual void Close();
    virtual unsigned int NominalBitRate();


private:
	void OutputHdrInfo();
	unsigned int SizeFrame( int bit_rate, int padding_bit );
	virtual void FillAUbuffer(unsigned int frames_to_buffer);


    /* Stream information for logging and parsing purposes */
    unsigned int samples_per_second;
	unsigned int version_id ;
    unsigned int layer		;
    unsigned int protection	;
    unsigned int bit_rate_code;
    unsigned int frequency	;
    unsigned int mode		;
    unsigned int mode_extension ;
    unsigned int copyright      ;
    unsigned int original_copy  ;
    unsigned int emphasis	;

	/* State variables for scanning source bit-stream */
    unsigned int framesize;
    unsigned int num_frames [2];
    unsigned int size_frames[2];

}; 	



class AC3Stream : public AudioStream
{
public:   
    AC3Stream(IBitStream &ibs,Multiplexor &into );
    virtual void Init(const int stream_num);
    static bool Probe(IBitStream &bs);
    virtual void Close();
    virtual unsigned int NominalBitRate();

    virtual unsigned int ReadPacketPayload(uint8_t *dst, unsigned int to_read);
    virtual unsigned int StreamHeaderSize() { return 4; }
    

private:
	void OutputHdrInfo();
    void DisplayAc3HeaderInfo();
	virtual void FillAUbuffer(unsigned int frames_to_buffer);
    
    static const unsigned int default_buffer_size;
	/* State variables for scanning source bit-stream */
    unsigned int framesize;
    unsigned int frequency;
    unsigned int samples_per_second;
    unsigned int bit_rate;
    unsigned int stream_num;
    unsigned int num_frames;
}; 	

class DTSStream : public AudioStream
{
public:   
    DTSStream(IBitStream &ibs,Multiplexor &into );
    virtual void Init(const int stream_num);
    static bool Probe(IBitStream &bs);
    virtual void Close();
    virtual unsigned int NominalBitRate();

    virtual unsigned int ReadPacketPayload(uint8_t *dst, unsigned int to_read);
    virtual unsigned int StreamHeaderSize() { return 4; }
    

private:
	void OutputHdrInfo();
#ifdef DEBUG_DTS
    void DisplayDtsHeaderInfo();
#endif
	virtual void FillAUbuffer(unsigned int frames_to_buffer);
    
    static const unsigned int default_buffer_size;

	/* State variables for scanning source bit-stream */
    unsigned int framesize;
    unsigned int samples_per_second;
    unsigned int bit_rate;
    unsigned int stream_num;
    unsigned int frequency	;
    unsigned int num_frames;
}; 	

class LPCMStream : public AudioStream
{
public:   
    LPCMStream(IBitStream &ibs, LpcmParams *parms, Multiplexor &into );
    virtual void Init(const int stream_num);
    static bool Probe(IBitStream &bs);
    virtual void Close();
    virtual unsigned int NominalBitRate();

    virtual unsigned int ReadPacketPayload(uint8_t *dst, unsigned int to_read);
    virtual unsigned int StreamHeaderSize() { return 7; }
    

private:
	void OutputHdrInfo();
	virtual void FillAUbuffer(unsigned int frames_to_buffer);
    
    static const unsigned int default_buffer_size;
    static const unsigned int ticks_per_frame_90kHz;
    unsigned int num_frames;

	/* State variables for scanning source bit-stream */
    unsigned int stream_num;
    unsigned int samples_per_second;
    unsigned int channels;
    unsigned int bits_per_sample;
    unsigned int bytes_per_frame;
    unsigned int frame_index;
    unsigned int dynamic_range_code;
    LpcmParams *parms;
}; 	

class SUBPStream : public AudioStream
{
public:   
    SUBPStream(IBitStream &ibs,Multiplexor &into );
    virtual void Init(const int stream_num);
    virtual void Close();
    // TODO: rough and ready measure...
    virtual unsigned int NominalBitRate() {return 50*1024;}
    virtual unsigned int ReadPacketPayload(uint8_t *dst, unsigned int to_read);
    virtual unsigned int StreamHeaderSize() { return 1; }
    

private:
	void OutputHdrInfo();
	virtual void FillAUbuffer(unsigned int frames_to_buffer);
    
    static const unsigned int default_buffer_size;

	/* State variables for scanning source bit-stream */
    unsigned int framesize;
    unsigned int samples_per_second;
    unsigned int bit_rate;
    unsigned int stream_num;
    unsigned int frequency	;
    unsigned int num_frames;
}; 	


#endif // __AUDIOSTRM_H__


/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
