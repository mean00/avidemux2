/**
    \fn ADM_audioResample.h
    \brief Wrapper around libsamplerate


*/
#ifndef ADM_audioResample_H
#define ADM_audioResample_H

class ADM_resample
{
protected:
      void *context;
      uint32_t fromFrequency;
      uint32_t toFrequency;
      uint32_t nbChannels;
      double   ratio;
public:
                ADM_resample(void);
                ~ADM_resample();
       bool     reset(void);
       bool     process(float *from, float *to, uint32_t nbSample,uint32_t maxOutSample, uint32_t *sampleProcessed, uint32_t *outNbSample);
       bool     init(uint32_t from, uint32_t to, uint32_t channel);

};


#endif
//EOF
