/***************************************************************************
            \file audiofilter_thread.h
            \brief Wrap an audio access inside its own thread
            \author (C) Mean 2010 fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef AUDM_ACCESS_THREAD_H
#define AUDM_ACCESS_THREAD_H

#include "ADM_audioStream.h"
#include "ADM_threadQueue.h"


/**
    \class ADM_audioAccess_thread
    \brief Wrap ADM_audioAccess inside a thread

*/
class ADM_audioAccess_thread : public ADM_audioAccess,public ADM_threadQueue
{
  protected:
                ADM_audioAccess   *son;
              
  public:


                                    ADM_audioAccess_thread(ADM_audioAccess *son) ;
                virtual           ~ADM_audioAccess_thread();
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return son->canSeekTime();};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return son->canSeekOffset();};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return son->canGetDuration();};
                                    /// Returns length in bytes of the audio stream
                virtual uint32_t  getLength(void){return son->getLength();}
                                    /// Set position in bytes
                virtual bool      setPos(uint64_t pos) ;
                                    /// Get position in bytes
                virtual uint64_t  getPos(void);
                                    /// Grab extra data
                virtual bool      getExtraData(uint32_t *l, uint8_t **d);

                virtual bool      getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts);
                virtual bool      isCBR(void) {return son->isCBR();};
                virtual bool      runAction(void);
};


#endif

