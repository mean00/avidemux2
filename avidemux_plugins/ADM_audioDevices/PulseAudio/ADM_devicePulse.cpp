/***************************************************************************
                          ADM_devicePulse.cpp  -  description

    audio output plugin for PulseAudio
                          
    copyright            : (C) 2008 by mean
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
#include "ADM_default.h"
#include "ADM_audiodevice.h"
#include <cmath>

#include "ADM_audioDeviceInternal.h"
#include "pulse/pulseaudio.h"
#include "ADM_devicePulse.h"

ADM_DECLARE_AUDIODEVICE(PulseAudio,pulseAudioDevice,1,0,2,"PulseAudio audio device (c) mean");

#define ADM_PULSE_LATENCY 50 // ms

static int appetite;

/**
    \fn pulseAudioDevice
    \brief ctor
*/
pulseAudioDevice::pulseAudioDevice()
{
    stream = connection = mainloop = NULL;
    muted = false;
    appetite = 0;
}
/**
    \fn pulseSimpleAudioDevice
    \brief Returns delay in ms, code mostly stolen from mpv
*/
uint32_t pulseAudioDevice::getLatencyMs(void)
{
    if(!mainloop || !stream || !connection)
        return ADM_PULSE_LATENCY;

    pa_threaded_mainloop *ml = (pa_threaded_mainloop *)mainloop;
    pa_stream *st = (pa_stream *)stream;
    pa_context *ctx = (pa_context *)connection;

    pa_threaded_mainloop_lock(ml);
    pa_stream_update_timing_info(st, NULL, NULL);

    pa_usec_t latency = (pa_usec_t) -1;

    int attempts = 10;
    while(pa_stream_get_latency(st, &latency, NULL) < 0 && attempts > 0)
    {
        if(pa_context_errno(ctx) != PA_ERR_NODATA)
        {
            ADM_warning("pa_stream_get_latency() failed.\n");
            break;
        }
        /* Wait until latency data is available again */
        //printf("[pulseAudioDevice::getLatencyMs] Waiting for latency data, %d attempts left\n",attempts);
        pa_threaded_mainloop_wait(ml);
        attempts--;
    }

    pa_threaded_mainloop_unlock(ml);

    if(attempts < 1)
    {
        //printf("[pulseAudioDevice::getLatencyMs] All attempts consumed, using hardcoded value.\n");
        return ADM_PULSE_LATENCY;
    }
    if(latency == (pa_usec_t) -1)
    {
        //printf("[pulseAudioDevice::getLatencyMs] Cannot query latency, using hardcoded value.\n");
        return ADM_PULSE_LATENCY;
    }

    return (uint32_t)(latency / 1000.0);
}

/**
    \fn localStop
    \brief stop & release device
*/
bool pulseAudioDevice::localStop(void) 
{
    if(mainloop)
        pa_threaded_mainloop_stop((pa_threaded_mainloop *)mainloop);

    if(stream)
    {
        pa_stream *st = (pa_stream *)stream;
        pa_stream_disconnect(st);
        pa_stream_unref(st);
        stream = NULL;
    }
    if(connection)
    {
        pa_context *ctx = (pa_context *)connection;
        pa_context_disconnect(ctx);
        pa_context_unref(ctx);
        connection = NULL;
    }
    if(mainloop)
    {
        pa_threaded_mainloop_free((pa_threaded_mainloop *)mainloop);
        mainloop = NULL;
    }
    return true;
}

//
// callbacks
//
static void context_state_cb(pa_context *c, void *userdata)
{
    pa_threaded_mainloop *ml = (pa_threaded_mainloop *)userdata;
    switch(pa_context_get_state(c))
    {
        case PA_CONTEXT_READY:
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(ml, 0);
            break;
        default:break;
    }
}

static void stream_state_cb(pa_stream *s, void *userdata)
{
    pa_threaded_mainloop *ml = (pa_threaded_mainloop *)userdata;
    switch(pa_stream_get_state(s))
    {
        case PA_STREAM_FAILED:
            ADM_warning("[pulse] Stream failed.\n");
            pa_threaded_mainloop_signal(ml, 0);
            break;
        case PA_STREAM_READY:
        case PA_STREAM_TERMINATED:
            pa_threaded_mainloop_signal(ml, 0);
            break;
        default:break;
    }
}

static void stream_request_cb(pa_stream *s, size_t length, void *userdata)
{
    pa_threaded_mainloop *ml = (pa_threaded_mainloop *)userdata;
    //printf("[pulse] Server requests %u bytes of data\n",length);
    appetite += length;
    pa_threaded_mainloop_signal(ml, 0);
}

/**
    \fn    localInit
    \brief initialize the device
*/
bool pulseAudioDevice::localInit(void) 
{
    ADM_info("PulseAudio, initiliazing channel=%d samplerate=%d\n",(int)_channels,(int)_frequency);
    // Create and start threaded main loop
    pa_threaded_mainloop *ml = pa_threaded_mainloop_new();
    if(!ml)
    {
        ADM_error("[pulse] Cannot allocate main loop.\n");
        return false;
    }
    mainloop = (void *)ml;
    int err = pa_threaded_mainloop_start(ml);
    if(err < 0)
    {
        ADM_error("[pulse] Error %d starting main loop: %s\n",err,pa_strerror(err));
        return false;
    }

    // Create context and connect to PulseAudio server
    pa_threaded_mainloop_lock(ml);
    pa_context *ctx = pa_context_new(pa_threaded_mainloop_get_api(ml), "Avidemux2");
    if(!ctx)
    {
        ADM_error("[pulse] Cannot allocate connection context.\n");
        pa_threaded_mainloop_unlock(ml);
        return false;
    }

    pa_context_set_state_callback(ctx, context_state_cb, mainloop);

    ADM_info("[pulse] Connection context created.\n");
    connection = (void *)ctx;

    err = pa_context_connect(ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);
    if(err < 0)
    {
        ADM_error("[pulse] Error %d connecting to server: %s\n",err,pa_strerror(err));
        pa_threaded_mainloop_unlock(ml);
        return false;
    }
    // Wait until connection gets established
    while(true)
    {
        pa_context_state_t state = pa_context_get_state(ctx);
        if(state == PA_CONTEXT_READY)
            break;
        if(!PA_CONTEXT_IS_GOOD(state))
        {
            pa_threaded_mainloop_unlock(ml);
            return false;
        }
#if 0
        const char *c;
        switch(state)
        {
            case PA_CONTEXT_CONNECTING: c = "connecting"; break;
            case PA_CONTEXT_AUTHORIZING: c = "authorizing"; break;
            case PA_CONTEXT_SETTING_NAME: c = "setting name"; break;
            default: c = "???"; break;
        }
        printf("Connection context state: %s\n",c);
#endif
        pa_threaded_mainloop_wait(ml);
    }

    // Create stream
    pa_sample_spec ss;
    pa_channel_map map,*pmap=NULL;

    // Channel mapping
    if(_channels>2)
    {
        pmap=&map;
        map.channels=_channels;
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
        map.map[2]=PA_CHANNEL_POSITION_FRONT_CENTER;
        map.map[3]=PA_CHANNEL_POSITION_SUBWOOFER;
        map.map[4]=PA_CHANNEL_POSITION_REAR_LEFT;
        map.map[5]=PA_CHANNEL_POSITION_REAR_RIGHT;
        map.map[6]=PA_CHANNEL_POSITION_SIDE_LEFT;
        map.map[7]=PA_CHANNEL_POSITION_SIDE_RIGHT;
    }

    ss.format = PA_SAMPLE_S16LE;
    ss.channels = _channels;
    ss.rate =_frequency;

    if(!pa_sample_spec_valid(&ss))
    {
        ADM_error("[pulse] Sample spec is invalid.\n");
        pa_threaded_mainloop_unlock(ml);
        return false;
    }

    pa_stream *st = pa_stream_new(ctx, "Avidemux2", &ss, pmap);
    if(!st)
    {
        ADM_error("[pulse] Cannot create stream.\n");
        pa_threaded_mainloop_unlock(ml);
        return false;
    }

    pa_stream_set_state_callback(st, stream_state_cb, mainloop);
    pa_stream_set_write_callback(st, stream_request_cb, mainloop);

    stream = (void *)st;

    // Connect to the default sink
    pa_buffer_attr attr;
    attr.maxlength = -1;
    attr.tlength   = -1;
    attr.prebuf    =  0;
    attr.minreq    = -1;
    attr.fragsize  = -1;

    // We want something like 20 ms latency
    uint64_t bufSize = _frequency;
    bufSize *= _channels;
    bufSize *= 2; // 1 second worth of audio

    bufSize /= 1000;
    bufSize *= ADM_PULSE_LATENCY;
    attr.tlength = bufSize; // Latency in bytes

    err = pa_stream_connect_playback(st, NULL, &attr, PA_STREAM_NOFLAGS, NULL, NULL);
    if(err < 0)
    {
        ADM_error("[pulse] Error %d connecting stream: %s\n",err,pa_strerror(err));
        pa_threaded_mainloop_unlock(ml);
        return false;
    }
    // Wait for connection to the sink
    while(true)
    {
        pa_stream_state_t state = pa_stream_get_state(st);
        if(state == PA_STREAM_READY)
            break;
        if(!PA_STREAM_IS_GOOD(state))
        {
            pa_threaded_mainloop_unlock(ml);
            return false;
        }
        pa_threaded_mainloop_wait(ml);
    }

    pa_threaded_mainloop_unlock(ml);

    ADM_info("[pulse] open ok for fq=%d channels=%d\n",ss.rate,ss.channels);
    return true;
}

/**
    \fn setVolume
*/
uint8_t pulseAudioDevice::setVolume(int volume)
{
    if(!stream || !connection || !mainloop)
        return 0;

    if(volume < 0) volume = 0;
    if(volume > 100) volume = 100;

    if(!volume && muted) // nothing to do
        return 1;

    pa_stream *st = (pa_stream *)stream;
    pa_context *ctx = (pa_context *)connection;
    pa_threaded_mainloop *ml = (pa_threaded_mainloop *)mainloop;
    pa_operation *op = NULL;

    pa_threaded_mainloop_lock(ml);
    uint32_t idx = pa_stream_get_index(st);

    if(!volume) // mute
    {
        op = pa_context_set_sink_input_mute(ctx, idx, true, NULL, NULL);
        if(op)
        {
            muted = true;
            pa_operation_unref(op);
        }else
        {
            ADM_warning("[pulse] Cannot mute output.\n");
        }
    }else
    {
        if(muted)
        {
            op = pa_context_set_sink_input_mute(ctx, idx, false, NULL, NULL);
            if(op)
            {
                muted = false;
                pa_operation_unref(op);
            }else
            {
                ADM_warning("[pulse] Cannot unmute output.\n");
            }
        }
        pa_cvolume vol;
        pa_cvolume_reset(&vol, _channels);
        volume = lrint((float)volume * PA_VOLUME_NORM / 100.);
        pa_cvolume_set(&vol, vol.channels, volume);
        op = pa_context_set_sink_input_volume(ctx, idx, &vol, NULL, NULL);
        if(op)
        {
            muted = false;
            pa_operation_unref(op);
        }else
        {
            ADM_warning("[pulse] Cannot set volume.\n");
        }
    }

    pa_threaded_mainloop_unlock(ml);

    return 1;
}

/**
    \fn sendData
    \brief Play samples
*/
void pulseAudioDevice::sendData(void)
{
    if(!stream) return;
    if(!mainloop) return;

    int err;
    pa_stream *st = (pa_stream *)stream;
    pa_threaded_mainloop *ml = (pa_threaded_mainloop *)mainloop;

    mutex.lock();
    ADM_assert(wrIndex >= rdIndex);
    uint32_t avail = wrIndex - rdIndex;
    if(!avail)
    {
        mutex.unlock();
        pa_threaded_mainloop_lock(ml);

        int size = appetite < sizeOf10ms ? appetite : sizeOf10ms;
        appetite -= size;
        if(appetite < 0) appetite = 0;

        err = pa_stream_write(st, silence.at(0), size, NULL, 0, PA_SEEK_RELATIVE);

        pa_threaded_mainloop_unlock(ml);

        if(err < 0)
            ADM_warning("[pulse] pa_stream_write error %d: %s\n",err,pa_strerror(err));
        return;
    }

    if(avail > appetite)
    {
        avail = appetite;
        appetite = 0;
    }else
    {
        appetite -= avail;
    }

    uint8_t *data = audioBuffer.at(rdIndex);
    mutex.unlock();
    pa_threaded_mainloop_lock(ml);

    err = pa_stream_write(st, data, avail, NULL, 0, PA_SEEK_RELATIVE);

    pa_threaded_mainloop_unlock(ml);

    if(err < 0)
        ADM_warning("[pulse] pa_stream_write error %d: %s\n",err,pa_strerror(err));

    mutex.lock();
    rdIndex += avail;
    mutex.unlock();
    return;
}
/**
    \fn getWantedChannelMapping
*/
static const CHANNEL_TYPE mono[MAX_CHANNELS]={ADM_CH_MONO};
static const CHANNEL_TYPE stereo[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT};
static const CHANNEL_TYPE fiveDotOne[MAX_CHANNELS]={
    ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT,
    ADM_CH_FRONT_CENTER,ADM_CH_LFE,
    ADM_CH_REAR_LEFT,ADM_CH_REAR_RIGHT
};
static const CHANNEL_TYPE sevenDotOne[MAX_CHANNELS]={
    ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT,
    ADM_CH_FRONT_CENTER,ADM_CH_LFE,
    ADM_CH_REAR_LEFT,ADM_CH_REAR_RIGHT,
    ADM_CH_SIDE_LEFT,ADM_CH_SIDE_RIGHT
};
const CHANNEL_TYPE *pulseAudioDevice::getWantedChannelMapping(uint32_t channels)
{
    switch(channels)
    {
        case 1: return mono;
        case 2: return stereo;
        case 5:
        case 6: return fiveDotOne;
        case 8: return sevenDotOne;
        default:break;
    }
    return NULL;
}
//EOF
