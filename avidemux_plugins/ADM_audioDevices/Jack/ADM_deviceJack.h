//
// Author: Mihail Zenkov <mihail.zenkov@gmail.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//



#include <jack/jack.h>
#include <jack/ringbuffer.h>

#ifdef USE_SRC
#include <samplerate.h>
#endif

#define MAX_CHANNELS 9

class jackAudioDevice : public audioDeviceThreaded
{
public:
	jackAudioDevice();
protected:
    virtual     bool     localInit(void);
    virtual     bool     localStop(void);
    virtual     void     sendData(void); 
                uint8_t  setVolume(int volume);
                int      process(jack_nframes_t nframes);
    virtual const CHANNEL_TYPE *getWantedChannelMapping(uint32_t channels);

protected:

    bool tempplay(uint32_t len, float *data); // FIXME: DOES NOT WORK!

	static void jack_shutdown(void *arg);
	static int process_callback(jack_nframes_t nframes, void *arg);

	jack_port_t *ports[MAX_CHANNELS];
	jack_client_t *client;
	jack_ringbuffer_t *ringbuffer;
	#ifdef USE_SRC
	float *src_out_buf;
	SRC_STATE *src_state;
	SRC_DATA src_data;
	#endif
};
