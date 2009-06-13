/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>

//#include "prefs.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "avidemutils.h"
#include "DIA_working.h"
#include "DIA_encoding.h"
#include "ADM_vidMisc.h"


DIA_encodingBase::DIA_encodingBase( uint32_t fps1000 )
{

        _lastnb=0;
        _totalSize=0;
        _audioSize=0;
        _videoSize=0;
        _current=0;
        setFps(fps1000);
        _originalPriority=getpriority(PRIO_PROCESS, 0);
        _lastTime=0;
        _lastFrame=0;
        _fps_average=0;
        _total=1000;
}

void DIA_encodingBase::setFps(uint32_t fps)
{
        _roundup=(uint32_t )floor( (fps+999)/1000);
        _fps1000=fps;
        ADM_assert(_roundup<MAX_BR_SLOT);
        memset(_bitrate,0,sizeof(_bitrate));
        _bitrate_sum=0;
        _average_bitrate=0;
        
}
void DIA_stop( void)
{
	printf("Stop request\n");

}


DIA_encodingBase::~DIA_encodingBase( )
{

	setpriority(PRIO_PROCESS, 0, _originalPriority);

}

void DIA_encodingBase::setPhasis(const char *n)
{

}
void DIA_encodingBase::setAudioCodec(const char *n)
{

}

void DIA_encodingBase::setCodec(const char *n)
{

}
void DIA_encodingBase::reset(void)
{
          _totalSize=0;
          _videoSize=0;
          _current=0;
}
void DIA_encodingBase::setContainer(const char *container)
{
}
#define  ETA_SAMPLE_PERIOD 60000 //Use last n millis to calculate ETA
#define  GUI_UPDATE_RATE 500  

void DIA_encodingBase::setFrame(uint32_t nb,uint32_t size, uint32_t quant,uint32_t total)
{
          _total=total;
          _videoSize+=size;
          if(nb < _lastnb || _lastnb == 0) // restart ?
           {
                _lastnb = nb;
                clock.reset();
                _lastTime=clock.getElapsedMS();
                _lastFrame=0;
                _fps_average=0;
                _videoSize=size;
    
                _nextUpdate = _lastTime + GUI_UPDATE_RATE;
                _nextSampleStartTime=_lastTime + ETA_SAMPLE_PERIOD;
                _nextSampleStartFrame=0;
          } 
          _lastnb = nb;
          _current=nb%_roundup;
          _bitrate[_current].size=size;
          _bitrate[_current].quant=quant;
}
void DIA_encodingBase::setAudioSize(uint32_t size)
{
      _audioSize=size;
}
uint8_t DIA_encodingBase::isAlive( void )
{
	return 0;
}
void DIA_encodingBase::setPercent(uint32_t percent)
{

}
//EOF

