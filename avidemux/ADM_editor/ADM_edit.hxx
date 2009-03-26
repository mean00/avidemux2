/***************************************************************************
                          ADM_edit.hxx  -  description
                             -------------------
    This file is the composer
    It presents the processed underlying files as if it was a flat
    file.
    Very useful for cut/copy/merge etc...


    The frame seen by GUI/user is converted in seg (segment number)
    	and segrel, the frame number compared to the beginning of the movie
    	described by the segment
    	** NOT the beginning of the segment start_frame **



    begin                : Thu Feb 28 2002
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
#define MAX_SEG  	100 // Should be enougth
#define MAX_VIDEO   100

#define ADM_EDITOR_AUDIO_BUFFER_SIZE (128*1024)


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
    \class ADM_audioStreamTack

*/
class ADM_audioStreamTrack
{
public:
    ADM_audioStream  *stream;
    audioInfo        *info;
    ADM_Audiocodec   *codec;
    WAVHeader		 wavheader;
    bool             vbr;
    uint64_t         duration;
    uint64_t         size;

protected:

};
//
//  The start frame correspond to the frame 0 of the segment (quite obvisous)
//  _nb_video_frames is the number of active frame in the segment
//
//

typedef struct
{
  	vidHeader 							*_aviheader;
  	decoders							*decoder;
    COL_Generic2YV12                    *color;

	
    uint32_t                            nbAudioStream;
    uint32_t                            currentAudioStream;
    ADM_audioStreamTrack                **audioTracks;

	uint32_t							_nb_video_frames;	
	uint8_t								_reorderReady;
    uint8_t                             _unpackReady;
	EditorCache							*_videoCache;
    uint32_t                            lastSentFrame;   // Last frame read/sent to decoder
    uint64_t                            lastDecodedPts;  // Pts of last frame out of decoder
    uint64_t                            lastReadPts;     // Pts of the last frame we read
    uint64_t                            timeIncrementInUs; // in case the video has no PTS, time increment (us)
}_VIDEOS;


typedef struct
{
  	uint32_t							_reference;
 	uint32_t							_start_frame;
	uint32_t							_nb_frames;
	uint32_t  							_audio_size;
	uint64_t							_audio_duration; //! IN SAMPLE
	uint32_t							_seg_audio_duration;
	uint32_t							_seg_video_duration;
 	uint32_t  							_audio_start;
}_SEGMENT;
/**
            \class ADM_Composer
*/
class ADM_Composer : public ADM_audioStream
{
  private:
                
//    uint8_t                             audioBuffer[ADM_EDITOR_AUDIO_BUFFER_SIZE];
//    uint32_t                            audioBufferStart;
//    uint32_t                            audioBufferEnd;

                    uint8_t dupe(ADMImage *src,ADMImage *dst,_VIDEOS *vid); 
                                                            // Duplicate img, do colorspace
                                                            // if needed
  					uint32_t	_internalFlags;
  					ADM_PP 		_pp;
					ADMImage	*_imageBuffer;
  					uint8_t		decodeCache(uint32_t frame,uint32_t seg, ADMImage *image);
  					uint32_t 	_nb_segment;
					uint32_t 	_nb_video;
					uint32_t    _nb_clipboard;
  					uint32_t 	_total_frames;
  					uint32_t 	_audio_size;
  					// _audiooffset points to the offset / the total segment
  					// not the used part !
  					uint32_t  _audioseg;
					int64_t   _audioSample;
  					uint32_t  _audiooffset;
					uint8_t	  _haveMarkers; // used for load/save edl
                    
       				uint32_t _lastseg,_lastframe,_lastlen;

					uint32_t	    max_seg;
  					_SEGMENT 		*_segments;
					_SEGMENT 		_clipboard[MAX_SEG];
					_VIDEOS 		_videos[MAX_VIDEO];
                    ADM_audioStreamTrack *getTrack(uint32_t i)
                                            {
                                                if(!_videos[i].audioTracks) return NULL;
                                                return _videos[i].audioTracks[_videos[i].currentAudioStream];

                                            }
                    ADMImage        *_scratch;
						uint8_t  	convFrame2Seg(uint32_t framenum,uint32_t *seg,
																			uint32_t *relframe);
						uint8_t  	convSeg2Frame(	uint32_t *framenum,
																			uint32_t seg,
																			uint32_t relframe);
						uint8_t 	crunch( void)																			;
						uint8_t 	duplicateSegment( uint32_t segno);
						uint32_t 	computeTotalFrames(void) ;

						uint8_t 	removeTo( 	uint32_t to, uint32_t seg,uint8_t included);
						uint8_t 	removeFrom( uint32_t from, uint32_t seg,uint8_t included);
						uint8_t 	checkInSeg( uint32_t seg, uint32_t frame);
						uint8_t 	sanityCheck( void);
				        uint8_t  	updateAudioTrack(uint32_t seg);			   	
						void 		deleteAllVideos(void );

						uint8_t 	getMagic(const char *name,uint32_t *magic);

						uint32_t 	searchForwardSeg(uint32_t startframe);
                        uint8_t     tryIndexing(const char *name, const char *idxname=NULL);
                        bool        rederiveFrameType(vidHeader *demuxer);

  public:
                            uint8_t hasVBRVideos(void);
                            uint8_t addSegment(uint32_t source,uint32_t start, uint32_t nb);
                            uint8_t deleteAllSegments(void);
  						uint8_t 	getExtraHeaderData(uint32_t *len, uint8_t **data);
                            uint32_t getPARWidth(void);
                            uint32_t getPARHeight(void);
                            uint8_t  rebuildDuration(void);
  								ADM_Composer();
  				virtual 			~ADM_Composer();
  						void		clean( void );
  						void		dumpSeg(void);
                        uint8_t     saveAsScript (const char *name, const char *out);
						uint8_t 	saveWorbench(const char *name);
						uint8_t 	loadWorbench(const char *name);
						uint8_t     resetSeg( void );
						uint8_t	    reorder( void );
						uint8_t	    isReordered( uint32_t framenum ) {return true;}
						uint8_t	    isIndexable( void);
  				//_______________________
  				// specific to composer
  				//_______________________
  						uint8_t 	addFile (const char *name, uint8_t mode=0);
  						uint8_t 	cleanup( void);
			   			uint8_t 	isMultiSeg( void) { if(_nb_segment>1) return 1; else return 0;}
  						uint8_t 	removeFrames(uint32_t start,uint32_t end);
  						uint8_t 	addFrameFrom(uint32_t to,uint32_t frombegin,uint32_t fromend);
						uint8_t 	copyToClipBoard (uint32_t start, uint32_t end);
						uint8_t 	pasteFromClipBoard (uint32_t whereto);
  				//_____________________________
  				// navigation & frame functions
  				//_____________________________
/************************************* Markers *****************************/
private:        
                        uint64_t markerAPts,markerBPts;
public:
                        uint64_t    getMarkerAPts();
                        uint64_t    getMarkerBPts();
                        bool        setMarkerAPts(uint64_t pts);
                        bool        setMarkerBPts(uint64_t pts);
public:
/************************************ Public API ***************************/
protected:
                        uint32_t    currentFrame;
public:
                        uint32_t    getCurrentFrame(void);
                        bool        setCurrentFrame(uint32_t frame);
                        bool        GoToIntra(uint32_t frame);
                        bool        GoToTime(uint64_t time);
                        bool        NextPicture(ADMImage *image);
                        bool        samePicture(ADMImage *image);
                        bool        getCompressedPicure(uint32_t framenum,ADMCompressedImage *img);
                        uint64_t    estimatePts(uint32_t frame);
                        uint32_t    searchFrameBefore(uint64_t pts);
/************************************ Internal ******************************/
public:
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
/************************************ Internal ******************************/
  						uint8_t 	getFrame(uint32_t   framenum,ADMCompressedImage *img,uint8_t *isSequential);
						
   					
	          			uint64_t 	getTime(uint32_t fn);
						uint32_t 	getFlags(uint32_t frame,uint32_t *flags);
						uint8_t   	isSequential (uint32_t framenum);
								// B follow A with just Bframes in between
						uint8_t 	sequentialFramesB(uint32_t frameA,uint32_t frameB);
						uint32_t 	getFlagsAndSeg (uint32_t frame, 
									uint32_t * flags,uint32_t *segs);
						uint8_t  	setFlag(uint32_t frame,uint32_t flags);
						uint8_t	    updateVideoInfo(aviInfo *info);

						uint8_t  	getFrameSize(uint32_t frame,uint32_t *size) ;
						uint8_t		sanityCheckRef(uint32_t start, uint32_t end,
									uint32_t *fatal);
                                                uint8_t         hasPtsDts(uint32_t ); // Return 1 if the container gives PTS & DTS info
                                                uint32_t        ptsDtsDelta(uint32_t framenum) ;
					//*******************************************	
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
        uint64_t        getDurationInUs(void) {return durationInUs;}
        uint8_t			getAudioStream(ADM_audioStream **audio);
virtual WAVHeader       *getInfo(void);
virtual CHANNEL_TYPE    *getChannelMapping(void );
/************************************ /audioStream ******************************/
					//______________________________
					//   /audioStream
					//______________________________
                    uint8_t         getAudioStreamsInfo(uint32_t frame,uint32_t *nbStreams, audioInfo **infos);
                    uint8_t         changeAudioStream(uint32_t frame,uint32_t newstream);
                    uint32_t        getCurrentAudioStreamNumber(uint32_t frame);

                    // /other audio stuff

			     		uint8_t 			setDecodeParam( uint32_t frame );
	 				AVIStreamHeader 	*getVideoStreamHeader(void ) ;
	 				MainAVIHeader 		*getMainHeader(void );
	 				ADM_BITMAPINFOHEADER 	*getBIH(void ) ;

	  				uint8_t			getVideoInfo(aviInfo *info);
                    uint64_t        getVideoDuration(void);

					
					
					
                  //
                  //	Coder/decoder
                  //
		  			// Search previous/ next key frame
		  			uint8_t			getPKFrame(uint32_t *frame);
					uint8_t			getNKFrame(uint32_t *frame);
					//
                  			uint8_t			getUncompressedFrame(uint32_t frame,ADMImage *out,
									uint32_t *flagz=NULL)    ;
protected:					// Obsolete									
              				uint8_t			searchNextKeyFrame(uint32_t in,uint32_t *oseg, uint32_t * orel);
                 			uint8_t			searchPreviousKeyFrame(uint32_t in,uint32_t *oseg, uint32_t * orel);
public:
                  // kludg
                  			void 			propagateBuildMap( void );

			
					uint8_t 		getMarkers(uint32_t *start, uint32_t *end);
								 // get markers from file
					uint8_t 		setPostProc( uint32_t type, uint32_t strength, 
										uint32_t swapuv);
					uint8_t 		getPostProc( uint32_t *type, uint32_t *strength, 
										uint32_t *swapuv);
										
				
					uint8_t			setEnv(_ENV_EDITOR_FLAGS newflag);
					uint8_t			getEnv(_ENV_EDITOR_FLAGS newflag);
					decoders 		*rawGetDecoder(uint32_t frame);

};
#endif
