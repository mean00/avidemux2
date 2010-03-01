/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "avidemutils.h"
#include "DIA_working.h"
#include "DIA_encoding.h"
#include "ADM_vidMisc.h"

#include <math.h>
#define  ETA_SAMPLE_PERIOD 60000 //Use last n millis to calculate ETA
#define  GUI_UPDATE_RATE 500    // Ms

DIA_encodingBase::DIA_encodingBase( uint64_t duration )
{
        _originalPriority=getpriority(PRIO_PROCESS, 0);
        _totalDurationUs=duration;
#ifdef __WIN32
        _originalPriority=getpriority(PRIO_PROCESS, 0);
#endif
        reset();
}

void DIA_stop( void)
{
	printf("Stop request\n");

}


DIA_encodingBase::~DIA_encodingBase( )
{
#ifdef __WIN32
	setpriority(PRIO_PROCESS, 0, _originalPriority);
#endif
}

void DIA_encodingBase::reset(void)
{
        _lastFrameCount=0;
        _currentFrameCount=0;
        _totalSize=0;
        _audioSize=0;
        _videoSize=0;
        _nextUpdate=GUI_UPDATE_RATE;
        _lastClock=0;
        _fps_average=0;
        clock.reset();

}

void DIA_encodingBase::pushVideoFrame(uint32_t size, uint32_t quant,uint64_t timeUs)
{
          _videoSize+=size;
          _currentFrameCount++;
          _currentDurationUs=timeUs;
}
void DIA_encodingBase::pushAudioFrame(uint32_t size)
{
          _audioSize+=size;
}
void DIA_encodingBase::refresh(void)
{
          uint32_t time=clock.getElapsedMS();
          if(time>_nextUpdate)
          {
                uint32_t deltaTime=time-_lastClock;
                uint32_t deltaFrame=_currentFrameCount-_lastFrameCount;
                if(deltaFrame)
                {
                    deltaFrame*=1000;
                    deltaFrame/=deltaTime;
                    _fps_average=((float)deltaFrame)/1000.;
                    setFps(deltaFrame);
                    float percent=_currentDurationUs/_totalDurationUs;
                    if(percent>1.0) percent=1.0;
                    percent*=100;
                    setPercent((uint32_t)percent);
                }
                _nextUpdate=time+GUI_UPDATE_RATE;
                setAudioSize(_audioSize);
                UI_purge();
          }
}
//EOF

