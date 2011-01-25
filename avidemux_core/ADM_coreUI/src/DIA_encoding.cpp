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
#define  GUI_UPDATE_RATE 1000    // Ms
extern void UI_purge(void);

#define aprintf(...) {}
/**
    \fn DIA_encodingBase
*/
DIA_encodingBase::DIA_encodingBase( uint64_t duration )
{
        #ifndef __HAIKU__
        _originalPriority=getpriority(PRIO_PROCESS, 0);
        #endif
        _totalDurationUs=duration;
#ifdef __WIN32
        _originalPriority=getpriority(PRIO_PROCESS, 0);
#endif
        reset();
}
/**
    \fn DIA_encodingBase
*/

DIA_encodingBase::~DIA_encodingBase( )
{
#ifdef __WIN32
	setpriority(PRIO_PROCESS, 0, _originalPriority);
#endif
}
/**
    \fn reset
*/

void DIA_encodingBase::reset(void)
{
        _lastFrameCount=0;
        _currentFrameCount=0;
        _currentDts=0;
        _lastDts=0;
        _totalSize=0;
        _audioSize=0;
        _videoSize=0;
        _nextUpdate=GUI_UPDATE_RATE;
        _lastClock=0;
        _fps_average=0;
        _remainingTimeUs=0;
        sampleIndex=0;
        memset(samples,0,sizeof(samples));
        clock.reset();
        UI_purge();
}
/**
    \fn pushVideoFrame
*/

void DIA_encodingBase::pushVideoFrame(uint32_t size, uint32_t quant,uint64_t timeUs)
{
          _videoSize+=size;
          _currentFrameCount++;
          _currentDts=timeUs;
          encodingSample *cur=samples+(sampleIndex%ADM_ENCODING_SAMPLE);
          cur->qz=quant;
          cur->sampleTime=timeUs;
          cur->size=_videoSize;
          sampleIndex++;
}
/**
    \fn pushAudioFrame
*/

void DIA_encodingBase::pushAudioFrame(uint32_t size)
{
          _audioSize+=size;
}
/**
    \fn refresh
*/

void DIA_encodingBase::refresh(void)
{
          uint32_t time=clock.getElapsedMS();
          if(time>_nextUpdate)
          {
                uint32_t deltaTime=time-_lastClock;
                uint32_t deltaFrame=_currentFrameCount-_lastFrameCount;
                uint64_t deltaDts=_currentDts-_lastDts;
                if(sampleIndex>ADM_ENCODING_SAMPLE)
                {
                    uint32_t qSum=0;
                    for(int i=0;i<ADM_ENCODING_SAMPLE;i++)
                            qSum+=samples[i].qz;
                    qSum/=ADM_ENCODING_SAMPLE;
                    aprintf("Q:%d\n",qSum);
                    setAverageQz(qSum);
                }

                if(sampleIndex>ADM_ENCODING_SAMPLE)
                {
                    int start=sampleIndex%ADM_ENCODING_SAMPLE;
                    int end=(sampleIndex+ADM_ENCODING_SAMPLE-1)%ADM_ENCODING_SAMPLE;
                    uint64_t deltaTime=samples[end].sampleTime-samples[start].sampleTime;
                    uint64_t deltaSize=samples[end].size-samples[start].size;
                    aprintf("dTime:%d dSize:%d\n",deltaTime,deltaSize);
                    if(deltaTime>1000)
                    {
                        float delta;
                        delta=deltaSize;
                        delta/=deltaTime;
                        delta*=8; // byte -> bit
                        delta*=1000; // b/us -> kb/s
                        aprintf("br:%d\n",(int)delta);
                        setAverageBitrateKbits((uint32_t)delta);
                    }
                }
                if(deltaFrame)
                {
                    float thisAverage;
                    //printf("**********************************DFrame=%d, DTime=%d\n",(int)deltaFrame,(int)deltaTime);
                    thisAverage=((float)deltaFrame);
                    thisAverage/=deltaTime;
                    thisAverage*=1000;
                    _fps_average=_fps_average*0.5+0.5*thisAverage;
                    //printf("************** Fps:%d\n",(int)_fps_average);
                    setFps(_fps_average);
                    float percent=(float)_currentDts/(float)_totalDurationUs;
                    if(percent>1.0) percent=1.0;
                    percent*=100;
                    setPercent((uint32_t)percent);
                    setFrameCount(_currentFrameCount);
                    setElapsedTimeMs(time);
                }
                if(deltaDts )
                {
                    float dtsPerSec=deltaDts;
                    dtsPerSec/=deltaTime;
                    dtsPerSec/=1000.;  // dts advance per second
                    float leftDts=_totalDurationUs-_currentDts;
                    //printf("***************%u to encoding\n",(int)(leftDts/1000000));
                    //printf("Advanc=%d ms/sec\n",(int)(dtsPerSec*1000));
                    if(dtsPerSec>0.01)
                    {
                        leftDts=leftDts/dtsPerSec;
                        _remainingTimeUs=(_remainingTimeUs/2)+(leftDts/2);
                        leftDts=_remainingTimeUs;
                        leftDts/=1000.; // us -> ms
                        //printf("***************%u s left\n",(int)(leftDts/1000));
                        setRemainingTimeMS((uint32_t)leftDts);
                    }
                    
                }
                _nextUpdate=time+GUI_UPDATE_RATE;
                setAudioSize(_audioSize);
                setVideoSize(_videoSize);
                setTotalSize(_audioSize+_videoSize);
                _lastFrameCount=_currentFrameCount;
                _lastDts=_currentDts;
                _lastClock=time;
           
          }
          UI_purge();
}
//EOF

