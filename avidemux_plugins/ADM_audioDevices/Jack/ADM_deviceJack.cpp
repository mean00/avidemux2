//
// Author: Mihail Zenkov <mihail.zenkov@gmail.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//


#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#include "ADM_default.h"
#include "ADM_audiodevice.h"
#include "ADM_assert.h"
#include "ADM_deviceJack.h"


#define BUFSIZE 16385

jackAudioDevice::jackAudioDevice()
{
	client = NULL;
	ringbuffer = NULL;
	#ifdef USE_SRC
	src_out_buf = NULL;
	src_state = NULL;
	#endif
}

void jackAudioDevice::jack_shutdown(void *arg)
{
	((jackAudioDevice*)arg)->stop();
}

uint8_t jackAudioDevice::stop()
{
	if (client) {
		printf("[JACK] Stop\n");
		jack_client_close(client);
		client = NULL;
		if (ringbuffer)
			jack_ringbuffer_free(ringbuffer);
		ringbuffer = NULL;
		#ifdef USE_SRC
		delete src_out_buf;
		src_out_buf = NULL;
		src_delete(src_state);
		src_state = NULL;
		#endif
	}

	return 1;
}

uint8_t jackAudioDevice::init(uint8_t channels, uint32_t fq)
{
	jack_status_t status;
	_channels = channels;

	if (sizeof(jack_default_audio_sample_t) != sizeof(float)) {
		printf("[JACK] jack_default_audio_sample_t != float\n");
		return 0;
	}

	client = jack_client_open("avidemux", JackNullOption, &status, NULL);

	if (client == NULL) {
		printf("[JACK] jack_client_open() failed, status = 0x%2.0x\n", status);
		if (status & JackServerFailed)
			printf(("[JACK] Unable to connect to server\n"));
		return 0;
	}

	if (status & JackServerStarted)
		printf("[JACK] Server started\n");

	if (jack_get_sample_rate(client) == fq) {
		jack_set_process_callback(client, process_callback, this);
	} else {
		printf("[JACK] audio stream sample rate: %i\n", fq);
		printf("[JACK] jack server sample rate: %i\n", (int)jack_get_sample_rate(client));
		#ifdef USE_SRC
			src_out_buf = new float[BUFSIZE * channels];
			src_state = src_new(SRC_SINC_FASTEST, channels, NULL);
			if (!src_state) {
				printf("[JACK] Can't init libsamplerate\n");
				stop();
				return 0;
			}
			src_data.data_out = src_out_buf;
			src_data.output_frames = BUFSIZE;
			src_data.src_ratio = jack_get_sample_rate(client) / (double)fq;
			src_data.end_of_input = 0;
//			printf("[JACK] ratio: %f\n", src_data.src_ratio);
		#else
			printf("[JACK] For play this, you need avidemux compiled with libsamplerate support\n");
			stop();
			return 0;
		#endif
	}

	ringbuffer = jack_ringbuffer_create(BUFSIZE * channels * sizeof(jack_default_audio_sample_t));

	jack_set_process_callback(client, process_callback, this);
	jack_on_shutdown(client, jack_shutdown, this);

	char name[10];
	for (int i = 0; i < channels; i++) {
		snprintf(name, 10, "output-%d", i);
		ports[i] = jack_port_register(client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		if (!ports[i]) {
			printf("[JACK] Can't create new port\n");
			stop();
			return 0;
		}
	}

	if (jack_activate(client)) {
		printf("[JACK] Cannot activate client\n");
		stop();
		return 0;
	}

	const char **input_ports = jack_get_ports(client, NULL, NULL, JackPortIsInput|JackPortIsPhysical);

	for (int i = 0; i < channels && input_ports[i]; i++) {
		if (jack_connect(client, jack_port_name(ports[i]), input_ports[i]))
			printf("[JACK] Connecting failed\n");
	}

	if (channels == 1 && input_ports[1])
		if (jack_connect(client, jack_port_name(ports[0]), input_ports[1]))
			printf("[JACK] Connecting failed\n");

	return 1;
}

int jackAudioDevice::process(jack_nframes_t nframes)
{
	jack_default_audio_sample_t *pbuf[_channels];
	for (int c = 0; c < _channels; c++)
		pbuf[c] = (jack_default_audio_sample_t *) jack_port_get_buffer(ports[c], nframes);

	size_t read = jack_ringbuffer_read_space(ringbuffer) / sizeof(jack_default_audio_sample_t) / _channels;

	if (read > nframes)
		read = nframes;

	int i;
	for (i = 0; i < read; i++)
		for (int c = 0; c < _channels; c++) {
			jack_ringbuffer_read(ringbuffer, (char *)pbuf[c], sizeof(jack_default_audio_sample_t));
			pbuf[c]++;
		}

	for (; i < nframes; i++)
		for (int c = 0; c < _channels; c++) {
			pbuf[c] = 0;
			pbuf[c]++;
		}

	if (read != nframes)
		printf("[JACK] UNDERRUN!\n");

	return 0;
}

int jackAudioDevice::process_callback(jack_nframes_t nframes, void* arg)
{
	return ((jackAudioDevice*)arg)->process(nframes);
}


uint8_t jackAudioDevice::play(uint32_t len, float *data)
{
//	static int min = 5000;
	static int sleep = (int)((float)BUFSIZE / jack_get_sample_rate(client) / 2. * 1000000.);
	size_t write;
	float writef;
	len /= _channels;

	#ifdef USE_SRC
	if (src_out_buf) {
		while (len) {
			writef = jack_ringbuffer_write_space(ringbuffer);
			writef /= src_data.src_ratio * sizeof(jack_default_audio_sample_t) * _channels;
			write = (size_t)writef;
			if (write >= len) {
				src_data.data_in = data;
				src_data.input_frames = len;
				src_process(src_state, &src_data);
				jack_ringbuffer_write(ringbuffer,
					(char *)src_out_buf,
					src_data.output_frames_gen * sizeof(jack_default_audio_sample_t) * _channels);
/*
				if (len != src_data.input_frames_used)
					printf("[JACK] len %i != %i input_frames_used\n", len, src_data.input_frames_used);
				if (len < min)
					min = len;
				printf("[JACK] %i %i %i %f %i\n",min, src_data.input_frames_used, src_data.output_frames_gen, writef, len);
				data += src_data.input_frames_used * _channels;
				len -= src_data.input_frames_used;
*/
				return 1;
			} else {
				printf("[JACK] OVERRUN!\n");
				usleep(sleep);
			}
		}
	} else
	#endif
	while (len) {
		writef = jack_ringbuffer_write_space(ringbuffer);
		writef /= sizeof(jack_default_audio_sample_t) * _channels;
		write = (size_t)writef;
		if (write >= len) {
			jack_ringbuffer_write(ringbuffer, (char *)data, len * sizeof(jack_default_audio_sample_t) * _channels);
			return 1;
		} else {
			printf("[JACK] OVERRUN!\n");
			usleep(sleep);
		}
	}

        return 1;
}

uint8_t jackAudioDevice::setVolume(int volume){
#if 0
#ifdef OSS_SUPPORT
	ossAudioDevice dev;
	dev.setVolume(volume);
#else
#ifdef ALSA_SUPPORT
	alsaAudioDevice dev;
	dev.setVolume(volume);
#endif
#endif
#endif
	return 1;
}

