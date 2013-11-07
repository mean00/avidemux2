/**
 *
 *
 *
 *
 */
#ifndef ADM_audioClock_H
#define ADM_audioClock_H

#include "ADM_coreAudio6_export.h"

class ADM_COREAUDIO6_EXPORT audioClock
{
        protected:
                uint32_t        _frequency;
                uint64_t        _nbSamples;
                uint64_t        _baseClock;

        public:
                                audioClock(uint32_t fq);
                 bool           advanceBySample(uint32_t samples);
                 uint64_t       getTimeUs(void);
                 bool           setTimeUs(uint64_t clk);



};


#endif
