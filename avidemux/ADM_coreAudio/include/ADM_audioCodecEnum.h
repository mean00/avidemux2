/**
    \file ADM_audioCodecEnum.h
*/
#ifndef ADM_audioCodecEnum_H
#define ADM_audioCodecEnum_H

#if 0
typedef struct
{
	uint16_t	encoding;	
	uint16_t	channels;					/* 1 = mono, 2 = stereo */
	uint32_t	frequency;				/* One of 11025, 22050, or 44100 48000 Hz */
	uint32_t	byterate;					/* Average bytes per second */
	uint16_t	blockalign;				/* Bytes per sample block */
	uint16_t	bitspersample;		/* One of 8, 12, 16, or 4 for ADPCM */
 // 16 bytes up to here, 14 left
 // Used for VBR mp3
  uint16_t   		cbsize ;
  uint16_t          wId ;
  uint32_t         	fdwflags ;
  uint16_t          nblocksize ;
  uint16_t          nframesperblock  ;
  uint16_t          ncodecdelay ;
} WAVHeaderVBR;
#endif

#define WAV_MP3 	85
#define WAV_MP2 	80
#define WAV_WMA 	353
#define WAV_PCM 	1
#define WAV_MSADPCM 	2
#define WAV_LPCM 	3
#define WAV_AC3 	0x2000
#define WAV_DTS 	0x2001
#define WAV_OGG 0x676f
#define WAV_8BITS 	53 // dummy id
#define WAV_MP4 	54 // dummy id
#define WAV_AAC 	0xff // dummy id
#define WAV_AAC_HE 	0xfe // dummy id
#define WAV_8BITS_UNSIGNED 	55 // dummy id
#define WAV_AMRNB 	56 // dummy id
#define WAV_ULAW	57 // dummy id
#define WAV_QDM2 	58
#define WAV_IMAADPCM    17
#define WAV_AMV_ADPCM    9900
#define WAV_NELLYMOSER   9901
#define WAV_UNKNOWN     9999

#endif
//EOF
