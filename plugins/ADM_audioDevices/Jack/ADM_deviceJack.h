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

class jackAudioDevice : public audioDevice
{
public:
	jackAudioDevice();
	virtual uint8_t init(uint8_t channel,uint32_t fq);
	virtual uint8_t play(uint32_t len, float *data);
	virtual uint8_t stop();
	uint8_t setVolume(int volume);
	int process(jack_nframes_t nframes);

protected:
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
