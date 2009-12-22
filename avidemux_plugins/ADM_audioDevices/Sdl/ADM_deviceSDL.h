/**
    \file ADM_deviceSDL.h
    \brief SDL audio device plugin

    (C) Mean 2008, fixounet@free.fr
    GPL-v2

*/
#ifndef ADM_deviceSDL_H
#define ADM_deviceSDL_H

class sdlAudioDevice : public audioDeviceThreaded
 {
     protected :
                bool       active;

                virtual     bool     localInit(void);
                virtual     bool     localStop(void);
                virtual     void     sendData(void);    
                                     
                virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);
     public:
                uint8_t callback( Uint8 *stream, int len);
                        sdlAudioDevice();
     }     ;
#endif

