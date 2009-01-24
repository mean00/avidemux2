/***************************************************************************
                          ADM_nuv.h  -  description
                             -------------------

	Handle Nuppel video file format

    begin                : Mon Jun 3 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __NUVHEADER__
#define __NUVHEADER__
#include "avifmt.h"
#include "avifmt2.h"
#include "ADM_Video.h"
#include "ADM_audio/aviaudio.hxx"

#include "ADM_inputs/ADM_nuv/nuppel.h"
#include "ADM_inputs/ADM_nuv/RTjpegN.h"

typedef struct nuvIndex
{
	uint64_t _pos;
	uint32_t _len;
	uint16_t _compression;
	uint32_t _kf;
}nuvIndex;

class nuvAudio : public AVDMGenericAudioStream
{
protected:
		
           	uint32_t 					_nb_chunks;
		uint64_t					_abs_position;
		uint32_t					_rel_position;

              	uint32_t 					_current_index;
	    	nuvIndex 					*_index;
		FILE					*_fd;

public:
				nuvAudio(nuvIndex *idx,
					uint32_t nbchunk,
		   			FILE * fd,
					uint32_t fq,
					mythHeader *hdr);
	virtual				~nuvAudio();
        virtual uint32_t 		read(uint32_t len,uint8_t *buffer);
        virtual uint8_t  		goTo(uint32_t newoffset);
		   uint8_t			getNbChunk(uint32_t *ch);

};


class nuvHeader         :public vidHeader
{
protected:
             			FILE 		*_fd;
				uint32_t		_audioResync;
				uint32_t		_audio_frequency;
				uint32_t		_ffv1_fourcc;
				uint32_t		_ffv1_extraLen;
				uint8_t		*_ffv1_extraData;

				uint64_t		_lzo_pos;
				uint64_t		_lzo_size;

				uint8_t 		saveIndex( const char *name,const char *org);
				uint8_t 		loadIndex( const char *name);


				void 		*_nuv_header;
				uint8_t 		*_vbuflzo;
				uint8_t 		*_vbufjpg;
				uint64_t 		_filesize;
				nuvIndex 		*_videoIndex;
				nuvIndex 		*_audioIndex;
				nuvIndex 		*_rIndex;
				uint32_t		_rcount;

				nuvIndex 		*_tableIndex;
				nuvAudio 	*_audioTrack;
				uint8_t 		_isXvid;
				uint8_t 		_isFFV1;
				uint8_t		_isPCM;
				uint8_t		_isMyth;
				uint32_t		_max;
				uint8_t		_jpegHeaderFound;
				mythHeader 	*_mythData;
				baseRT		*_rtjpeg;
				uint8_t 		*_old;
				
				void			_dump(void);
public:

virtual   void 			Dump(void) ;

					nuvHeader( void ) ;
		   virtual  		~nuvHeader(  ) ;
// AVI io
virtual 	uint8_t			open(const char *name);
virtual 	uint8_t			close(void) ;
  //__________________________
  //				 Info
  //__________________________

  //__________________________
  //				 Audio
  //__________________________

virtual 	WAVHeader 		*getAudioInfo(void )  ;
virtual 	uint8_t			getAudioStream(AVDMGenericAudioStream **audio);
virtual	  uint8_t			getExtraHeaderData(uint32_t *len, uint8_t **data)
						{
							if(_ffv1_extraLen)
							{
										*data=_ffv1_extraData;
										*len=_ffv1_extraLen;
										return 1;
							}
							else
							{
									*len=0;
									*data=NULL;
							}
							return 0;
						}
// Frames
  //__________________________
  //				 video
  //__________________________
virtual 	uint8_t  getFrameSize(uint32_t frame,uint32_t *size) ;

virtual 	uint8_t  setFlag(uint32_t frame,uint32_t flags);
virtual 	uint32_t getFlags(uint32_t frame,uint32_t *flags) ;			
virtual 	uint8_t  getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img);
};


 

#endif


