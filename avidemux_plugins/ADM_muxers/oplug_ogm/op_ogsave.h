//
// C++ Interface: op_ogsave
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef OP_OGSAVE
#define OP_OGSAVE

#include "ADM_userInterfaces/ADM_commonUI/DIA_encoding.h"
#include "op_ogpage.h"
#include "ADM_encoder/adm_encoder.h"

// Some little / big endian stuff
extern void memcpyswap(uint8_t *dest, uint8_t *src, uint32_t size);
#ifdef ADM_BIG_ENDIAN	
	 #define MEMCPY(a,b,c) memcpyswap((uint8_t *)a, (uint8_t *)b,c)
#else
	#define MEMCPY memcpy
#endif	
//
class ADM_ogmWrite
{
	protected:
			char 			*TwoPassLogFile;	
	
			uint32_t		_audioTarget,_audioCurrent;
			
			
			uint32_t		_togo;
			uint64_t		_packet;			
			FILE			*_fd;
			uint32_t		_fps1000;
			uint8_t 		*_audioBuffer;
			uint8_t 		*_videoBuffer;
			DIA_encoding		*encoding_gui;
			ogm_page 		*videoStream;
			ogm_page		*audioStream;
			
				uint8_t		initAudio(void);
				uint8_t		writeAudio(uint32_t j);
				uint8_t		endAudio( void);
				uint32_t 	putAC3( uint32_t j );
				uint32_t 	putMP3( uint32_t j );
				
				AVDMGenericAudioStream	*audioFilter;
				
			virtual uint8_t		initVideo(const char *name);
			virtual uint8_t		writeVideo(uint32_t frame);
					
	public:
					ADM_ogmWrite(void);				
					~ADM_ogmWrite(void);
			uint8_t 	save(const char *name);
};

class ADM_ogmWriteCopy : public ADM_ogmWrite
{
protected:
			
			virtual uint8_t		initVideo(const char *name);			
			virtual uint8_t		writeVideo(uint32_t frame);
			uint32_t 		searchForward(uint32_t startframe);
			uint32_t		_lastIPFrameSent;	
public:
						ADM_ogmWriteCopy(void);
						~ADM_ogmWriteCopy(void);
};

class ADM_ogmWriteProcess : public ADM_ogmWrite
{
protected:
			AVDMGenericVideoStream  *_incoming;
			Encoder 		*_encode;
                        uint32_t                _prestore;
                        uint32_t                _prestoring;
			virtual uint8_t	initVideo(const char *name);			
			virtual uint8_t	writeVideo(uint32_t frame);	
public:
			ADM_ogmWriteProcess(void);
			~ADM_ogmWriteProcess(void);


};


#endif
