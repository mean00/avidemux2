//
// C++ Interface: op_ogpage
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef OP_OGM
#define OP_OHM

#include "ADM_inputs/ADM_ogm/ADM_oghead.h"

#define MAX_OGM_PAGESIZE (64*1024)

class ogm_page
{
protected:
	FILE		*_fd;
	OG_Header	_header;
	uint32_t	_pageNumber;
	uint8_t 	_lacing[512];
	uint8_t 	_page[MAX_OGM_PAGESIZE];
	uint32_t 	_current_lacing;
	uint32_t	_current_off;
	
	
	uint32_t	_first;		// first page ?
	uint8_t		reset(void);
	uint8_t		_fresh;		// If =1, this page starts with a fresh packet
	uint64_t	_timestamp;
	uint8_t		_keyFrame;	// if =1 this page starts with a keyframe
	uint8_t		_needSequence;
	uint32_t	_stream;
	uint32_t 	_sequence;
	uint8_t 	push(uint32_t size,uint32_t remain);
	uint8_t 	buildHeader( void );
public:
			ogm_page(FILE *fd,uint32_t streamId);
			~ogm_page(void);
	uint8_t		flush(void);
	uint8_t		write(uint32_t size, uint8_t *data,uint32_t flags,uint64_t timestamp);
	uint8_t		writeHeaders(uint32_t page, uint8_t *data);
	uint8_t 	writeDirect(uint32_t size, uint8_t *data);
	// Direct writing, bypass our own muxer
	uint8_t 	writeRawData( uint32_t size, uint8_t *data,uint64_t samples);
	
	
};
#endif
