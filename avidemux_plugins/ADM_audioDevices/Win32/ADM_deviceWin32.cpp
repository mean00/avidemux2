//
// C++ Implementation: ADM_deviceWin32
//
// Description:
// C++ Implementation: ADM_deviceWin32
// Use MM layer to output sound
//
// Author: mean <fixounet@free.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//

#include "ADM_default.h"
#include  "ADM_audiodevice.h"
#include  "ADM_audioDeviceInternal.h"

#include <windows.h>


#include "ADM_deviceWin32.h"

#if 1
#define aprintf(...) {}
#define  markTime(...) {}
#else
#define aprintf printf
#define markTime markTime_
#endif


ADM_DECLARE_AUDIODEVICE(Win32,win32AudioDevice,1,0,0,"Win32 audio device (c) mean");

static void markTime_(const char *s)
{
#if 1
  SYSTEMTIME st;
    GetLocalTime(&st);
    ADM_info("MXE [%d.%d] %s\n",st.wSecond,st.wMilliseconds,s);
#endif
}

/**
    \fn ctor
*/
win32AudioDevice::win32AudioDevice(void)
{
    ADM_info("[Win32] Creating audio device\n");
    _inUse=0;
}
/**
  \fn dtor
*/
win32AudioDevice::~win32AudioDevice(void)
{

}

/**
    \fn localStop
*/

bool win32AudioDevice::localStop(void)
{
    if (!_inUse)
        return false;

    ADM_info("[Win32] Closing audio\n");

    waveOutReset(myDevice);

    for (int  i = 0; i < NB_BUCKET; i++)
    {
        waveOutUnprepareHeader(myDevice, &waveHdr[i], sizeof(WAVEHDR));
        delete [] waveHdr[i].lpData;
    }

    myError = waveOutClose(myDevice);

    if (myError != MMSYSERR_NOERROR)
    {
        ADM_info("[Win32] Close failed %d\n", myError);
        handleMM(myError);
        return 0;
    }

    _inUse=0;
    myDevice = NULL;

    return true;
}
/**
    \fn localInit
*/
bool win32AudioDevice::localInit(void)
{
    ADM_info("[Win32] Opening Audio, channels=%u freq=%u\n",_channels, _frequency);
    if(_channels > 2)
    {
        ADM_error("[Win32] The API supports 2 channels max, please enable downmixing.\n");
        return false;
    }

    if (_inUse)
    {
        ADM_warning("[Win32] Already running?\n");
        return false;
    }

    bucketSize = (_channels * _frequency*2)/10; // 100 ms bucket * 32 => 3 sec buffer
    ADM_info("Bucket size=%d\n",(int)bucketSize);
    WAVEFORMATEX wav;

    memset(&wav, 0, sizeof(WAVEFORMATEX));

    wav.wFormatTag = WAVE_FORMAT_PCM;
    wav.nSamplesPerSec = _frequency;
    wav.nChannels = _channels;
    wav.nBlockAlign = 2 * _channels;
    wav.nAvgBytesPerSec = 2 * _channels * _frequency;
    wav.wBitsPerSample = 16;

    myError = waveOutOpen(&myDevice, WAVE_MAPPER, &wav, NULL, NULL, CALLBACK_NULL);

    if (MMSYSERR_NOERROR != myError)
    {
        ADM_warning("[Win32] waveOutOpen failed\n");
        handleMM(myError);
        return 0;
    }

    _inUse = 1;

    for (uint32_t i = 0; i < NB_BUCKET; i++)
    {
        memset(&waveHdr[i], 0, sizeof(WAVEHDR));

        waveHdr[i].dwBufferLength = bucketSize;
        waveHdr[i].lpData = (char*)new uint8_t[bucketSize];

        if (waveOutPrepareHeader(myDevice, &waveHdr[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
            ADM_warning("[Win32] waveOutPrepareHeader error\n");

        waveHdr[i].dwBufferLength = 0;
        waveHdr[i].dwFlags |= WHDR_DONE;
    }

    return 1;
}
/**
    \fn setVolume
*/
uint8_t  win32AudioDevice::setVolume(int volume)
{
    if(!_inUse) return 1;

    uint32_t value;

    value = (0xffff * volume) / 100;
    value = value + (value << 16);

    waveOutSetVolume(myDevice, value);

    return 1;
}
/**
    \fn sendData
*/

int win32AudioDevice::findFreeBucket()
{
    for (uint32_t i = 0; i < NB_BUCKET; i++)
    {
        if (waveHdr[i].dwFlags & WHDR_DONE) // Free ?
        {
            return i;
        }
    }
    return -1;
}

/**
  \fn sendData
*/
void win32AudioDevice::sendData(void)
{
    mutex.lock();
    uint32_t len=wrIndex-rdIndex;
    mutex.unlock();
    MMRESULT er;
    markTime("Enter");
    if(len< (2 * _channels * _frequency)/100) // less than 10 ms, skip
    {
      ADM_usleep(20*1000);
      markTime("Not enough data");
      return;
    }
    while(len)
    {
        markTime("loop");
        if (!_inUse)
        {
           markTime("Not in use");
            ADM_usleep(20*1000); // avoid busy looping
            return;
        }
        int bucket=findFreeBucket();
        if(bucket==-1)
        {
            markTime("No Bucket");
            ADM_usleep(10*1000); // Wait for a bucket , if it failed,return
            return;
        }
        WAVEHDR *b=&(waveHdr[bucket]);
        b->dwFlags &=~WHDR_DONE; // mark as used
        if (len > bucketSize)
              b->dwBufferLength = bucketSize;
        else
              b->dwBufferLength = len;
        mutex.lock();
        memcpy(b->lpData, audioBuffer.at(rdIndex), b->dwBufferLength);
        mutex.unlock();
        markTime("write");
        er=waveOutWrite(myDevice, b, sizeof(WAVEHDR));
        markTime("write done");
        if (er != MMSYSERR_NOERROR)
        {
            handleMM(er);
            b->dwFlags|=WHDR_DONE;
            ADM_warning("Audio out : Error !\n");
            break;
        }
        // consume data
        mutex.lock();
        rdIndex += b->dwBufferLength;
        mutex.unlock();
        len-= b->dwBufferLength ;
    }
    markTime("exit");
}

const CHANNEL_TYPE mono[MAX_CHANNELS]={ADM_CH_MONO};
const CHANNEL_TYPE stereo[MAX_CHANNELS]={ADM_CH_FRONT_LEFT,ADM_CH_FRONT_RIGHT};
/**
    \fn getWantedChannelMapping
*/
const CHANNEL_TYPE *win32AudioDevice::getWantedChannelMapping(uint32_t channels)
{
    switch(channels)
    {
        case 1: return mono;
        case 2: return stereo;
        default:break;
    }
    return NULL;
}
/**
    \fn handleMM
*/
void win32AudioDevice::handleMM(MMRESULT err)
{
#define ERMM(x) if(err==x)  ADM_info("[Win32/Audio ] Error "#x"\n");

    ERMM(MMSYSERR_ALLOCATED);
    ERMM(MMSYSERR_BADDEVICEID);
    ERMM(MMSYSERR_NODRIVER);
    ERMM(WAVERR_BADFORMAT);
    ERMM(WAVERR_SYNC);
    ERMM(WAVERR_STILLPLAYING);
}
// EOF
