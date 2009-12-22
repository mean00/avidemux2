/***************************************************************************
                          ADM_deviceEsd.h  -  description
                             -------------------
                             Audio device for PulseAudio sound daemon
    copyright            : (C) 2005/2008 by mean
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
#ifndef ADM_devicePulseSimple_H
#define ADM_devicePulseSimple_H
class pulseSimpleAudioDevice : public audioDeviceThreaded
 {
     protected :
                     void    *instance;
                     uint32_t latency;
         virtual     bool     localInit(void);
         virtual     bool     localStop(void);
         virtual     void     sendData(void); 
         virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
      public:
                pulseSimpleAudioDevice(void);
                
                uint32_t getLatencyMs(void);
     }     ;
#endif
