//
// C++ Interface: ADM_deviceWin32
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ADM_DEVICEWIN32_H
#define ADM_DEVICEWIN32_H
class win32AudioDevice : public audioDeviceThreaded
{
protected:
	uint8_t	    _inUse;
	virtual     bool     localInit(void);
    virtual     bool     localStop(void);
    virtual     void     sendData(void); 
    virtual     const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
public:
    uint8_t setVolume(int volume);
	win32AudioDevice(void);

};
#endif


