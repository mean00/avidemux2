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
 #include "ADM_codec.h"
 #include "ADM_image.h"
 #include "../ADM_editor/ADM_edCache.h"
 #include "ADM_pp.h"
 #include "ADM_colorspace.h"

 #include "ADM_audioStream.h"
 #include "ADM_audiocodec/ADM_audiocodec.h"
 #include "ADM_segment.h"
 #include <vector>

#define ADM_EDITOR_AUDIO_BUFFER_SIZE (128*1024*6*sizeof(float))
#define AVS_PROXY_DUMMY_FILE "::ADM_AVS_PROXY::" 
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
                    std::string currentProjectName;
  private:
//*********************************PRIVATE API *******************************************
                    //bool		decodeCache(uint32_t ref, uint32_t frame,ADMImage *image);
                    bool        switchToNextSegment(bool dontdecode=false);
                    bool        switchToSegment(uint32_t s,bool dontdecode=false);
                    uint32_t    currentFrame;

                    bool        nextPictureInternal(uint32_t ref,ADMImage *image);
                    bool        samePictureInternal(uint32_t ref,ADMImage *image);
                    bool        seektoTime(uint32_t ref,uint64_t timeToSeek,bool dontdecode=false);
                    // Some useful functions...
                    void        recalibrate(uint64_t *time,_SEGMENT *seg);
                    void        recalibrateSigned(int64_t *time,_SEGMENT *seg);
                    bool        updateImageTiming(_SEGMENT *seg,ADMImage *image);
                    // Need to get the image just before targetPts
                    bool        decodeTillPictureAtPts(uint64_t targetPts,ADMImage *image);

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
                                
                    

//******************************************************************************************
  private:
                    ADM_EditorSegment _segments;
                    uint8_t     dupe(ADMImage *src,ADMImage *dst,_VIDEOS *vid); 
                    uint32_t	_internalFlags;  // Flags :
                    ADM_PP      _pp;             // Postprocessing settings
                    ADMImage	*_imageBuffer;   // Temp buffer used for decoding
                    uint64_t    _currentPts;        // Current image PTS
                    uint32_t    _currentSegment;    // Current video segment
                    int64_t     _nextFrameDts;      // COPYMODE Used in copy mode to fill the missing timestamp
                                                    // Warning, it is actually the DTS of the NEXT frame to fetch
//****************************** Audio **********************************
                    // _audiooffset points to the offset / the total segment
                    // not the used part !
                    uint32_t  _audioSeg;
                    int64_t   _audioSample;

                    ADM_audioStreamTrack *getTrack(uint32_t i);
                    ADMImage    *_scratch;																		;
                    uint8_t  	updateAudioTrack(uint32_t seg);
                    bool        switchToNextAudioSegment(void);
//****************************** Audio **********************************
                    void        deleteAllVideos(void );
                    uint8_t     getMagic(const char *name,uint32_t *magic);
                    uint32_t    searchForwardSeg(uint32_t startframe);
                    bool        rederiveFrameType(vidHeader *demuxer);

  public:
                    bool        hasVBRAudio(void);
                    bool     	getExtraHeaderData(uint32_t *len, uint8_t **data);
                    uint32_t    getPARWidth(void);
                    uint32_t    getPARHeight(void);
                    bool        rebuildDuration(void);
                                ADM_Composer();
virtual                         ~ADM_Composer();
                    void        clean( void );
                    uint8_t     saveAsScript (const char *name, const char *out);
                    bool        saveAsPyScript(const char *name);
                    uint8_t     resetSeg( void );
                    bool     	addFile (const char *name);
                    uint8_t 	cleanup( void);
                    bool        isMultiSeg( void);
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
public:

                    uint64_t    getCurrentFramePts(void);
                    bool        goToTimeVideo(uint64_t time);
                    bool        goToIntraTimeVideo(uint64_t time);
                    bool        nextPicture(ADMImage *image,bool dontcross=false);
                    bool        samePicture(ADMImage *image);
                    bool        previousPicture(ADMImage *image);
                    bool        rewind(void);
// Used for stream copy
                    bool        GoToIntraTime_noDecoding(uint64_t time,uint32_t *toframe=NULL);
                    bool        getCompressedPicture(uint64_t delay,ADMCompressedImage *img);     //COPYMODE                
                    // Use only for debug purpose !!!
                    bool        getDirectImageForDebug(uint32_t frameNum,ADMCompressedImage *img);             
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
                    bool            getPCMPacket(float  *dest, uint32_t sizeMax, uint32_t *samples,uint64_t *odts);
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
                    bool            setDecodeParam( uint64_t frameTime );
/**************************************** Video Info **************************/
                    AVIStreamHeader 	*getVideoStreamHeader(void ) ;
                    MainAVIHeader 		*getMainHeader(void );
                    ADM_BITMAPINFOHEADER 	*getBIH(void ) ;

                    uint8_t			getVideoInfo(aviInfo *info);
                    uint64_t        getVideoDuration(void);
                    uint64_t        getFrameIncrement(void); /// Returns the # of us between 2 frames or the smaller value of them

/**************************************** /Video Info **************************/					
/***************************************** Project Handling ********************/
public:
                    const std::string   &getProjectName(void);
                    bool                 setProjectName(const std::string &name);
					
/***************************************** Seeking *****************************/            
public:
                    bool                getNKFramePTS(uint64_t *frameTime);
                    bool                getPKFramePTS(uint64_t *frameTime);
                    bool                getDtsFromPts(uint64_t *time);
                                        /// Returns pts-dts for given frame
                    bool		        getPtsDtsDelta(uint64_t *frameTime);
/******************************* Post Processing ************************************/
                    uint8_t             setPostProc( uint32_t type, uint32_t strength,	uint32_t swapuv);
                    uint8_t             getPostProc( uint32_t *type, uint32_t *strength,uint32_t *swapuv);
/******************************* /Post Processing ************************************/	
/******************************* Editing ************************************/	
                    bool                remove(uint64_t start,uint64_t end);
                    bool                addSegment(uint32_t ref, uint64_t startRef, uint64_t duration);
                    bool                clearSegment(void);
                    uint32_t            getNbSegment(void)
                                        {
                                            return _segments.getNbSegments();
                                        }
// For js
                    bool                dumpRefVideos(void);
                    bool                dumpSegments(void);
                    bool                dumpSegment(int i);
                    bool                dumpTiming(void);
                    bool                getVideoPtsDts(uint32_t frame, uint32_t *flags,uint64_t *pts, uint64_t *dts);
/******************************* /Editing **********************************/										
/******************************* Misc ************************************/				
                    uint8_t             setEnv(_ENV_EDITOR_FLAGS newflag);
                    uint8_t             getEnv(_ENV_EDITOR_FLAGS newflag);
/******************************* /Misc ************************************/				
/******************************** Info ************************************/
                    const char          *getVideoDecoderName(void);
};
#endif
