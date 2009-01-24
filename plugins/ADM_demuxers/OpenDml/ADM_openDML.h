//
//
// C++ Interface: ADM_openDML
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//


#ifndef __ODMLHEADER__
#define __ODMLHEADER__
#include "avifmt.h"
#include "avifmt2.h"

#include "ADM_Video.h"
#include "ADM_riff.h"
#include "ADM_audioStream.h"
class AVDMGenericAudioStream;

typedef struct odmlIndex
{
	uint64_t offset;
	uint64_t size;
	uint32_t intra;
    uint64_t pts;
    uint64_t dts;
}odmlIndex;

typedef struct odmlTrack
{
	odmlIndex strf;
	odmlIndex strh;
	odmlIndex indx;
}odmlTrack;
/**
    \class odmlAudioTrack
*/
class odmlAudioTrack
{
public:
                 odmlAudioTrack(void);
                 ~odmlAudioTrack();
//********************************
                 odmlIndex               *index;
                 
                 WAVHeader               *wavHeader;
                 uint32_t                nbChunks;
                 uint32_t                extraDataLen;
                 uint8_t                 *extraData;
                 uint32_t                trackNum;
                 uint32_t                totalLen;
                 AVIStreamHeader          *avistream;
};

/**
    \class OpenDMLHeader
*/
class OpenDMLHeader         :public vidHeader
{
protected:
       				
	  uint64_t			_fileSize;
	  FILE 				*_fd;
	  odmlIndex 		*_idx;
	  odmlAudioTrack                *_audioTracks;
      ADM_audioStream              **_audioStreams;
      uint32_t                       _nbAudioTracks;
      uint32_t                       _currentAudioTrack;

      odmlIndex                     *_audioIdx;
	  
	  void 				walk(riffParser *p) ;
	  uint32_t			_nbTrack;
	  uint8_t			_recHack;
      //
      bool              ptsAvailable;
	  //_________________________________________
	  // This is temporary stuff to read the avi
	  //_________________________________________
	  odmlTrack			_Tracks[10];
	  odmlIndex			_regularIndex;
	  odmlIndex			_movi;
	  //_________________________________________
	  // Extra data for audio & video track
	  //_________________________________________	  
	  
	  uint8_t			_reordered;	/// set to DTS ?
	  
	  
          char                         *myName;
	  uint32_t 			    countAudioTrack( void );
	  uint32_t  			searchAudioTrack(uint32_t which);
        uint64_t            frameToUs(uint32_t frame);
	  // _____________________________________________
	  //		indexer, vanilla, odml and others
	  // _____________________________________________
	  uint8_t			indexODML(uint32_t vidTrack);
	  uint8_t 			indexRegular(uint32_t vidTrack);

	  uint8_t 			indexReindex(uint32_t vidTrack,uint32_t audTrack,
                                        uint32_t audioTrackNumber);	
					// scan one track for openDML						
	  uint8_t			scanIndex(uint32_t track,odmlIndex **index,uint32_t *nbElem);
	  uint32_t			read32( void )
                                        {
                                                uint8_t i[4]={0,0,0,0};
                                                ADM_assert(_fd);
                                                if(1!=fread(i,4,1,_fd))
                                                {
                                                        printf("Problem using odml read32\n");
                                                        return 0;
                                                }
                                                return (i[3]<<24)+(i[2]<<16)+(i[1]<<8)+i[0];
                                        };

	  uint8_t           computePtsDts(void);
      uint8_t           mpegReorder(void);
	  uint8_t			unpackPacked( void );	  	
public:
	  
virtual   void 				Dump(void) ;

					OpenDMLHeader( void ) ;
       		    			~OpenDMLHeader(  ) ;
// AVI io
virtual 	uint8_t			open(const char *name);
virtual 	uint8_t			close(void) ;
  //__________________________
  //				 Info
  //__________________________

  //__________________________
  //				 Audio
  //__________________________

virtual 	WAVHeader              *getAudioInfo(uint32_t i )  ;
virtual 	uint8_t                 getAudioStream(uint32_t i,ADM_audioStream  **audio);
virtual     uint8_t                 getNbAudioStreams(void);
// Frames
  //__________________________
  //				 video
  //__________________________

virtual 	uint8_t  setFlag(uint32_t frame,uint32_t flags);
virtual 	uint32_t getFlags(uint32_t frame,uint32_t *flags);
virtual 	uint8_t  getFrame(uint32_t framenum,ADMCompressedImage *img);
virtual 	uint8_t  getFrameSize(uint32_t frame,uint32_t *size) ;
	     	 		
virtual	    uint8_t	 getExtraHeaderData(uint32_t *len, uint8_t **data);
virtual     uint64_t getTime(uint32_t frameNum);
virtual     uint64_t getVideoDuration(void);
virtual     bool     providePts(void) {return ptsAvailable;};
};

#endif


