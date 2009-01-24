#ifndef __OGHEADER__
#define __OGHEADER__
#include "avifmt.h"
#include "avifmt2.h"
#include "ADM_Video.h"
#include "ADM_audio/aviaudio.hxx"
#include "ADM_oghead.h"
#include "ADM_ogmpages.h"

typedef struct OgIndex
{
	uint32_t flags;
	uint64_t pos;
	uint32_t size;
	uint32_t frame_index; // Frame number as seen in the stream. When no reordering=frame #
}OgIndex;

typedef struct OgAudioIndex
{
	uint64_t pos;
	uint64_t sampleCount;	// Nb of sample seen for track 1/2 
				// the sample are of the equivalent of PCM 16 bytes, so the number gets high
				// -very- quickly. The good news is that once we get the frequency, it is
				// very accurate seeking.
	uint32_t dataSum;	// accumulative data seen for track1/2 in usefull payload
				// handy for seeking
}OgAudioIndex;

typedef struct OgAudioTrack
{	
	uint32_t			audioTrack;
	uint16_t			encoding;
	uint16_t			channels;
	uint32_t			byterate;
	uint32_t			nbAudioPacket;	// nb of packet
	uint32_t			trackSize;	// size in bytes 
	uint32_t			frequency;
	OgAudioIndex			*index;	// indexes
}OgAudioTrack;

#define NO_FRAG 0xFFFF
class oggAudio :  public AVDMGenericAudioStream
{

		protected:
			    
			    OGMDemuxer			*_demuxer;			    
			    uint8_t			_trackIndex;
			    
			    OgAudioTrack		*_tracks;
			    OgAudioTrack		*_currentTrack;
			    uint8_t			_buffer[64*1025];
			    uint32_t			_inBuffer; 
			    uint64_t			_pos;
			    uint8_t 			fillBuffer( void );
			    uint32_t			_lastFrag;
			    uint32_t 			_extraLen;
			    uint8_t  			*_extraData;
			    uint64_t			_lastPos;
			    
 			
				
		public:
					oggAudio(const char *name,OgAudioTrack *tracks,uint8_t trkidx );
			virtual 	~oggAudio() ;
			virtual uint8_t goTo(uint32_t offset);
			virtual uint32_t read(uint32_t size,uint8_t *ptr);
			virtual uint32_t readPacket(uint32_t *size, uint8_t *data,uint32_t *flags,uint64_t *position);
			//virtual uint32_t readDecompress( uint32_t size,uint8_t *ptr );
			virtual uint8_t	 goToTime(uint32_t mstime);
			virtual uint8_t	getPacket(uint8_t *dest, uint32_t *len, 
						uint32_t *samples);
			virtual uint8_t	extraData(uint32_t *l,uint8_t **d);
};



class oggHeader         :public vidHeader
{
protected:
             			FILE 			*_fd;
				uint64_t 			_filesize;
				uint32_t			_videoTrack;
				uint32_t                        _currentAudioTrack;
				OgAudioTrack			_audioTracks[2];	
				
				void				_dump(void);
				OGMDemuxer 			*_demux;				
				uint8_t 			buildIndex(uint32_t  *nb);
				OgIndex				*_index;				
				uint32_t			_lastImage;
				uint32_t			_lastFrag;
				oggAudio			*_audio;
				uint8_t				_reordered;
                                char                            *_name;
				uint8_t  			dumpHeader(stream_header *header,uint8_t isaudio);
public:

virtual   void 			Dump(void) ;

					oggHeader( void ) ;
		   virtual  		~oggHeader(  ) ;
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
virtual         uint8_t                 getAudioStreamsInfo(uint32_t *nbStreams, audioInfo **infos);
virtual         uint8_t                 changeAudioStream(uint32_t newstream);
virtual         uint32_t                getCurrentAudioStreamNumber(void) { return _currentAudioTrack;}

// Frames
  //__________________________
  //				 video
  //__________________________
virtual 	uint8_t  getFrameSize(uint32_t frame,uint32_t *size) ;
virtual		uint8_t  reorder( void );
virtual		uint8_t	 isReordered( void ) { return _reordered;} // by default we don"t do frame re-ordering
virtual 	uint8_t  setFlag(uint32_t frame,uint32_t flags);
virtual 	uint32_t getFlags(uint32_t frame,uint32_t *flags) ;
virtual 	uint8_t  getFrameNoAlloc(uint32_t framenum,ADMCompressedImage *img);
};




#endif


