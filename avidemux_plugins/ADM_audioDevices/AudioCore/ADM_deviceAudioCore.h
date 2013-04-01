//
// C++ Interface: ADM_deviceAudioCore
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
class coreAudioDevice : public audioDeviceThreaded
 {
     protected :
                     bool     _inUse;
         virtual     bool     localInit(void);
         virtual     bool     localStop(void);
         virtual     void     sendData(void); 
         virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
      public:
                                coreAudioDevice(void);  
         virtual                ~coreAudioDevice();
                
                uint32_t getLatencyMs(void);
                uint8_t  setVolume(int volume);
     }     ;
