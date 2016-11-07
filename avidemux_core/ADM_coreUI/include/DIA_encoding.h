/**********************************************************************
            \file            DIA_encoding.h

    
        
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_ENCODING_H
#define ADM_ENCODING_H

#include "ADM_coreUI6_export.h"
#include "ADM_clock.h"
/**
    \class DIA_encodingBase
    \brief Base class for encoding dialog
*/
#define ADM_ENCODING_SAMPLE 32 // must be a power of 2
typedef struct
{
    uint64_t sampleTime;
    uint64_t size;
    uint32_t qz;
}encodingSample;

class ADM_COREUI6_EXPORT DIA_encodingBase
{
protected:
                Clock	  clock;
                
                uint32_t  _lastFrameCount;       // Start frame used to calc. ETA
                uint32_t  _currentFrameCount;    //
                uint32_t  _lastClock;            // Start time used to calc. ETA
                uint32_t  _nextUpdate;           // Next time to update the GUI
                float     _fps_average;
                uint32_t  _average_bitrate;
                uint64_t  _totalDurationUs;
                uint64_t  _currentDts;
                uint64_t  _lastDts;
                uint64_t  _remainingTimeUs;    // ERA
                uint64_t  _totalSize;
                uint64_t  _audioSize;
                uint64_t  _videoSize;
                uint32_t  _originalPriority;
                encodingSample samples[ADM_ENCODING_SAMPLE];
                uint32_t  sampleIndex;
        
public:
                             DIA_encodingBase(uint64_t duration);
                virtual      ~DIA_encodingBase( );
                
                virtual void reset( void );
protected:
                virtual void setFps(uint32_t fps1000)=0;
                
                virtual void setAudioSize(uint64_t size)=0;
                virtual void setVideoSize(uint64_t size)=0;
                virtual void setTotalSize(uint64_t size)=0;
                virtual void setFrameCount(uint32_t nb)=0;
                virtual void setElapsedTimeMs(uint32_t nb)=0;
                virtual void setRemainingTimeMS(uint32_t nb)=0;
                virtual void setAverageQz(uint32_t nb)=0;
                virtual void setAverageBitrateKbits(uint32_t kb)=0;
            

public:
                virtual void setPercent(uint32_t percent)=0;
public:
                virtual void setPhasis(const char *n)=0;
                virtual void setVideoCodec(const char *n)=0;
                virtual void setAudioCodec(const char *n)=0;
                virtual void setContainer(const char *container)=0;
                virtual bool isAlive(void)=0;

                virtual void pushVideoFrame(uint32_t size, uint32_t quant,uint64_t timeUs);
                virtual void pushAudioFrame(uint32_t size);
                virtual void refresh(void);
};
//********************
ADM_COREUI6_EXPORT DIA_encodingBase *createEncoding(uint64_t duration);
#endif
