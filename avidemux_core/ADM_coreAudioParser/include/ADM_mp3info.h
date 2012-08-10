//
//	Used to get info on a MP3 stream
//	Some parameters can be given to use as guideline
//

#ifndef ADM__MP3__INFO
#define ADM__MP3__INFO

#include "ADM_audioParser6_export.h"

typedef struct MpegAudioInfo
{
	
	uint32_t level;		// Mpeg 1, 2 or 2.5 (1,2 or 3)
	uint32_t layer;		// Layer 1,2 or 3
	uint32_t samplerate;	// i.e. Frequency
	uint32_t bitrate;	// Bitrate in b/s
	uint32_t size;		// Packet size including header
	uint32_t samples;	// # of sample in this packet
	uint32_t protect;	// protected
        uint32_t privatebit;    // Private bit
	uint32_t padding;	// Padding slot
	uint32_t mode;		// 0 /1 : stereo j sterero   10 dual mono 11 single (mono)
        uint32_t lsf;           // Used to compute frame length
        uint32_t mode_extension;
}MpegAudioInfo;

ADM_AUDIOPARSER6_EXPORT uint8_t	getMpegFrameInfo(const uint8_t *stream,uint32_t maxSearch, MpegAudioInfo *mpegInfo,MpegAudioInfo *templ,
			uint32_t *offset);


#endif
