/**
    \fn ADM_audioStretch.h
    \brief Wrapper around SoundTouch


*/
#include "ADM_default.h"
#include "ADM_coreAudio.h"
#include "ADM_audioStretch.h"
#include "ADM_soundtouch/SoundTouch.h"
#include <math.h>

/**
    \fn ADM_audioStretch
    \brief
*/
ADM_audioStretch::ADM_audioStretch(void)
{
    tempo=1.0;
    pitch=1.0;
    nbChannels=0;
    context=NULL;
}

#define CONTEXT ((soundtouch::SoundTouch* )context)

/**
    \fn init
    \brief
    @param tempoRatio : tempo multiplier (2.0 == play twice as fast)
    @param pitchScale : pitch multiplier (2.0 == 1 octave up)
    @param channel    : Nb Channel
*/
bool ADM_audioStretch::init(double tempoRatio, double pitchScale, uint32_t sampleRate, uint32_t channel)
{
    
    // NaN safe limiting
    if (tempoRatio > 0.1)
    {
        if (tempoRatio < 10.0)
            tempo = tempoRatio;
        else
            tempo = 10.0;
    } else {
        tempo = 0.1;
    }

    if (pitchScale > 0.1)
    {
        if (pitchScale < 10.0)
            pitch = pitchScale;
        else
            pitch = 10.0;
    } else {
        pitch = 0.1;
    }

    if (channel == 0) return false;
    if (channel > MAX_CHANNELS) return false;

    nbChannels=channel;

    printf("[AudioStretch] Creating x%f, with %d channels\n",tempo,nbChannels);
    soundtouch::SoundTouch * tmpobj = NULL;
    tmpobj = new soundtouch::SoundTouch();
    tmpobj->setSampleRate(sampleRate);
    tmpobj->setChannels(channel);
    tmpobj->setTempo(tempo);
    tmpobj->setPitch(pitch);

    context=(void *)tmpobj;
    if(!context) 
    {
        printf("[AudioStretch] Failed ctor\n");
        return false;
    }
    reachedEOF = false;
    // 
    return true;
}
/**
    \fn ~ ADM_audioStretch
    \brief Destructor
*/
ADM_audioStretch::~ADM_audioStretch(void)
{
    if(context)
        delete (CONTEXT);
    context=NULL;
    printf("[AudioStretch] Deleted\n");
}
/**
    \fn  reset
    \brief Reset the stretcher. Call it when you start resampling a new "file"
*/
bool ADM_audioStretch::reset(void)
{
    ADM_assert(context);
    reachedEOF = false;
    CONTEXT->clear();
    return true;

}

/**
    \fn process
    \brief convert samples
    @param  from : Pointer to incoming sample
    @param  to   : Pointer to generated sample
    @param  nbSample : Nb of incoming sample
    @param  maxOutSample : Max # of generated sample
    @param  sampleProdiced : Nb of produced sample
    @return bool

*/
bool ADM_audioStretch::process(float *from, float *to, uint32_t nbSample,uint32_t maxOutSample, bool last, uint32_t *sampleProcessed, uint32_t *outNbSample)
{
    ADM_assert(context);
    
    *sampleProcessed=0;
    *outNbSample = 0;
    if (!reachedEOF)
    {
        int nSamples = CONTEXT->receiveSamples(to, maxOutSample);
        
        if (nSamples > 0)
        {
            *outNbSample = nSamples;
            return true;
        }
        
        CONTEXT->putSamples(from, nbSample);
        *sampleProcessed = nbSample;
        if (last)
        {
            CONTEXT->flush();
            reachedEOF = true;
        }
        return true;
    }
    else
    {
        int nSamples = CONTEXT->receiveSamples(to, maxOutSample);
        if (nSamples > 0)
        {
            *outNbSample = nSamples;
            return true;
        }
        return false;
    }

    return true;
}
//EOF
