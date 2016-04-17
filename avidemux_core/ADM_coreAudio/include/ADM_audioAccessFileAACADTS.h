/***************************************************************************
    \file ADM_audioAccessfileAACADTS
    \brief read audio from a file, AAC inside ADTS
    \author mean (c) 2012 fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once
#include "ADM_coreAudio6_export.h"
#include "ADM_audioStream.h"
#include "vector"
#include "ADM_audioClock.h"
#include "ADM_aacadts.h"
#include <vector>

/**
        \fn      ADM_audioAccessFile
        \brief   Input is a plain file
*/

class aacAdtsSeek
{
public:
    uint64_t position;
    uint64_t dts;
};

class ADM_COREAUDIO6_EXPORT ADM_audioAccessFileAACADTS  : public ADM_audioAccess
{
protected:
                FILE            *_fd;
                uint64_t        payloadSize;
                uint64_t        fileSize;
                uint64_t        durationUs;
                bool            inited;
                audioClock      *clock;
                ADM_adts2aac    *aac;
                bool            refill(void);
                WAVHeader       headerInfo;
                 std::vector<aacAdtsSeek>seekPoints;

protected:
                bool            init(void);

public:
                                  ADM_audioAccessFileAACADTS(const char *fileName,int offset);
                virtual           ~ADM_audioAccessFileAACADTS() ;
                
            const WAVHeader       &getHeaderInfo() {return headerInfo;};
                
                virtual bool        getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,
                            uint64_t *dts);
                                    /// Go to a given time
                virtual bool      goToTime(uint64_t timeUs);

                // stubs
                                    /// Hint, the stream is pure CBR (AC3,MP2,MP3)
                virtual bool      isCBR(void) { return false;}
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return true;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return false;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return true;};
                                    /// Returns audio duration in us
                virtual uint64_t  getDurationInUs(void) {return durationUs;}

                                    /// Grab extra data
                virtual bool      getExtraData(uint32_t *l, uint8_t **d)
                                    {
                                            *l=extraDataLen;    
                                            *d=extraData;
                                            return true;
                                    };
                                    /// Returns length in bytes of the audio stream
                virtual uint32_t  getLength(void) {return payloadSize;};

                                    /// Set position in bytes
                virtual bool      setPos(uint64_t pos) { ADM_assert(0);return true;};
                                    /// Get position in bytes
                virtual uint64_t  getPos() { ADM_assert(0);return true;};

                
};

// EOF

