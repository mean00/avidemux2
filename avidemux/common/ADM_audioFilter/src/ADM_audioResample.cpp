/**
    \fn ADM_audioResample.h
    \brief Wrapper around libsamplerate


*/
#include "ADM_default.h"
#include "ADM_coreAudio.h"
#include "ADM_audioResample.h"
#include "ADM_libsamplerate/samplerate.h"
#include <math.h>


/**
    \fn ADM_resample
    \brief
*/
ADM_resample::ADM_resample(void)
{
    fromFrequency=0;
    toFrequency=0;
    nbChannels=0;
    context=NULL;
}

#define CONTEXT ((SRC_STATE* )context)

/**
    \fn init
    \brief
    @param from    : Starting frequency
    @param to      : Ending frequency
    @param channel : Nb Channel
*/
bool ADM_resample::init(uint32_t from, uint32_t to, uint32_t channel)
{
int er=0;

    ratio=to;
    ratio/=from;
    if(true!=src_is_valid_ratio (ratio))
    {
        printf("[SRC] Invalid ratio %lf\n",ratio);
        return false;
    }

    printf("[SRC] Creating %u->%u, with %d channels\n",from,to,channel);
    fromFrequency=from;
    toFrequency=to;
    nbChannels=channel;
    context=(void *)src_new (SRC_SINC_FASTEST*0+1*SRC_SINC_MEDIUM_QUALITY, channel, &er) ;
    if(!context) 
    {
        printf("[SRC] Error :%d\n",er);
        return false;
    }
    ADM_assert(!src_set_ratio (CONTEXT,ratio)) ;
    // 
    return true;
}
/**
    \fn ~ ADM_resample
    \brief Destructor
*/
ADM_resample::~ADM_resample(void)
{
    if(context)
        src_delete (CONTEXT) ;
     context=NULL;
    printf("[SRC] Deleted\n");
}
/**
    \fn  reset
    \brief Reset the downsampler. Call it when you start resampling a new "file"
*/
bool ADM_resample::reset(void)
{
    ADM_assert(context);
    src_reset (CONTEXT);
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
bool ADM_resample::process(float *from, float *to, uint32_t nbSample,uint32_t maxOutSample, uint32_t *sampleProcessed, uint32_t *outNbSample)
{
    SRC_DATA block;
    block.data_in=from;
    block.data_out=to;
    block.input_frames=nbSample;
    block.output_frames=maxOutSample;
    block.input_frames_used=0;
    block.output_frames_gen=0;
    block.end_of_input=0;
    block.src_ratio=ratio;
    int er=src_process (CONTEXT,&block);
    if(er)
    {
        printf("[SRC] Error :%d->%s\n",er,src_strerror(er));
        return false;
    }
    *sampleProcessed=block.input_frames_used;
    *outNbSample=block.output_frames_gen;
    return true;
}
//EOF
