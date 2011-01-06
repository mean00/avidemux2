#ifndef AUDIO_F_NORMALIZE_H
#define AUDIO_F_NORMALIZE_H

#include "ADM_audioFilter.h"
#include "audiofilter_normalize_param.h"
class AUDMAudioFilterNormalize : public AUDMAudioFilter
{
  protected:
              float       _ratio;
              uint32_t    _scanned;
              uint8_t     preprocess(void);
  public:
    // gainDB10 is the gain in DB multiplied by 10
    // 0 meaning automatic
                          AUDMAudioFilterNormalize(AUDMAudioFilter *previous,GAINparam *param);
    virtual                ~AUDMAudioFilterNormalize();
    virtual    uint32_t   fill(uint32_t max,float *output,AUD_Status *status);
};
#endif
