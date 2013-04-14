//
// C++ Interface: ADM_audiodevice
//
// Description: 
//
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ADM_AUDIODEVICE_H
#define ADM_AUDIODEVICE_H

// Converts float to int16_t with dithering
#include "ADM_coreAudioDevice6_export.h"
#include "ADM_coreAudio.h"
#include "ADM_threads.h"
#include "ADM_byteBuffer.h"

#define AUDIO_DEVICE_STOPPED  0
#define AUDIO_DEVICE_STARTED  1
#define AUDIO_DEVICE_STOP_REQ 2
#define AUDIO_DEVICE_STOP_GR  3

ADM_COREAUDIODEVICE6_EXPORT uint8_t ADM_av_loadPlugins(const char *path);

/**
    \class audioDevice
    \brief Base class for audioDeviceThreaded, the one actually used.
*/
 class audioDevice
 {
        protected:
                        uint32_t _channels; /// # of channels we want to setup
                        uint32_t _frequency;/// Frequency we want to setup

        public:
                                        audioDevice(void) {};
                        virtual         ~audioDevice() {};
                        virtual uint8_t  init(uint32_t channel, uint32_t fq ,CHANNEL_TYPE *channelMapping) =0;
                        virtual uint8_t  stop(void)=0;
                        virtual uint8_t  play(uint32_t len, float *data) =0;
                        virtual uint8_t  setVolume(int volume) {return 1;}
                        virtual uint32_t getLatencyMs(void) {return 0;}
}   ;

/**
    \class audioDeviceThreaded
    \brief Audio is run in a separate thread

*/
#define ADM_THREAD_BUFFER_SIZE (8*1024*1024)
#define MODULO_THREADED(x)  (x&(ADM_THREAD_BUFFER_SIZE-1))
class ADM_COREAUDIODEVICE6_EXPORT audioDeviceThreaded: public audioDevice
{
protected:
            CHANNEL_TYPE incomingMapping[MAX_CHANNELS];
            uint32_t    rdIndex;
            uint32_t    wrIndex;
            ADM_byteBuffer     audioBuffer;
            admMutex    mutex;
            uint8_t     stopRequest;
            pthread_t   myThread;
            uint32_t    sizeOf10ms; /// Nb of bytes to make 10 ms
            ADM_byteBuffer  silence;   /// Silence
public:
                            audioDeviceThreaded();
    virtual                 ~audioDeviceThreaded() ;
    virtual     bool        writeData(uint8_t *data,uint32_t lenInByte);
    virtual     bool        readData(uint8_t *data,uint32_t lenInByte);
                uint32_t    getBufferFullness(void); /// Returns the number of ms of audio in the buffer
protected:
    //
    virtual     bool     localInit(void)=0;
    virtual     bool     localStop(void)=0;
    virtual     void     sendData(void)=0;   
    virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels)=0;

public:
    virtual     void        Loop(void);
    virtual     uint8_t     init(uint32_t channel, uint32_t fq,CHANNEL_TYPE *channelMapping );
    virtual     uint8_t     stop(void);

    virtual     uint8_t     play(uint32_t len, float *data);
    virtual     bool        getVolumeStats(uint32_t *vol); // Vol is 6 channels between 0 and 255
};

/**
    \class dummyAudioDevice
    \brief this dummy is used when no suitable device have been found.
*/
class dummyAudioDevice : public audioDeviceThreaded
{
 static const   CHANNEL_TYPE myChannelType[MAX_CHANNELS];
		  protected:
                    virtual     bool     localInit(void);
                    virtual     bool     localStop(void);
                    virtual     void     sendData(void);    
                    virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels){return myChannelType;}
}   ;

#endif
