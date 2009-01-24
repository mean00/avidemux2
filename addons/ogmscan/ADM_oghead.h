/***************************************************************************
                          ADM_oghead.cpp  -  description
                             -------------------

		Some headers from xiph.org and http://tobias.everwicked.com/packfmt.htm

    begin                : Tue Apr 28 2003
    copyright            : (C) 2003 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 **************************************************************************/
/*
 0x01: unset = fresh packet
	               set = continued packet
0x02: unset = not first page of logical bitstream
                       set = first page of logical bitstream (bos)
0x04: unset = not last page of logical bitstream
                       set = last page of logical bitstream (eos)
*/
#define OG_CONTINUE_PACKET 		1
#define OG_FIRST_PAGE 				2
#define OG_LAST_PAGE_PACKET 	4

 typedef struct OG_Header
 {
		uint8_t sig[4] ; 		// OggS
		uint8_t version;		// 0 ?
		uint8_t header_type;	// Flags
		uint8_t abs_pos[8];	// frame # or position in logical stream
		uint8_t serial[4];		// id;
		uint8_t page_sequence[4];	// seq
		uint8_t checksum[4];	// seq
		uint8_t nb_segment;	// seq
 }OG_Header;

/* from http://tobias.everwicked.com/packfmt.htm
- 1. Packet (header)

This packet contains information about the stream like type, time base a.s.o.

0x0000  0x01  indicates "Header packet"
0x0001 stream_header structure the size is indicated in the
size member

- 2. Packet (comment)
0x0000  0x03  indicates "Comment packet"
0x0001 data see vorbis doc on www.xiph.org


Data packets
0x0000 Bit0   0
Bit1    Bit 2 of lenbytes
Bit2    unused
Bit3    Keyframe
Bit4   unused
Bit5   unused
Bit6   Bit 0 of lenbytes
Bit7   Bit 1 of lenbytes indicates data packet

*/
#define OG_KEYFRAME 8

typedef struct stream_header_video
{
uint32_t width;
uint32_t height;
} stream_header_video;

typedef struct stream_header_audio
{
uint16_t channels;
uint16_t blockalign;
uint32_t avgbytespersec;
} stream_header_audio;

typedef struct stream_header
{
char streamtype[8];
char subtype[4];

uint32_t size; // size of the structure

uint64_t time_unit; // in reference time
uint64_t samples_per_unit;
uint32_t default_len; // in media time

uint32_t buffersize;
uint16_t bits_per_sample;

	union
	{
		// Video specific
		stream_header_video video;
		// Audio specific
		stream_header_audio audio;
	};
uint16_t	padd;
} stream_header;
