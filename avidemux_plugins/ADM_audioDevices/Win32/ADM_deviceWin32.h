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
#pragma once
#define NB_BUCKET 2


class win32AudioDevice : public audioDeviceThreaded
{
protected:
                uint8_t  _inUse;
    virtual     bool     localInit(void);
    virtual     bool     localStop(void);
    virtual     void     sendData(void);
    virtual     const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
public:
                uint8_t setVolume(int volume);
                        win32AudioDevice(void);
                        ~win32AudioDevice();

protected:
                uint32_t bucketSize;
                HWAVEOUT myDevice;
                MMRESULT myError;
                WAVEHDR  waveHdr[NB_BUCKET];
protected:
                 void handleMM(MMRESULT err);

protected:
                int     findFreeBucket();


};
