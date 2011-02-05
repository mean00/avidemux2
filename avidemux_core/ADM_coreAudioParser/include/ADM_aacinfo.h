//
//	Used to get info on a AAC/ADTS stream
//	Some parameters can be given to use as guideline
//

#ifndef ADM__AAC__INFO
#define ADM__AAC__INFO

typedef struct 
{
	
	uint32_t layer;		// 0 mpeg4, 1 mpeg2 
	uint32_t profile;	// 0 Main/1 LC
	uint32_t samplerate;	// i.e. Frequency
	uint32_t channels;	// # channels
	uint32_t nbBlock;	// Packet size including header
	uint32_t size;		// size of complete frame
	uint32_t samples;	// # of sample in this packet
	uint8_t  extra[2];

}AacAudioInfo;


bool getAdtsAacInfo(int size, uint8_t *data, AacAudioInfo *info,int *offset,bool createExtra);
#endif