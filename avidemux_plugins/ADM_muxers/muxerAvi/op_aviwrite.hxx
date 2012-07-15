/***************************************************************************
                 \file         op_aviwrite.hxx
                 \brief low level AVI/openDML Writter
                             -------------------
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 by mean fixounet@free.fr
                           (C)  2005 by GMV: ODML write support
    GPL V2.0
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 #ifndef __op_avi__
 #define __op_avi__
#include "ADM_audioStream.h"
#include "avilist_avi.h"
#include "avifmt.h"
#include "avifmt2.h"
#include <vector>
#include "aviIndex.h"

#define ADM_AVI_MAX_AUDIO_TRACK 5
class aviIndexBase;

/**
    \struct aviAudioTrack
    \brief  Describe a audiotrack

*/
typedef struct
{
    AVIStreamHeader  header;
    uint32_t        sizeInBytes;
    uint32_t        nbBlocks;
}aviAudioTrack;


/**
    \class aviWrite

*/
 class  aviWrite
 {
friend class aviIndexBase;
friend class aviIndexAvi;
friend class aviIndexOdml;
 protected:
		FILE 		         *_out;
        ADMFile              *_file;
        ADM_audioStream      **_audioStreams;
		MainAVIHeader	     _mainheader;
		AVIStreamHeader      _videostream;
		ADM_BITMAPINFOHEADER _bih;       
		uint32_t             nb_audio;
        aviAudioTrack         audioTracks[ADM_AVI_MAX_AUDIO_TRACK];

        aviIndexBase        *indexMaker;
		uint32_t vframe;	

        uint64_t   openDmlHeaderPosition[1+ADM_AVI_MAX_AUDIO_TRACK]; // position of the openDml sindex
        uint32_t   audioStreamHeaderPosition[ADM_AVI_MAX_AUDIO_TRACK];
protected:
		
		uint8_t writeVideoHeader( uint8_t *extra, uint32_t extraLen );
        uint8_t writeAudioHeader(ADM_audioStream *stream, AVIStreamHeader *header,uint32_t sizeInByte,int	trackNumber);
        uint8_t updateHeader(MainAVIHeader *mainheader,	AVIStreamHeader *videostream);

        bool setVideoStreamInfo (ADMFile * fo,
			 const AVIStreamHeader      &stream,
			 const ADM_BITMAPINFOHEADER &bih,
			 uint8_t * extra, uint32_t extraLen,
			 uint32_t maxxed);

        bool setAudioStreamInfo (ADMFile * fo,
			 const AVIStreamHeader      &stream,
			 const WAVHeader &wav,
			 int   trackNumber,
			 uint8_t * extra, uint32_t extraLen,
			 uint32_t maxxed);

        bool createAudioHeader(WAVHeader *wav,ADM_audioStream *stream, AVIStreamHeader *header,
                                uint32_t sizeInBytes,int	trackNumber, uint8_t *extra, int *extraLen);
	
	public:
                aviWrite(void);	
	virtual     ~aviWrite();
	uint8_t     saveBegin (
                             const char        *name,
                             ADM_videoStream    *video,
                             uint32_t           nbAudioStreams,
                             ADM_audioStream 	*audiostream[]);
      uint8_t   setEnd(void);

	uint8_t    saveVideoFrame(uint32_t len,uint32_t flags,uint8_t *data);
	uint8_t    saveAudioFrame(uint32_t index,uint32_t len,uint8_t *data) ;
 };
 #endif
                        	
