/*


*/
#ifndef AUDIO_DEF
#define AUDIO_DEF
#include <string.h>


/**
    \enum FILMCONV
*/
typedef enum 
{
	FILMCONV_NONE=0,
	FILMCONV_FILM2PAL=1,
	FILMCONV_PAL2FILM=2,
	FILMCONV_LAST
}FILMCONV;
/**
    \struct WAVHeader
*/
typedef struct
{
    uint16_t	encoding;	
    uint16_t	channels;					/* 1 = mono, 2 = stereo */
    uint32_t	frequency;				/* One of 11025, 22050, or 44100 48000 Hz */
    uint32_t	byterate;					/* Average bytes per second */
    uint16_t	blockalign;				/* Bytes per sample block */
    uint16_t	bitspersample;		/* One of 8, 12, 16, or 4 for ADPCM */
} WAVHeader;

typedef enum 
{
    CHANNEL_INVALID=0,
    CHANNEL_MONO,
    CHANNEL_STEREO,
    CHANNEL_2F_1R,
    CHANNEL_3F,
    CHANNEL_3F_1R,
    CHANNEL_2F_2R,
    CHANNEL_3F_2R,
    CHANNEL_3F_2R_LFE,
    CHANNEL_DOLBY_PROLOGIC,
    CHANNEL_DOLBY_PROLOGIC2,
    CHANNEL_LAST
} CHANNEL_CONF;

#define MAX_CHANNELS 9

typedef enum 
{
	ADM_CH_INVALID=0,
	ADM_CH_MONO,
	ADM_CH_FRONT_LEFT,
	ADM_CH_FRONT_RIGHT,
	ADM_CH_FRONT_CENTER,
	ADM_CH_REAR_LEFT,
	ADM_CH_REAR_RIGHT,
	ADM_CH_REAR_CENTER,
	ADM_CH_SIDE_LEFT,
	ADM_CH_SIDE_RIGHT,
	ADM_CH_LFE
}CHANNEL_TYPE;
// returns true if channel mapping is identical
bool ADM_audioCompareChannelMapping(WAVHeader *wh1, WAVHeader *wh2,CHANNEL_TYPE *map1,CHANNEL_TYPE *map2);
/**
\fn ADM_printChannel
\brief return the name of the channel
*/

inline const char *ADM_printChannel(CHANNEL_TYPE c)
{
#define CX(c) case ADM_CH_##c: return #c;
        switch(c)
        {
                CX(INVALID)
                CX(MONO)
                CX(FRONT_LEFT)
                CX(FRONT_RIGHT)
                CX(FRONT_CENTER)
                CX(REAR_LEFT)
                CX(REAR_RIGHT)
                CX(REAR_CENTER)
                CX(SIDE_LEFT)
                CX(SIDE_RIGHT)
                CX(LFE)
        }
         return "unkown channel";
}

typedef struct
{
  const char    *desc;
  CHANNEL_CONF  conf;
}AudioChannelDesc;

const AudioChannelDesc mixerStringDescriptor[]=
{
#define MUXERSTRING(x) {#x,CHANNEL_##x}
    MUXERSTRING(MONO),
    MUXERSTRING(STEREO),
    MUXERSTRING(2F_1R),
    MUXERSTRING(3F),
    MUXERSTRING(3F_1R),
    MUXERSTRING(2F_2R),
    MUXERSTRING(3F_2R),
    MUXERSTRING(3F_2R_LFE),
    MUXERSTRING(DOLBY_PROLOGIC),
    MUXERSTRING(DOLBY_PROLOGIC2)
};
#undef MUXERSTRING
const AudioChannelDesc localDownmixing[]=
{
  {"No downmixing (multichannel)", CHANNEL_INVALID},
  {"Stereo", CHANNEL_STEREO},
  {"Dolby Prologic", CHANNEL_DOLBY_PROLOGIC},
  {"Dolby Prologic II", CHANNEL_DOLBY_PROLOGIC2}
  
};

const char *AudioMixerIdToString(CHANNEL_CONF  cnf);
CHANNEL_CONF AudioMuxerStringToId(const char *st);

#define NB_LOCAL_DOWNMIX (sizeof(localDownmixing)/sizeof(AudioChannelDesc))
#define NB_MIXER_DESC    (sizeof(mixerStringDescriptor)/sizeof(AudioChannelDesc))

#endif
