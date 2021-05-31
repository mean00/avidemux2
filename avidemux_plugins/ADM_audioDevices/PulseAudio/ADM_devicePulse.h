/***************************************************************************
                          ADM_devicePulse.h  -  description
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
#ifndef ADM_devicePulse_H
#define ADM_devicePulse_H
class pulseAudioDevice : public audioDeviceThreaded
{
private:
                void    *stream;
                void    *connection;
                void    *mainloop;
                bool    muted;
protected:
    virtual     bool    localInit(void);
    virtual     bool    localStop(void);
    virtual     void    sendData(void);
    virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
public:
                pulseAudioDevice(void);
    virtual     uint8_t  setVolume(int volume);
                uint32_t getLatencyMs(void);
};
#endif
