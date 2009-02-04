/**
    \file ADM_deviceSDL.cpp 
    \brief SDL audio device plugin

    (C) Mean 2008, fixounet@free.fr
    GPL-v2

*/

#include "ADM_default.h"
#include "SDL.h"

#include  "ADM_audiodevice.h"
#include  "ADM_audioDeviceInternal.h"

#include "ADM_deviceSDL.h"


extern "C"
{
static void SDL_callback(void *userdata, Uint8 *stream, int len);
}

ADM_DECLARE_AUDIODEVICE(Sdl,sdlAudioDevice,1,0,1,"Sdl audio device (c) mean");

sdlAudioDevice::sdlAudioDevice()
{
    active=false;
}
/**
    \fn localStop
    \brief stop audio playback + cleanup buffers

*/
bool  sdlAudioDevice::localStop(void) 
{
	active=false;
    SDL_PauseAudio(1); // First pause it
    SDL_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    
	printf("[SDLAUDIO]Closing SDL audio\n");
	return true;
}
/**
    \fn SDL_callback
    \brief interface between SDL and our class
*/
void SDL_callback(void *userdata, Uint8 *stream, int len)
{

    sdlAudioDevice *me=(sdlAudioDevice *)userdata;
    me->callback(stream,len);
}
/**
    \fn callback
    \brief callback invoked by SDL when it is time to put more datas
*/
uint8_t sdlAudioDevice::callback( Uint8 *stream, int len)
{	
	if(false==active)
    {
        memset(stream,0,len);
        return true;
    }
    // Make sure there is no race with the other thread...
    if(stopRequest!=AUDIO_DEVICE_STARTED || !audioBuffer) return true;
    mutex.lock();
    uint32_t avail=wrIndex-rdIndex;
    if(avail<len)
    {
        printf("[SDLAudio] underflow wanted :%u got %u\n",len,avail);
        memset(stream+avail,0,len-avail);
        len=avail;
    }
    memcpy(stream,audioBuffer+rdIndex,len);
    rdIndex+=len;
    mutex.unlock();
    return true;
   
}
/**
    \fn localInit
    \brief Initialize SDL audio data pump

*/
bool sdlAudioDevice::localInit(void) 
{
SDL_AudioSpec spec,result;

		
		printf("[SDL] Opening audio, fq=%d\n",_frequency);
		
		if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) 
		{
			printf("[SDL] FAILED initialising Audio subsystem\n");
			printf("[SDL] ERROR: %s\n", SDL_GetError());
			return false;
		}
		memset(&spec,0,sizeof(spec));
		memset(&result,0,sizeof(result));
		spec.freq=_frequency;
		spec.channels=_channels;
		spec.samples=4*1024; // nb samples in the buffer
		spec.callback=SDL_callback;
		spec.userdata=this;
		spec.format=AUDIO_S16;
	
		int res=SDL_OpenAudio(&spec,&result);
		if(res<0)
		{
			printf("[SDL] Audio device FAILED to open\n");
			printf("[SDL] ERROR: %s\n", SDL_GetError());

			printf("fq   %d \n",spec.freq);
			printf("chan %d \n", spec.channels);
			printf("samples %d \n",spec.samples);
			printf("format %d \n",spec.format);
			
			printf("fq   %d \n",result.freq);
			printf("chan %d \n", result.channels);
			printf("samples %d \n",result.samples);
			printf("format %d \n",result.format);

			return false;
		}
        active=true;
        SDL_PauseAudio(0); 

    return true;
}
/**
    \fn sendData
    \brief Do nothing, SDL has its own thread/callback

*/
 void     sdlAudioDevice::sendData(void)
{
    ADM_usleep(5*1000);

}


