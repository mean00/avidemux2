//
//	Used to get info on a AAC/ADTS stream
//	Some parameters can be given to use as guideline
//

#ifndef ADM__AAC__INFO
#define ADM__AAC__INFO

typedef struct AacAudioInfo
{
	
	uint32_t layer;		// 0 mpeg4, 1 mpeg2 
	uint32_t profile;	// 0 Main/1 LC
	uint32_t samplerate;	// i.e. Frequency
	uint32_t channels;	// # channels
	uint32_t nbBlock;	// Packet size including header
	uint32_t size;		// size of complete frame
	uint32_t samples;	// # of sample in this packet
	

}AacAudioInfo;

uint8_t	getAACFrameInfo(uint8_t *stream,uint32_t maxSearch, AacAudioInfo *mpegInfo,AacAudioInfo *templ,
			uint32_t *offset);


#endif
