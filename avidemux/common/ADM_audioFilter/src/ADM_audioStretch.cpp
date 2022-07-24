/**
    \fn ADM_audioStretch.h
    \brief Wrapper around rubberband


*/
#include "ADM_default.h"
#include "ADM_coreAudio.h"
#include "ADM_audioStretch.h"
#include "ADM_rubberband/rubberband/RubberBandStretcher.h"
#include <math.h>

#define REORDER_BUFSIZE     (65536)
#define OUTP_REORDER_BUFSIZE     (REORDER_BUFSIZE * 11)

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
    inReorderBuf = NULL;
    outReorderBuf = NULL;    
}

#define CONTEXT ((RubberBand::RubberBandStretcher* )context)

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

    inReorderBuf = new float* [nbChannels];
    outReorderBuf = new float* [nbChannels];   

    for (int i=0; i<nbChannels; i++)
    {
        inReorderBuf[i] = new float [REORDER_BUFSIZE];
        outReorderBuf[i] = new float [OUTP_REORDER_BUFSIZE];
    }

    printf("[AudioStretch] Creating x%f, with %d channels\n",tempo,nbChannels);
    RubberBand::RubberBandStretcher * tmpobj = NULL;
    tmpobj = new RubberBand::RubberBandStretcher(sampleRate, nbChannels, 
                                                    RubberBand::RubberBandStretcher::OptionProcessRealTime | 
                                                    RubberBand::RubberBandStretcher::OptionPitchHighQuality | 
                                                    RubberBand::RubberBandStretcher::OptionStretchPrecise, 
                                                (1.0/tempo), pitch);
    context=(void *)tmpobj;
    if(!context) 
    {
        printf("[AudioStretch] Failed ctor\n");
        return false;
    }
    tmpobj->setMaxProcessSize(REORDER_BUFSIZE);
    latency = tmpobj->getLatency();
    discard = latency;
    feedInput = true;
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
    if (inReorderBuf)
    {
        for (int i=0; i<nbChannels; i++)
        {
            delete [] inReorderBuf[i];
        }
        delete [] inReorderBuf;
    }
    inReorderBuf = NULL;

    if (outReorderBuf)
    {
        for (int i=0; i<nbChannels; i++)
        {
            delete [] outReorderBuf[i];
        }
        delete [] outReorderBuf;
    }
    outReorderBuf = NULL;
    printf("[AudioStretch] Deleted\n");
}
/**
    \fn  reset
    \brief Reset the stretcher. Call it when you start resampling a new "file"
*/
bool ADM_audioStretch::reset(void)
{
    ADM_assert(context);
    discard = latency;
    feedInput = true;
    CONTEXT->reset();
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
    
    if (discard <= 0)
    {
        // prevent high peak memory usage by starving RubberBand
        if (CONTEXT->available()) nbSample=0;
    }
    
    if (nbSample > REORDER_BUFSIZE)
        nbSample = REORDER_BUFSIZE;
    // de-interleave input
    for (int s=0; s<nbSample; s++)
    {
        for (int c=0; c<nbChannels; c++)
        {
            inReorderBuf[c][s] = *from;
            from++;
        }
    }
    *sampleProcessed = nbSample;

    if (feedInput)
        CONTEXT->process(inReorderBuf,nbSample,last);
    if (last)
        feedInput = false;
    
    *outNbSample = 0;
    
    while (discard > 0)
    {
        int availSamp = CONTEXT->available();
        if (availSamp == 0) return true;
        if (availSamp < 0) return false;    // EOF
        
        if (availSamp > discard)
            availSamp = discard;
        if (availSamp > OUTP_REORDER_BUFSIZE)
            availSamp = OUTP_REORDER_BUFSIZE;
        availSamp = CONTEXT->retrieve(outReorderBuf, availSamp);
        ADM_assert(discard >= availSamp);
        discard -= availSamp;
    }

    while (*outNbSample < maxOutSample)
    {
        int availSamp = CONTEXT->available();
        if (availSamp == 0) return true;
        if (availSamp < 0) 
        {
            if (*outNbSample > 0)
                return true;
            else
                return false;    // EOF
        }
        
        if (availSamp > (maxOutSample - *outNbSample))
            availSamp = (maxOutSample - *outNbSample);
        if (availSamp > OUTP_REORDER_BUFSIZE)
            availSamp = OUTP_REORDER_BUFSIZE;
        availSamp = CONTEXT->retrieve(outReorderBuf, availSamp);
        ADM_assert(availSamp <= (maxOutSample - *outNbSample));
        ADM_assert(availSamp <= OUTP_REORDER_BUFSIZE);
        // interleave output
        for (int s=0; s<availSamp; s++)
        {
            for (int c=0; c<nbChannels; c++)
            {
                *to = outReorderBuf[c][s];
                to++;
            }
        }
        *outNbSample += availSamp;
    }
    return true;
}
//EOF
