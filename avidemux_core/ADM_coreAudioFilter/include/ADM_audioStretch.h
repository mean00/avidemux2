/**
    \fn ADM_audioResample.h
    \brief Wrapper around rubberband


*/
#ifndef ADM_audioStretch_H
#define ADM_audioStretch_H

class ADM_audioStretch
{
protected:
      void *context;
      double   tempo;
      double   pitch;
      uint32_t nbChannels;
      float ** inReorderBuf;
      float ** outReorderBuf;
      long int latency;
      long int discard;
      bool     feedInput;
public:
                ADM_audioStretch(void);
                ~ADM_audioStretch();
       bool     reset(void);
       bool     process(float *from, float *to, uint32_t nbSample,uint32_t maxOutSample, bool last, uint32_t *sampleProcessed, uint32_t *outNbSample);
       bool     init(double tempoRatio, double pitchScale, uint32_t sampleRate, uint32_t channel);

};


#endif
//EOF
