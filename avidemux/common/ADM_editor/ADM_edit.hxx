/***************************************************************************
     \file  ADM_edit.hxx  
     \brief Editor class
    This file is the composer
    It presents the processed underlying files as if it was a flat
    file.
    Very useful for cut/copy/merge etc...


    The frame seen by GUI/user is converted in seg (segment number)
    	and segrel, the frame number compared to the beginning of the movie
    	described by the segment
    	** NOT the beginning of the segment start_frame **

    (C) 2002-2009 Mean, fixounet@free.Fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 #ifndef __ADM_composer__
 #define __ADM_composer__
 #include "ADM_Video.h"
 #include "../ADM_codecs/ADM_codec.h"
 #include "ADM_image.h"
 #include "../ADM_editor/ADM_edCache.h"
 #include "ADM_pp.h"
 #include "ADM_colorspace.h"

 #include "ADM_audioStream.h"
 #include "ADM_audiocodec/ADM_audiocodec.h"
 #include "ADM_segment.h"
 #include <vector>

#define ADM_EDITOR_AUDIO_BUFFER_SIZE (128*1024*6*sizeof(float))

/**
    \enum _ENV_EDITOR_FLAGS
*/
typedef enum
{
	ENV_EDITOR_NONE=   0x0000,
	ENV_EDITOR_BFRAME= 0x0001,
	ENV_EDITOR_PVOP=   0x0002,
    ENV_EDITOR_X264=   0x0004,
    ENV_EDITOR_SMART=  0x0005,
	ENV_EDITOR_LAST=   0x8000
}_ENV_EDITOR_FLAGS;



/**
            \class ADM_Composer
            \brief Wrapper class that handles all the logic to seek/deal with multiple video files
                        with editing
*/
class ADM_Composer : public ADM_audioStream
{
  private:
                    ADM_EditorSegment _segments;
                    uint8_t     dupe(ADMImage *src,ADMImage *dst,_VIDEOS *vid); 
                                                            // Duplicate img, do colorspace
                                                            // if needed
  					uint32_t	_internalFlags;
  					ADM_PP 		_pp;
					ADMImage	*_imageBuffer;
  					uint8_t		decodeCache(uint32_t frame,uint32_t seg, ADMImage *image);
  					// _audiooffset points to the offset / the total segment
  					// not the used part !
  					uint32_t  _audioseg;
					int64_t   _audioSample;
  					uint32_t  _audiooffset;
					uint8_t	  _haveMarkers; // used for load/save edl
                    
       				uint32_t _lastseg,_lastframe,_lastlen;

                    ADM_audioStreamTrack *getTrack(uint32_t i);
                    ADMImage    *_scratch;																		;
                    uint8_t  	updateAudioTrack(uint32_t seg);			   	
                    void 		deleteAllVideos(void );
                    uint8_t 	getMagic(const char *name,uint32_t *magic);
                    uint32_t 	searchForwardSeg(uint32_t startframe);
                    bool        rederiveFrameType(vidHeader *demuxer);

  public:
                    bool        hasVBRAudio(void);
                    bool     	getExtraHeaderData(uint32_t *len, uint8_t **data);
                    uint32_t    getPARWidth(void);
                    uint32_t    getPARHeight(void);
                    uint8_t     rebuildDuration(void);
  								ADM_Composer();
  				virtual 			~ADM_Composer();
                    void		clean( void );
                    uint8_t     saveAsScript (const char *name, const char *out);
                    uint8_t 	saveWorbench(const char *name);
                    uint8_t 	loadWorbench(const char *name);
                    uint8_t     resetSeg( void );
                    bool     	addFile (const char *name);
                    uint8_t 	cleanup( void);
                    bool 	    isMultiSeg( void);
/************************************* Markers *****************************/
private:        
                    uint64_t    markerAPts,markerBPts;
public:
                    uint64_t    getMarkerAPts();
                    uint64_t    getMarkerBPts();
                    bool        setMarkerAPts(uint64_t pts);
                    bool        setMarkerBPts(uint64_t pts);
public:
/************************************ Public API ***************************/
protected:
                    uint32_t    currentFrame;
                    bool        GoToIntra(uint32_t frame);
                    uint32_t    getCurrentFrame(void); 
                    bool        setCurrentFrame(uint32_t frame);

                    uint64_t    estimatePts(uint32_t frame);
public:
                    bool        getCompressedPicure(ADMCompressedImage *img);
      
public:
                   
                    uint64_t    getCurrentFramePts(void);
                   
                    
                    bool        GoToTime(uint64_t time);
                    bool        GoToIntraTime(uint64_t time);
                    bool        NextPicture(ADMImage *image);
                    bool        samePicture(ADMImage *image);

                   // Fixme, framenumber !
                    uint32_t    searchFrameBefore(uint64_t pts);
                    uint32_t    searchFrameAt(uint64_t pts);
                    bool        getImageFromCacheForFrameBefore(uint64_t pts,ADMImage *out);
                    bool        getPictureJustBefore(uint64_t pts);
                    bool        getPtsDts(uint32_t frame,uint64_t *pts,uint64_t *dts);
/************************************ Internal ******************************/
protected:
                                /// Decode frame and on until frame is popped out of decoders
                    bool        DecodePictureUpToIntra(uint32_t frame,uint32_t ref);
                                /// compressed image->yb12 image image and do postproc/colorconversion
                    bool        decompressImage(ADMImage *out,ADMCompressedImage *in,uint32_t ref);
                                /// Decode next image
                    bool        DecodeNextPicture(uint32_t ref);
                                /// Get the next decoded picture
                    bool     	getNextPicture(ADMImage *out,uint32_t ref);
                                /// Get again last decoded picture
                    bool        getSamePicture(ADMImage *out,uint32_t ref);

                    bool        searchNextKeyFrameInRef(int ref,uint64_t refTime,uint64_t *nkTime);
                    bool        searchPreviousKeyFrameInRef(int ref,uint64_t refTime,uint64_t *nkTime);

/************************************ Internal ******************************/
protected:
                    uint8_t 	getFrame(uint32_t   framenum,ADMCompressedImage *img,uint8_t *isSequential);
                    
                
                    uint64_t 	getTime(uint32_t fn);
                    uint32_t 	getFlags(uint32_t frame,uint32_t *flags);

                            // B follow A with just Bframes in between
                    uint32_t 	getFlagsAndSeg (uint32_t frame,    uint32_t * flags,uint32_t *segs);
                    uint8_t  	setFlag(uint32_t frame,uint32_t flags);

                    uint8_t  	getFrameSize(uint32_t frame,uint32_t *size) ;

public:
                    uint8_t	    updateVideoInfo(aviInfo *info);
                    uint32_t 	getSpecificMpeg4Info( void );
/************************************ audioStream ******************************/
protected:
#define ADM_EDITOR_PACKET_BUFFER_SIZE (20*1024)
                    uint8_t  packetBuffer[ADM_EDITOR_PACKET_BUFFER_SIZE];
                    uint32_t packetBufferSize;
                    uint64_t packetBufferDts;
                    uint32_t packetBufferSamples;
                    bool     refillPacketBuffer(void);
  
public:
                    
            virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts);
                    uint8_t         getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts);
            virtual bool            goToTime(uint64_t nbUs);
                    bool            getExtraData(uint32_t *l, uint8_t **d);
                    uint64_t        getDurationInUs(void);
                    uint8_t			getAudioStream(ADM_audioStream **audio);
            virtual WAVHeader       *getInfo(void);
            virtual CHANNEL_TYPE    *getChannelMapping(void );
/************************************ /audioStream ******************************/
                    bool            getAudioStreamsInfo(uint64_t xtime,uint32_t *nbStreams, audioInfo **infos);
                    bool            changeAudioStream(uint64_t xtime,uint32_t newstream);
                    uint32_t        getCurrentAudioStreamNumber(uint64_t xframe);
                    bool    		setDecodeParam( uint64_t frameTime );
/**************************************** Video Info **************************/
	 				AVIStreamHeader 	*getVideoStreamHeader(void ) ;
	 				MainAVIHeader 		*getMainHeader(void );
	 				ADM_BITMAPINFOHEADER 	*getBIH(void ) ;

	  				uint8_t			getVideoInfo(aviInfo *info);
                    uint64_t        getVideoDuration(void);
                    uint64_t        getFrameIncrement(void); /// Returns the # of us between 2 frames or the smaller value of them

/**************************************** /Video Info **************************/					
					
					
/***************************************** Seeking *****************************/            
protected:
		  			bool			getPKFrame(uint32_t *frame);
					bool			getNKFrame(uint32_t *frame);

                    
                    bool			getUncompressedFrame(uint32_t frame,ADMImage *out,uint32_t *flagz=NULL);
public:
                    bool			getNKFramePTS(uint64_t *frameTime);
                    bool			getPKFramePTS(uint64_t *frameTime);   
/******************************* Post Processing ************************************/
					uint8_t 		setPostProc( uint32_t type, uint32_t strength,	uint32_t swapuv);
					uint8_t 		getPostProc( uint32_t *type, uint32_t *strength,uint32_t *swapuv);
/******************************* /Post Processing ************************************/										
/******************************* Misc ************************************/				
					uint8_t			setEnv(_ENV_EDITOR_FLAGS newflag);
					uint8_t			getEnv(_ENV_EDITOR_FLAGS newflag);
/******************************* /Misc ************************************/				

};
#endif
