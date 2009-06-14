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
#include "avilist.h"
#define ADM_AVI_MAX_AUDIO_TRACK 5

// GMV
typedef struct
{		// odml index data
    uint64_t fpos;	// absolute file position of a/v-chunk data
    uint32_t size;	// data size without padding
}odml_index_data_t;

typedef struct
{		// odml index
    uint64_t fpos;		// index chunk position in the file
    uint32_t nEntriesInUse;
    odml_index_data_t* index;// array of indexing data [number of frames per index chunk]
}odml_index_t;

typedef struct
{		// super index data
    uint64_t fpos;	// super index chunk position in the file
    uint32_t pad;   // Available size to put the superindex
    uint32_t index_count;	// current index number
    uint32_t odml_nbrof_index;
    odml_index_t* odml_index;	// array of indexes [number of indexes per stream]
}odml_super_index_t;

typedef enum {	// ODML Generation Control
    NO,	// do not generate ODML (intentional or forced if no largefile support)
    HIDDEN,	// default; generate AVI with hidden ODML 
    NORMAL,	// >4GB; generate normal ODML
} doODML_t;

typedef struct
{
  uint32_t fcc;
  uint32_t flags;
  uint32_t offset;
  uint32_t len;
}IdxEntry;


#include "avifmt.h"
#include "avifmt2.h"
#include <vector>

/**
    \struct aviAudioTrack
    \brief  Describe a audiotrack

*/
typedef struct
{
    AVIStreamHeader header;
    uint32_t        sizeInBytes;
    uint32_t        nbBlocks;
    uint32_t        audioHeaderOffset;
}aviAudioTrack;


/**
    \class aviWrite

*/
 class  aviWrite
 {
 protected:
        doODML_t             doODML;
		FILE 		         *_out;
        ADMFile              *_file;
		MainAVIHeader	     _mainheader;
		AVIStreamHeader      _videostream;
		ADM_BITMAPINFOHEADER _bih;
        std::vector <IdxEntry > myindex;
		uint32_t             nb_audio;
        aviAudioTrack        audioTracks[ADM_AVI_MAX_AUDIO_TRACK];

		uint32_t vframe;

		uint32_t curindex;
		AviList *LAll ;
		AviList	*LMovie ;
		AviList *LMain;

        uint8_t saveFrame(uint32_t len,uint32_t flags,uint8_t *data,uint8_t *fcc);
		uint8_t writeMainHeader( void );
		uint8_t writeVideoHeader( uint8_t *extra, uint32_t extraLen );
        uint8_t writeAudioHeader(ADM_audioStream *stream, AVIStreamHeader *header,uint8_t	odml_stream_nbr);
        uint8_t setStreamInfo(ADMFile *fo,
							uint8_t 	*stream,
                            uint8_t 	*info,
							uint32_t 	infolen,
                            // MOD Feb 2005 by GMV: ODML super index
                            uint32_t	odml_headerlen,
                            uint8_t		odml_stream_nbr,
                            // END MOD Feb 2005 by GMV
                            uint8_t		*extra,
                            uint32_t 	extraLen,
							uint32_t  	maxxed);
	uint8_t  updateHeader(           MainAVIHeader *mainheader,
									AVIStreamHeader *videostream);
	
	
             odml_super_index_t* odml_indexes;	// array of super indexes [number of streams]
	int      odml_nbrof_streams;	// number of streams
	int      odml_index_size;	// number of frames per index chunk
	//int odml_nbrof_index;	// number of indexes per stream
        int  odml_default_nbrof_index;
	uint64_t odml_header_fpos;	// file position of the odml header
	uint64_t odml_riff_fpos[4];	// file positions of AVIX-RIFF start (since RIFFs will be a little smaller than 1GB there will be 4 below 4GB)
	int      odml_riff_count;	// current odml riff number
	uint32_t odml_frames_inAVI;	// number or frames in the first RIFF chunk
	void     odml_destroy_index(void);	// deallocate index data structures
	void     odml_write_dummy_chunk(AviList* alist, uint64_t* fpos, uint32_t size);	// write a dummy chunk and get its file position
	bool     odml_index_frame(int stream_nbr, uint32_t data_size, bool keyFrame);	//index one data chunk
	void     odml_write_sindex(int stream_nbr, const char* stream_fcc);	// write super index
	bool     odml_write_index(int stream_nbr, const char* stream_fcc, const char* index_fcc);	// write index
	void     odml_riff_break(uint32_t len);	// advance to the next riff if required; len = chunk size to be written (incl. 4cc and size info) without padding
	// END MOD Feb 2005 by GMV
        void reallocIndeces( odml_super_index_t *idx);
public:
		aviWrite(void);	
	// MOD Feb 2005 by GMV: ODML support
	virtual ~aviWrite();
	uint8_t saveBegin (
             const char         *name,
		     ADM_videoStream    *video,
		     
             uint32_t           nbAudioStreams,
             ADM_audioStream 	*audiostream[]);
                           

      uint8_t setEnd(void);

	uint8_t     saveVideoFrame(uint32_t len,uint32_t flags,uint8_t *data);
	uint8_t     saveAudioFrame(uint32_t index,uint32_t len,uint8_t *data) ;
	uint32_t	getPos( void );
	uint8_t 	sync( void );


 };
 #endif
                        	
