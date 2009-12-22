/***************************************************************************
                          ADM_deviceEsd.h  -  description
                             -------------------
                             Audio device for ESD sound daemon
    copyright            : (C) 2005 by mean
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
#ifndef ADM_deviceEsd_H
#define ADM_deviceEsd_H
class esdAudioDevice : public audioDeviceThreaded
	 {
		 protected :
                    int esdDevice;
                    int esdServer;
                    uint32_t latency;
         virtual     bool     localInit(void);
         virtual     bool     localStop(void);
         virtual     void     sendData(void); 
         virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
		  public:
		  			esdAudioDevice(void) {esdDevice=-1;esdServer=-1;}
		     		 
                            uint32_t getLatencyMs(void);
		 }     ;
#endif
