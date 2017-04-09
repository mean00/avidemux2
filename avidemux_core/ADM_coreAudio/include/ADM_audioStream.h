/***************************************************************************
                          ADM_audioStream.h  -  description
                             -------------------
    copyright            : (C) 2008 by mean
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
#ifndef ADM_audioStream_H
#define ADM_audioStream_H

#include "ADM_coreAudio6_export.h"
#include "ADM_assert.h"
#include "ADM_baseAudioStream.h"
#include "string"


#define ADM_UNKNOWN_LANGUAGE std::string("unknown")

/**
        \fn      ADM_audioAccess
        \brief   Access layer to the file. That one is re-instancied by each demuxer.
                 Some methods are also present in audioStream to allow override or
                        computation when access does not provide it.
*/
#define ADM_AUDIO_NO_DTS ((uint64_t)-1)
class ADM_audioAccess
{
protected:
                        /// must be allocated/freed if needed by derived class
                        uint8_t *extraData;
                        uint32_t extraDataLen;

public:
                                  ADM_audioAccess() {extraData=NULL;extraDataLen=0;}
                virtual           ~ADM_audioAccess() {}
                                    /// Hint, the stream is pure CBR (AC3,MP2,MP3)
                virtual bool      isCBR(void) { return true;}
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return false;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return false;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return false;};
                                    /// Returns audio duration in us
                virtual uint64_t  getDurationInUs(void) {return 0;}
                                    /// Returns length in bytes of the audio stream
                virtual uint32_t  getLength(void){return 0;}
                                    /// Set position in bytes
                virtual bool      setPos(uint64_t pos){ADM_assert(0); return 0;};
                                    /// Get position in bytes
                virtual uint64_t  getPos(){ADM_assert(0); return 0;};
                                    /// Go to a given time
                virtual bool      goToTime(uint64_t timeUs){ADM_assert(0); return false;}
                                    /// Grab extra data
                virtual bool      getExtraData(uint32_t *l, uint8_t **d)
                                    {
                                            *l=extraDataLen;
                                            *d=extraData;
                                            return true;
                                    };


                virtual bool    getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts)=0;
};
/**
        \fn ADM_audioStream
        \brief Base class for audio stream

*/
class ADM_COREAUDIO6_EXPORT ADM_audioStream
{
        protected:
                       WAVHeader                wavHeader;
/// Access will be allocated externally, but will be destroy by ADM_audioStream when it is destroyed
                       ADM_audioAccess          *access;
                       uint32_t                 lengthInBytes;
                       uint64_t                 position;
                       uint64_t                 lastDts;
                       uint64_t                 durationInUs;
                       uint64_t                 lastDtsBase;
                       uint64_t                 sampleElapsed;
                       std::string              language;
                       
    ///
                        void                    setDts(uint64_t newDts);
    /// increment DTS by samples
                       bool                     advanceDtsBySample(uint32_t samples);
    /// Same with provided frequency (SBR)
                       bool                     advanceDtsByCustomSample(uint32_t samples,uint32_t fq);
        public:
/// Default constructor
                       ADM_audioStream(WAVHeader *header,ADM_audioAccess *access);
              virtual  ~ADM_audioStream() ;
/// Returns wavheader
virtual                 WAVHeader                *getInfo(void) {return &wavHeader;}
///  Get a packet
virtual uint8_t         getPacket(uint8_t *buffer,uint32_t *size, uint32_t sizeMax,uint32_t *nbSample,uint64_t *dts);
/// Go to a given time, in microseconds
virtual bool            goToTime(uint64_t nbUs);
/// Returns current time in us. Not used.
//virtual uint8_t         getTime(uint64_t *nbUs);
/// Returns extra configuration data
virtual bool            getExtraData(uint32_t *l, uint8_t **d);
/// Returns or compute duration. If the access cannot provide it, it will be computed here
        uint64_t        getDurationInUs(void) {return durationInUs;}
virtual bool            isCBR()
                            {
                                if(!access) return false;
                                return access->isCBR();
                            }
virtual        const std::string &getLanguage() {return language;}
virtual        void              setLanguage(const std::string &lan) {language=lan;}
virtual        bool              isLanguageSet(void);
};
/**
   \fn ADM_audioCreateStream
    \brief Create the appropriate audio stream. It will be a derivated class of audioStream if possible (MP3/AC3)
*/
ADM_COREAUDIO6_EXPORT ADM_audioStream  *ADM_audioCreateStream(WAVHeader *wavheader, ADM_audioAccess *access,bool createTimeMap=true);
#endif
// EOF

