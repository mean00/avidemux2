/***************************************************************************
    \file ADM_audioAccessfile
    \brief read audio from a file
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
#ifndef ADM_audioStreamFile_H
#define ADM_audioStreamFile_H

#include "ADM_audioStream.h"
/**
        \fn      ADM_audioAccessFile
        \brief   Input is a plain file
*/

class ADM_audioAccessFile  : public ADM_audioAccess
{
protected:
                        /// must be allocated/freed if needed by derived class
                        FILE     *_fd;
                        uint64_t fileLength;

public:
                                  ADM_audioAccessFile(const char *fileName);
                virtual           ~ADM_audioAccessFile() ;
                                    /// Hint, the stream is pure CBR (AC3,MP2,MP3)
                virtual bool      isCBR(void) { return false;}
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return false;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return true;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return false;};
                                    /// Returns audio duration in us
                virtual uint64_t  getDurationInUs(void) {return 0;}

                                    /// Go to a given time
                virtual bool      goToTime(uint64_t timeUs){ADM_assert(0); return false;}
                                    /// Grab extra data
                virtual bool      getExtraData(uint32_t *l, uint8_t **d)
                                    {
                                            *l=extraDataLen;    
                                            *d=extraData;
                                            return true;
                                    };
                                    /// Returns length in bytes of the audio stream
                virtual uint32_t  getLength(void) {return fileLength;};

                                    /// Set position in bytes
                virtual bool      setPos(uint64_t pos);
                                    /// Get position in bytes
                virtual uint64_t  getPos();

                
                virtual bool    getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,
                                            uint64_t *dts);
};
#endif
// EOF

