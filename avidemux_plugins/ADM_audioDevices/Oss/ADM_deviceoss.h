/***************************************************************************
                          ADM_deviceoss.h  -  description
                             -------------------
    begin                : Sat Sep 28 2002
    copyright            : (C) 2002 by mean
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
#ifndef OSSAUDIODEVICE_H
#define OSSAUDIODEVICE_H
class ossAudioDevice : public audioDeviceThreaded
{
protected :
                                int oss_fd;
    virtual     bool     localInit(void);
    virtual     bool     localStop(void);
    virtual     void     sendData(void); 
    virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
public:   
			uint8_t setVolume(int volume);
}     ;

#endif
