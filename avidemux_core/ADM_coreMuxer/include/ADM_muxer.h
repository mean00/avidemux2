/**
 *      \file ADM_muxer.h
 *      \brief External interface for muxers
 *      (c) Mean 2008
 *
 */
#ifndef ADM_muxer_H
#define ADM_muxer_H
#include "ADM_default.h"
#include "ADM_audioStream.h"
#include "DIA_working.h"
#include "DIA_coreToolkit.h"
#include "DIA_encoding.h"

/**
    \class ADM_videoStream

*/
class ADM_videoStream
{
protected:
            uint32_t width,height,averageFps1000;
            uint32_t fourCC;
            bool     isCFR;
            uint64_t videoDelay;


            DIA_workingBase *encoding;
public:
                      ADM_videoStream() {videoDelay=0;} ;
            virtual ~ADM_videoStream() {};
            uint32_t getWidth(void) {return width;}
            uint32_t getHeight(void) {return height;}
            uint32_t getFCC(void) {return fourCC;}
            bool     getIsCfr(void) {return isCFR;}
            uint32_t getAvgFps1000(void) {return averageFps1000;}
            uint64_t getVideoDelay(void) {return videoDelay;}

virtual     bool     getPacket(uint32_t *len, uint8_t *data, uint32_t maxLen,uint64_t *pts,uint64_t *dts,
                                        uint32_t *flags)=0;
virtual     bool     getExtraData(uint32_t *extraLen, uint8_t **extraData) {*extraLen=0;*extraData=NULL;return true;};
virtual     bool     providePts(void) {return false;}
virtual     uint64_t getVideoDuration(void) {return 1;}
};
/**
 *      \class ADM_muxer
 *
 */
class ADM_muxer
{
protected:
                ADM_videoStream *vStream;       // Internal copy of the parameters
                ADM_audioStream  **aStreams;
                uint32_t         nbAStreams;


                uint64_t videoIncrement; // Used/set by initUI
                uint64_t videoDuration;
                DIA_workingBase  *encoding;
                
public:
                          ADM_muxer() {vStream=NULL;aStreams=NULL;nbAStreams=0;encoding=NULL;};
        virtual           ~ADM_muxer() {closeUI();};
        virtual bool      open(const char *filename,   ADM_videoStream *videoStream,
                                uint32_t nbAudioTrack, ADM_audioStream **audioStreams)=0;

        virtual  bool     save(void)=0;
        virtual  bool     close(void)=0; 
        
        virtual  bool     initUI(const char *title);
        virtual  bool     updateUI(uint64_t time);
        virtual  bool     closeUI(void);
        virtual  bool     useGlobalHeader(void) {return false;}

};
#endif

