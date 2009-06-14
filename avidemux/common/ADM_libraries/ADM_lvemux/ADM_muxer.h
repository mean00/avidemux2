//
// C++ Interface: ADM_muxer
//
// Description: 
//
//	Front end to lvemux low level muxer
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#define MUX_MPEG_VRATE 0
#define MUX_BUFFER_SIZE 200*1024
class MpegMuxer
{
	void *packStream;
	int  audioType;
	uint32_t byteHead;
	uint32_t byteTail;
	uint32_t _packSize;
	uint8_t  buffer[MUX_BUFFER_SIZE*2];
	
	uint8_t muxAC3(void);
	uint8_t muxMP2(void);
	uint32_t frequency;
	uint32_t audioBitrate;
	uint32_t keepGoing;	// Set to 0 if a null frame is met
	float	needPerFrame;
public:
		MpegMuxer(void );
		~MpegMuxer(  );
	uint8_t open( char *filename, uint32_t vbitrate, uint32_t fps1000, WAVHeader *audioheader,float need);
	uint8_t writeAudioPacket(uint32_t len, uint8_t *buf);
	uint8_t writeVideoPacket(uint32_t len, uint8_t *buf);
	uint8_t forceRestamp(void);
	uint8_t close( void );
	uint8_t audioEmpty( void);

};
